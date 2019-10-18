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



void run_test(Query::Update::Ptr update_req, int64_t cid, int check=1, bool deleting=false) {
  // Req::Query::Update
  int err = SWC::Error::OK;
  SWC::DB::SchemaPtr schema = SWC::Env::Clients::get()->schemas->get(err, cid);
    if(err) {
    std::cerr << "err=" << err << "(" << SWC::Error::get_text(err) << ")\n";
    exit(1);
  }
  std::cout << "cid=" << cid << " " << schema->to_string() << "\n";
  update_req->columns_cells->create(schema);

  int counted = 0;
  // master-range
  Cells::Cell cell;
  for(int vers=0;vers<2;vers++) {

  for(int i=0;i<20000;i++) {
  std::string cell_number(std::to_string(check)+"-"+std::to_string(i));
  cell.flag = !deleting? Cells::INSERT : Cells::DELETE;
  //cell.set_timestamp(111);
  //cell.set_revision(1234);
  cell.set_time_order_desc(true);

  cell.key.free();
  cell.key.add("a123451");
  cell.key.add("b123451");
  cell.key.add("c123451");
  cell.key.add("d123451");
  cell.key.add("e"+cell_number);
  update_req->columns_cells->add(cid, cell);
  counted++;

  cell.key.free();
  cell.key.add("a987651");
  cell.key.add("b987652");
  cell.key.add("c987653");
  cell.key.add("d987654");
  cell.key.add("e"+cell_number);
  update_req->columns_cells->add(cid, cell);
  counted++;

  cell.key.free();
  cell.key.add("a123454");
  cell.key.add("b123453");
  cell.key.add("c123452");
  cell.key.add("d123451");
  cell.key.add("e"+cell_number);
  update_req->columns_cells->add(cid, cell);
  counted++;

  cell.key.free();
  cell.key.add("a8");
  cell.key.add("e"+cell_number);
  update_req->columns_cells->add(cid, cell);
  counted++;

  cell.key.free();
  cell.key.add("a7");
  cell.key.add("a8");
  cell.key.add("a9");
  cell.key.add("e"+cell_number);
  update_req->columns_cells->add(cid, cell);
  counted++;

  //std::cout << cell.to_string() << "\n";
  cell.free();
  }
  }
  size_t bytes = update_req->columns_cells->size_bytes();
  std::cout << update_req->columns_cells->to_string() << "\n";

  int64_t took =  SWC::Time::now_ns();
  update_req->commit();
  std::cout << " completion=" << update_req->result->completion.load() << "\n";
  update_req->wait();
  took = SWC::Time::now_ns() - took;
  std::cout << " TOOK=" << took << " cells=" << counted << " bytes=" << bytes << " avg="<< took/counted << "\n";

}

int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);
  
  SWC::Env::Clients::init(std::make_shared<SWC::client::Clients>(
    nullptr,
    std::make_shared<SWC::client::AppContext>()
  ));
  
  Query::Update::Ptr update_req = std::make_shared<Query::Update>(
    [](Query::Update::Result::Ptr result)
    {
      std::cout << "CB completion=" << result->completion.load() << "\n";
    }
  );
  

  //std::cout << " ### running-cid=1 ###\n";
  //run_test(update_req, 1);
  //std::cout << " ### running-cid=2 ###\n";
  //run_test(update_req, 2);
  
  for(int check=1; check<=10; check++)
    run_test(update_req, 11, check);

  for(int check=1; check<=10; check++)
    run_test(update_req, 11, check, true);


  std::cout << " ### Waiting ###\n";
  update_req->wait();

  
  SWC::Env::IoCtx::io()->stop();
  std::cout << " ### EXIT ###\n";
  
  exit(0);
}
