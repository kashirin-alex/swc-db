/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */
 
#include "swcdb/lib/core/config/Settings.h"
#include "swcdb/lib/core/comm/Settings.h"

#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/client/AppContext.h"
#include "swcdb/lib/db/Protocol/Common/req/Query.h"


#include "swcdb/lib/db/Stats/Stat.h"


namespace SWC{ namespace Config {

void Settings::init_app_options(){
  init_comm_options();
  // file_desc().add_options();
}
void Settings::init_post_cmd_args(){ }
}}

namespace Cells = SWC::DB::Cells;
namespace Query = SWC::Protocol::Common::Req::Query;



void run_test() {
  // Req::Query::Update
  std::atomic<int> pending=0;
  Query::Update::Ptr update_req = std::make_shared<Query::Update>(
    [&pending=pending](Query::Update::Result result)
    {
      pending--;
      std::cout << "CB pending=" << pending.load() << "\n";
    }
  );

  for(int i=0;i<1000000;i++) {
  Cells::Cell cell;
  cell.flag = Cells::INSERT;
  cell.set_timestamp(111);
  cell.set_revision(222);
  cell.set_time_order_desc(true);

  cell.key.add("a123451");
  cell.key.add("b123451");
  cell.key.add("c123451");
  cell.key.add("d123451");
  update_req->cells->add(11, cell);

  cell.key.free();
  cell.key.add("a987651");
  cell.key.add("b987652");
  cell.key.add("c987653");
  cell.key.add("d987654");
  update_req->cells->add(11, cell);

  cell.key.free();
  cell.key.add("a123454");
  cell.key.add("b123453");
  cell.key.add("c123452");
  cell.key.add("d123451");
  update_req->cells->add(11, cell);

  cell.key.free();
  cell.key.add("a8");
  update_req->cells->add(11, cell);
  cell.free();
    
  }
  pending++;
  update_req->commit();


  std::cout << " ### Waiting ###\n";

  while(pending > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

  SWC::Env::IoCtx::io()->stop();
  std::cout << " ### EXIT ###\n";
  
  exit(0);
}

int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);
  
  SWC::Env::Clients::init(std::make_shared<SWC::client::Clients>(
    nullptr,
    std::make_shared<SWC::client::AppContext>()
  ));
  
  run_test();
}
