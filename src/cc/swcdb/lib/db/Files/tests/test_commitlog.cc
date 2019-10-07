/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/lib/fs/Interface.h"
#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/db/Columns/RangeBase.h"
#include "swcdb/lib/db/Columns/Rgr/CommitLog.h"

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

  int num_cells = 100000;
  int err = Error::OK;
  Env::Schemas::get()->add(
    err, 
    DB::Schema::make(
      1, 
      "col-test",
      Types::Column::PLAIN,
      1, 
      0,
      
      0, 
      Types::Encoding::PLAIN,
      6400000,
      0
    )
  );

  DB::RangeBase::Ptr range = std::make_shared<DB::RangeBase>(1,1);
  server::Rgr::CommitLog::Ptr commit_log = server::Rgr::CommitLog::make(range);
  
  Env::FsInterface::interface()->mkdirs(
    err, range->get_path(DB::RangeBase::log_dir));

  std::cout << " init:  \n" << commit_log->to_string() << "\n";
  

  DB::Cells::Cell cell;
  int64_t rev;
  for(auto i=0;i<num_cells;++i){
      //std::cout << "Initial Cell-"<< i << ":\n";
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
      cell.set_value(std::string("A-Data-Value-1234567890-"+n).c_str());

    commit_log->add(cell);
  }


  std::this_thread::sleep_for(std::chrono::milliseconds(30000));
  std::cout << " added cell=" << num_cells << ": \n" << commit_log->to_string() << "\n";
  
  commit_log->load(err);

  std::this_thread::sleep_for(std::chrono::milliseconds(30000));
  std::cout << " loaded logs: \n" << commit_log->to_string() << "\n";
  
  exit(0);
}
