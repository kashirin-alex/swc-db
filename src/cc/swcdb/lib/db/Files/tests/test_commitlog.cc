/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/fs/Interface.h"
#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/db/Columns/RangeBase.h"
#include "swcdb/lib/db/Files/CellStore.h"

#include <iostream>


void SWC::Config::Settings::init_app_options(){
  init_fs_options();
}
void SWC::Config::Settings::init_post_cmd_args(){}

using namespace SWC;



int main(int argc, char** argv) {
  Env::Config::init(argc, argv);

  Env::FsInterface::init();
  Env::Schemas::init();
  Env::IoCtx::init(8);

  int num_cells = 1000000;
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
  server::Rgr::CommitLog::Ptr commit_log = server::Rgr::CommitLog::make(range);

  DB::SchemaPtr schema = Env::Schemas::get()->get(range->cid);
  
  Env::FsInterface::interface()->rmdir(err, range->get_path(""));
  Env::FsInterface::interface()->mkdirs(
    err, range->get_path(DB::RangeBase::log_dir));
  Env::FsInterface::interface()->mkdirs(
    err, range->get_path(DB::RangeBase::cellstores_dir));

  std::cout << " init:  \n" << commit_log->to_string() << "\n";
  

  int num_threads = 1;
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
  
  commit_log = server::Rgr::CommitLog::make(range);
  std::cout << "new loading: \n" << commit_log->to_string() << "\n";

  commit_log->load(err); // initial range loaded state
  std::cout << "loaded: \n" << commit_log->to_string() << "\n";


  Files::CellStore::ReadersPtr cellstores = std::make_shared<Files::CellStore::Readers>();

  cellstores->push_back(
    Files::CellStore::create_init_read(err, schema->blk_encoding, range));
  auto log = SWC::server::Rgr::CommitLog::make(range);
  log->load(err);
  cellstores->front()->set(log);

  int num_chks = 10;
  std::atomic<int> chk = num_chks;
  for(int i = 1;i<=num_chks; i++){
    
    DB::Cells::Mutable::Ptr cells_mutable = DB::Cells::Mutable::make(2, 2, 0, SWC::Types::Column::PLAIN);
    DB::Cells::ReqScan::Ptr req = DB::Cells::ReqScan::make();
    req->spec = SWC::DB::Specs::Interval::make_ptr();
    req->spec->flags.limit = 2;
    req->cells = cells_mutable;
    req->cb = [cellstores, req, &chk, i](int err){
        std::cout << " chk=" << i ;
        std::cout << " err=" <<  err << "(" << SWC::Error::get_text(err) << ") " ;
        std::cout << req->to_string() << "\n";
        std::cout << cellstores->front()->to_string() << "\n";
        chk--;
      };
    cellstores->front()->scan(req);
  }

  while(chk > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  
  // re-check over already loaded fragments
  num_chks = 10;
  chk = num_chks;
  for(int i = 1;i<=num_chks; i++){
    
    DB::Cells::Mutable::Ptr cells_mutable = DB::Cells::Mutable::make(2, 2, 0, SWC::Types::Column::PLAIN);
    DB::Cells::ReqScan::Ptr req = DB::Cells::ReqScan::make();
    req->spec = SWC::DB::Specs::Interval::make_ptr();
    req->spec->flags.limit = 2;
    req->cells = cells_mutable;
    
    req->cb = [cellstores, req, &chk, i](int err){
        std::cout << " chk=" << i ;
        std::cout << " err=" <<  err << "(" << SWC::Error::get_text(err) << ") " ;
        std::cout << req->to_string() << "\n";
        std::cout << cellstores->front()->to_string() << "\n";
        chk--;
      };
    cellstores->front()->scan(req);
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

  size_t counted = 0;
  for(auto& cs : *cellstores.get())
    counted += cs->cell_count();

  std::cout << " cs counted=" << counted;
  if(schema->cell_versions*num_cells != counted) {
    std::cerr << " BAD, expected="<< schema->cell_versions*num_cells << "\n";
    exit(1);
  }
  std::cerr << " OK\n";

  exit(0);
}
