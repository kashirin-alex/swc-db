/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */


#include "swcdb/ranger/Settings.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/core/sys/Resources.h"

#include "swcdb/ranger/RangerEnv.h"
#include <iostream>


using namespace SWC;


void count_all_cells(size_t num_cells, 
                     SWC::Ranger::Blocks& blocks) {
  std::cout << " count_all_cells: \n";
  std::atomic<int> chk = 1;
  
  auto req = Ranger::ReqScanTest::make();
  req->spec.flags.max_versions = blocks.range->cfg->cell_versions();
  req->cells.reset(
    req->spec.flags.max_versions, 
    0, 
    SWC::Types::Column::PLAIN
  );
  req->spec.flags.limit = num_cells * blocks.range->cfg->cell_versions();
    
  req->cb = [req, &chk, blocks=&blocks](int err){
    std::cout << " err=" <<  err 
              << "(" << SWC::Error::get_text(err) << ") \n" ;
    if(req->cells.size() != req->spec.flags.limit) {
      std::cerr << "all-ver, req->cells.size() != req->spec.flags.limit  \n" 
                << " " << req->cells.size() 
                << " != "  << req->spec.flags.limit <<"\n";
      exit(1);
    }
    //std::cout << req->to_string() << "\n";
    //std::cout << blocks.to_string() << "\n";
    --chk;
  };

  blocks.scan(req);

  while(chk > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  std::cout << " count_all_cells, OK\n";
}

int main(int argc, char** argv) {
  Env::Config::init(argc, argv);

  Env::FsInterface::init(FS::fs_type(
    Env::Config::settings()->get_str("swc.fs")));
  Env::IoCtx::init(8);
  Env::Resources.init(
    Env::IoCtx::io()->ptr(),
    Env::Config::settings()->get<Property::V_GINT32>(
      "swc.rgr.ram.percent"),
    Env::Config::settings()->get<Property::V_GINT32>(
      "swc.rgr.ram.release.rate")
  );

  RangerEnv::init();
  
  auto cid = 11;
  DB::Schema schema;
  schema.cid = cid;
  schema.col_name = "col-test";
  schema.cell_versions = 2;
  schema.blk_size = 64000000;
  schema.blk_cells = 100000;
  Ranger::ColumnCfg col_cfg(cid, schema);

  int err = Error::OK;
  int num_cells = 1000000;
  int versions = 3;

  auto range = std::make_shared<Ranger::Range>(&col_cfg, 1);
  Ranger::CommitLog::Fragments commitlog(col_cfg.key_comp);
  commitlog.init(range);

  Env::FsInterface::interface()->rmdir(err, range->get_path(""));
  Env::FsInterface::interface()->mkdirs(
    err, range->get_path(range->LOG_DIR));
  Env::FsInterface::interface()->mkdirs(
    err, range->get_path(range->CELLSTORES_DIR));

  std::cout << " init:  \n" << commitlog.to_string() << "\n";
  

  int num_threads = 8;
  std::atomic<int> threads_processing = num_threads;

  for(int t=0;t<num_threads;++t) {
    
    std::thread([t, versions, commitlog = &commitlog, &threads_processing, 
                 num=num_cells/num_threads]() {
      std::cout << "thread-adding=" << t 
                << " offset=" << t*num 
                << " until=" << t*num+num << "\n";
      DB::Cells::Cell cell;
      int64_t rev;
      for(int v=0;v<versions;++v) {
      for(auto i=t*num;i<t*num+num;++i){

        std::string n = std::to_string(i);
      
        rev = SWC::Time::now_ns();
        cell.flag = DB::Cells::INSERT;
        cell.set_timestamp(rev-1);
        //cell.set_revision(rev);
        cell.set_time_order_desc(false);

        cell.key.free();
        cell.key.add("aFraction1");
        cell.key.add("aFraction2");
        cell.key.add(n);
        cell.key.add("aFraction3");
        cell.key.add("aFraction4");
        cell.key.add("aFraction5");
        //if(num_revs == r)
        //  cell.set_counter(Cells::OP_EQUAL, 0);
        //else
        //  cell.set_counter(0, 1);
        std::string s("A-Data-Value-1234567890-"+n);
        cell.set_value(s.data(), s.length());

        commitlog->add(cell);
        
        if((i % 100000) == 0)
          std::cout << "thread-adding=" << t 
                    << " progress=" << i << "\n";
      }
      }
      --threads_processing;
    }).detach();
  }

  while(threads_processing > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  commitlog.commit_new_fragment(true);

  std::cout << " added cell=" << num_cells 
            << ": \n" << commitlog.to_string() << "\n";
  std::cout << " cells_count=" << commitlog.cells_count() << "\n";
  if((versions == 1 || versions == col_cfg.cell_versions()) 
      && num_cells*col_cfg.cell_versions() != commitlog.cells_count()) {
    exit(1);
  }
  commitlog.unload();
  std::cout << "\n FINISH CREATE LOG\n\n ";
  
  std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  

  ///


  SWC::Ranger::Blocks blocks(col_cfg.key_comp);
  blocks.init(range);
  std::cout << "new loading: \n" << blocks.to_string() << "\n";
  blocks.cellstores.add(Ranger::CellStore::create_initial(err, range));
  blocks.load(err);
  std::cout << "loaded: \n" << blocks.to_string() << "\n";


  int num_chks = 10;
  std::atomic<int> chk = num_chks;
  for(int i = 1;i<=num_chks; ++i){
    
    auto req = Ranger::ReqScanTest::make();
    req->cells.reset(1, 0, SWC::Types::Column::PLAIN);
    req->spec.flags.limit = num_cells;
    
    req->cb = [req, &chk, i, blocks=&blocks](int err){
      std::cout << " chk=" << i 
                << " err=" <<  err 
                << "(" << SWC::Error::get_text(err) << ") \n" ;
      if(req->cells.size() != req->spec.flags.limit) {
        std::cerr << "one-ver, req->cells.size() != req->spec.flags.limit  \n" 
                  << " " << req->cells.size() 
                  << " !="  << req->spec.flags.limit <<"\n";
        exit(1);
      }
      //std::cout << req->to_string() << "\n";
      //std::cout << blocks.to_string() << "\n";
      --chk;
    };
    blocks.scan(req);
  }

  std::cout << " scanned blocks: \n" << blocks.to_string() << "\n";

  while(chk > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  
  count_all_cells(num_cells, blocks);

  std::cout << " scanned blocks, OK\n";


  
  std::cout << "blocks.add_logged: \n";

  std::cout << " adding to blocks: \n" << blocks.to_string() << "\n";

  int added_num = 1000000;
  DB::Cells::Cell cell;
  int64_t rev;
  for(int v=0;v<versions;++v) {
    for(auto i=0;i<added_num;++i){

        std::string n = std::to_string(i)+"-added";
      
        rev = SWC::Time::now_ns();
        cell.flag = DB::Cells::INSERT;
        cell.set_timestamp(rev-1);
        //cell.set_revision(rev);
        cell.set_time_order_desc(false);

        cell.key.free();
        cell.key.add("aFraction1");
        cell.key.add("aFraction2");
        cell.key.add(n);
        cell.key.add("aFraction3");
        cell.key.add("aFraction4");
        cell.key.add("aFraction5");
        //if(num_revs == r)
        //  cell.set_counter(Cells::OP_EQUAL, 0);
        //else
        //  cell.set_counter(0, 1);
        std::string s("A-Data-Value-1234567890-"+n);
        cell.set_value(s.data(), s.length());
        
        blocks.add_logged(cell);
    }
    std::cout << " add_logged ver=" << v 
              << " : \n" << blocks.to_string() << "\n";
  }


  count_all_cells(num_cells+added_num, blocks);

  std::cout << " scanned blocks (add_logged): \n" 
            << blocks.to_string() << "\n";
  std::cerr << " scanned blocks (add_logged), OK\n";

  Env::FsInterface::interface()->rmdir(
    err, DB::RangeBase::get_column_path(range->cfg->cid));
  
  std::cout << "\n-   OK   -\n\n";

  exit(0);
}
