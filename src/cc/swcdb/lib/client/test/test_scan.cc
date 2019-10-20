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


void run_test(Query::Select::Ptr select_req, int64_t cid, int versions=2, int num_cells=1, int check=1) {
  std::cout << "Select run_test cid=" << cid << " versions=" << versions 
                                      << " num-cells=" << num_cells 
                                      << " check=" << check <<"\n";
  // Req::Query::Select
  select_req->specs.columns = {SWC::DB::Specs::Column::make_ptr(cid, {SWC::DB::Specs::Interval::make_ptr()})};
  select_req->scan();
  select_req->wait();

}


void run_test(Query::Update::Ptr update_req, int64_t cid, int versions=2, int num_cells=1, int check=1, bool deleting=false) {
  std::cout << "Update run_test cid=" << cid << " versions=" << versions 
                                      << " num-cells=" << num_cells 
                                      << " check=" << check 
                                      << " deleting=" << deleting << "\n";
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
  for(int vers=0;vers<versions;vers++) {

  for(int i=0;i<num_cells;i++) {
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

  Query::Select::Ptr select_req = std::make_shared<Query::Select>(
    [](Query::Select::Result::Ptr result)
    {
      std::cout << "CB completion=" << result->completion.load() << "\n";
    }
  );
  

  //std::cout << " ### running-cid=1 ###\n";
  //run_test(update_req, 1);
  //std::cout << " ### running-cid=2 ###\n";
  //run_test(update_req, 2);
  run_test(update_req, 11, 1, 100000, 1);
  run_test(select_req, 11, 1, 100, 1);
  exit(0);

  for(int check=1; check<=10; check++)
    run_test(update_req, 11, 2, 100000, check);

  for(int check=1; check<=10; check++)
    run_test(update_req, 11, 2, 100000, check, true);

  for(int check=1; check<=1000; check++)
    run_test(update_req, 11, 2, 1, check);

  for(int check=1; check<=1000; check++)
    run_test(update_req, 11, 2, 1, check, true);

  
  SWC::Env::IoCtx::io()->stop();
  std::cout << " ### EXIT ###\n";
  
  exit(0);
}
