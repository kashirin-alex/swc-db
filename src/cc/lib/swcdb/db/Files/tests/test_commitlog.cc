/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/ranger/Settings.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/client/Clients.h"
#include "swcdb/db/Columns/RangeBase.h"
#include "swcdb/core/Resources.h"

#include "swcdb/db/Columns/Rgr/IntervalBlocks.h"
#include <iostream>


using namespace SWC;


void count_all_cells(DB::Schema::Ptr schema, size_t num_cells, 
                     SWC::server::Rgr::IntervalBlocks& blocks) {
  std::cout << " count_all_cells: \n";
  std::atomic<int> chk = 1;
  
  auto req = DB::Cells::ReqScanTest::make();
  req->cells.reset(
    schema->cid, schema->cell_versions, 0, SWC::Types::Column::PLAIN);
  req->spec.flags.limit = num_cells * schema->cell_versions;
    
  req->cb = [req, &chk, blocks=&blocks](int err){
    std::cout << " err=" <<  err << "(" << SWC::Error::get_text(err) << ") \n" ;
    if(req->cells.size() != req->spec.flags.limit) {
      std::cerr << "all-ver, req->cells.size() != req->spec.flags.limit  \n" 
                << " " << req->cells.size() << " != "  << req->spec.flags.limit <<"\n";
      exit(1);
    }
    //std::cout << req->to_string() << "\n";
    //std::cout << blocks->to_string() << "\n";
    chk--;
  };

  blocks.scan(req);

  while(chk > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  std::cout << " count_all_cells, OK\n";
}

int main(int argc, char** argv) {
  Env::Config::init(argc, argv);

  Env::FsInterface::init(FS::fs_type(
    Env::Config::settings()->get<std::string>("swc.fs")));
  Env::Schemas::init();
  Env::IoCtx::init(8);
  Env::Resources.init(
    Env::IoCtx::io()->ptr(),
    Env::Config::settings()->get_ptr<SWC::gInt32t>(
      "swc.rgr.ram.percent")
  );

  int num_cells = 200000;
  int versions = 3;
  int err = Error::OK;
  Env::Schemas::get()->add(
    err, 
    DB::Schema::make(
      1, 
      "col-test",
      Types::Column::PLAIN,
      2, //versions, 
      0,
      
      0, 
      Types::Encoding::PLAIN,
      6400000,
      0
    )
  );


  auto range = std::make_shared<DB::RangeBase>(1, 1);
  auto schema = Env::Schemas::get()->get(range->cid);
  Files::CommitLog::Fragments commitlog;
  commitlog.init(range);

  Env::FsInterface::interface()->rmdir(err, range->get_path(""));
  Env::FsInterface::interface()->mkdirs(
    err, range->get_path(DB::RangeBase::log_dir));
  Env::FsInterface::interface()->mkdirs(
    err, range->get_path(DB::RangeBase::cellstores_dir));

  std::cout << " init:  \n" << commitlog.to_string() << "\n";
  

  int num_threads = 8;
  std::atomic<int> threads_processing = num_threads;

  for(int t=0;t<num_threads;t++) {
    
    std::thread([t, versions, commitlog = &commitlog, &threads_processing, num=num_cells/num_threads](){
      std::cout << "thread-adding=" << t 
                << " offset=" << t*num 
                << " until=" << t*num+num << "\n";
      DB::Cells::Cell cell;
      int64_t rev;
      for(int v=0;v<versions;v++) {
      for(auto i=t*num;i<t*num+num;++i){

        std::string n = std::to_string(i);
      
        rev = SWC::Time::now_ns();
        cell.flag = DB::Cells::INSERT;
        cell.set_timestamp(rev-1);
        cell.set_revision(rev);
        cell.set_time_order_desc(false);

        cell.key.free();
        cell.key.add("aFraction1");
        cell.key.add("aFraction2");
        cell.key.add(n);
        cell.key.add("aFraction3");
        cell.key.add("aFraction4");
        cell.key.add("aFraction5");
        //if(num_revs == r)
        //  cell.set_value(Cells::OP::EQUAL, 0);
        //else
        //  cell.set_value(Cells::OP::PLUS, 1);
        std::string s("A-Data-Value-1234567890-"+n);
        cell.set_value(s.data(), s.length());

        commitlog->add(cell);
        
        if((i % 100000) == 0)
          std::cout << "thread-adding=" << t 
                    << " progress=" << i << "\n";
      }
      }
      threads_processing--;
    }).detach();
  }

  while(threads_processing > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  commitlog.commit_new_fragment(true);

  std::cout << " added cell=" << num_cells << ": \n" << commitlog.to_string() << "\n";
  std::cout << " cells_count=" << commitlog.cells_count() << "\n";
  if((versions == 1 || versions == schema->cell_versions) 
      && num_cells*schema->cell_versions != commitlog.cells_count()) {
    exit(1);
  }
  commitlog.unload();
  std::cout << "\n FINISH CREATE LOG\n\n ";
  
  std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  

  ///


  SWC::server::Rgr::IntervalBlocks blocks;
  blocks.init(range);
  std::cout << "new loading: \n" << blocks.to_string() << "\n";
  blocks.cellstores.add(
    Files::CellStore::create_init_read(err, schema->blk_encoding, range));
  blocks.load(err);
  std::cout << "loaded: \n" << blocks.to_string() << "\n";


  int num_chks = 10;
  std::atomic<int> chk = num_chks;
  for(int i = 1;i<=num_chks; i++){
    
    auto req = DB::Cells::ReqScanTest::make();
    req->cells.reset(schema->cid, 1, 0, SWC::Types::Column::PLAIN);
    req->spec.flags.limit = num_cells;
    
    req->cb = [req, &chk, i, blocks=&blocks](int err){
      std::cout << " chk=" << i 
                << " err=" <<  err << "(" << SWC::Error::get_text(err) << ") \n" ;
      if(req->cells.size() != req->spec.flags.limit) {
        std::cerr << "one-ver, req->cells.size() != req->spec.flags.limit  \n" 
                  << " " << req->cells.size() << " !="  << req->spec.flags.limit <<"\n";
        exit(1);
      }
      //std::cout << req->to_string() << "\n";
      //std::cout << blocks->to_string() << "\n";
      chk--;
    };
    blocks.scan(req);
  }

  std::cout << " scanned blocks: \n" << blocks.to_string() << "\n";

  while(chk > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  
  count_all_cells(schema, num_cells, blocks);

  std::cout << " scanned blocks, OK\n";


  
  std::cout << "blocks.add_logged: \n";

  std::cout << " adding to blocks: \n" << blocks.to_string() << "\n";

  int added_num = 1000000;
  DB::Cells::Cell cell;
  int64_t rev;
  for(int v=0;v<versions;v++) {
    for(auto i=0;i<added_num;i++){

        std::string n = std::to_string(i)+"-added";
      
        rev = SWC::Time::now_ns();
        cell.flag = DB::Cells::INSERT;
        cell.set_timestamp(rev-1);
        cell.set_revision(rev);
        cell.set_time_order_desc(false);

        cell.key.free();
        cell.key.add("aFraction1");
        cell.key.add("aFraction2");
        cell.key.add(n);
        cell.key.add("aFraction3");
        cell.key.add("aFraction4");
        cell.key.add("aFraction5");
        //if(num_revs == r)
        //  cell.set_value(Cells::OP::EQUAL, 0);
        //else
        //  cell.set_value(Cells::OP::PLUS, 1);
        std::string s("A-Data-Value-1234567890-"+n);
        cell.set_value(s.data(), s.length());

        blocks.add_logged(cell);
    }
    std::cout << " add_logged ver=" << v 
              << " : \n" << blocks.to_string() << "\n";
  }


  count_all_cells(schema, num_cells+added_num, blocks);

  std::cout << " scanned blocks (add_logged): \n" << blocks.to_string() << "\n";
  std::cerr << " scanned blocks (add_logged), OK\n";

  exit(0);
}
