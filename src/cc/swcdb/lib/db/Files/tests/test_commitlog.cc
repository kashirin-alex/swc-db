/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/ranger/Settings.h"

#include "swcdb/lib/fs/Interface.h"
#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/db/Columns/RangeBase.h"
#include "swcdb/lib/core/Resources.h"

#include "swcdb/lib/db/Columns/Rgr/IntervalBlocks.h"
#include <iostream>


using namespace SWC;



int main(int argc, char** argv) {
  Env::Config::init(argc, argv);

  Env::FsInterface::init();
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

  DB::RangeBase::Ptr range = std::make_shared<DB::RangeBase>(1,1);
  Files::CommitLog::Fragments::Ptr commit_log 
    = Files::CommitLog::Fragments::make(range);

  auto schema = Env::Schemas::get()->get(range->cid);
  
  Env::FsInterface::interface()->rmdir(err, range->get_path(""));
  Env::FsInterface::interface()->mkdirs(
    err, range->get_path(DB::RangeBase::log_dir));
  Env::FsInterface::interface()->mkdirs(
    err, range->get_path(DB::RangeBase::cellstores_dir));

  std::cout << " init:  \n" << commit_log->to_string() << "\n";
  

  int num_threads = 8;
  std::atomic<int> threads_processing = num_threads;

  for(int t=0;t<num_threads;t++) {
    
    std::thread([t, versions, commit_log, &threads_processing, num=num_cells/num_threads](){
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

        commit_log->add(cell);
        
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

  commit_log->commit_new_fragment(true);

  std::cout << " added cell=" << num_cells << ": \n" << commit_log->to_string() << "\n";
  std::cout << " cells_count=" << commit_log->cells_count() << "\n";
  if((versions == 1 || versions == schema->cell_versions) 
      && num_cells*schema->cell_versions != commit_log->cells_count()) {
    exit(1);
  }
  
  std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  
  commit_log = Files::CommitLog::Fragments::make(range);
  std::cout << "new loading: \n" << commit_log->to_string() << "\n";

  commit_log->load(err); // initial range loaded state
  std::cout << "loaded: \n" << commit_log->to_string() << "\n";

  SWC::server::Rgr::IntervalBlocks blocks;
  blocks.init(range);
  blocks.load(err);
  blocks.cellstores->add(
    Files::CellStore::create_init_read(err, schema->blk_encoding, range));



  int num_chks = 10;
  std::atomic<int> chk = num_chks;
  for(int i = 1;i<=num_chks; i++){
    
    auto req = DB::Cells::ReqScanTest::make();
    req->cells = DB::Cells::Mutable::make(2, 2, 0, SWC::Types::Column::PLAIN);
    req->spec = SWC::DB::Specs::Interval::make_ptr();
    req->spec->flags.limit = 2;
    
    req->cb = [req, &chk, i, cellstores = blocks.cellstores](int err){
        std::cout << " chk=" << i ;
        std::cout << " err=" <<  err << "(" << SWC::Error::get_text(err) << ") " ;
        std::cout << req->to_string() << "\n";
        std::cout << cellstores->to_string() << "\n";
        chk--;
      };
    blocks.scan(req);
  }

  while(chk > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  
  // re-check over already loaded fragments
  num_chks = 10;
  chk = num_chks;
  for(int i = 1;i<=num_chks; i++){
    
    auto req = DB::Cells::ReqScanTest::make();
    req->spec = SWC::DB::Specs::Interval::make_ptr();
    req->spec->flags.limit = 2;
    req->cells = DB::Cells::Mutable::make(2, 2, 0, SWC::Types::Column::PLAIN);
    
    req->cb = [req, &chk, i, cellstores = blocks.cellstores](int err){
      std::cout << " chk=" << i ;
      std::cout << " err=" <<  err << "(" << SWC::Error::get_text(err) << ") " ;
      std::cout << req->to_string() << "\n";
      std::cout << cellstores->to_string() << "\n";
      chk--;
    };
    blocks.scan(req);
  }
  
  while(chk > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    

  std::cout << " loaded logs: \n" << commit_log->to_string() << "\n";
  
  std::cout << " cells_count=" << commit_log->cells_count() << "\n";
  if((versions == 1 || versions == schema->cell_versions) 
    && schema->cell_versions*num_cells != commit_log->cells_count()) {
    std::cerr << " BAD, expected="<< schema->cell_versions*num_cells << "\n";
    exit(1);
  }
  std::cerr << " OK\n";

  size_t counted = blocks.cells_count();

  std::cout << " cs counted=" << counted;
  if(schema->cell_versions*num_cells != counted) {
    std::cerr << " BAD, expected="<< schema->cell_versions*num_cells << "\n";
    exit(1);
  }
  std::cerr << " OK\n";
  
  std::cerr << "blocks.add_logged: \n";

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
  }

  std::cout << "AFTER(add_logged): \n" << blocks.to_string() << "\n";

  counted = blocks.cells_count();

  std::cout << " cs counted=" << counted;
  if(schema->cell_versions*(num_cells+added_num) != counted) {
    std::cerr << " BAD after(add_logged), expected="<< (schema->cell_versions*(num_cells+added_num)) << "\n";
    exit(1);
  }
  std::cerr << " OK\n";
  exit(0);
}
