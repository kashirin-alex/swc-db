/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */
 
#include "swcdb/lib/core/config/Settings.h"
#include "swcdb/lib/core/comm/Settings.h"

#include "swcdb/lib/client/Clients.h"
#include "swcdb/lib/client/AppContext.h"
#include "swcdb/lib/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/lib/db/Protocol/Mngr/req/ColumnGet.h"
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

bool quite = true;
std::atomic<bool> finished = false;


void run_test(Query::Select::Ptr select_req, int64_t cid, int versions=2, int num_cells=1, int check=1) {
  std::cout << "Select run_test cid=" << cid << " versions=" << versions 
                                      << " num-cells=" << num_cells 
                                      << " check=" << check <<"\n";
  // Req::Query::Select
  int err = SWC::Error::OK;
  SWC::DB::SchemaPtr schema = SWC::Env::Clients::get()->schemas->get(err, cid);
    if(err) {
    std::cerr << "err=" << err << "(" << SWC::Error::get_text(err) << ")\n";
    exit(1);
  }
  std::cout << "cid=" << cid << " " << schema->to_string() << "\n";

  select_req->specs.columns = {SWC::DB::Specs::Column::make_ptr(schema->cid, {SWC::DB::Specs::Interval::make_ptr()})};
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
  for(int vers=1;vers<=versions;vers++) {

  for(int i=0;i<num_cells;i++) {
  std::string cell_number(std::to_string(check)+"-"+std::to_string(i));
  cell.flag = !deleting? Cells::INSERT : Cells::DELETE;
  //cell.set_timestamp(111);
  //cell.set_revision(1234);
  cell.set_revision(vers);
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

void expect_empty_column(int64_t cid) {
    // Req::Query::Select
  Query::Select::Ptr select_req = std::make_shared<Query::Select>(
    [](Query::Select::Result::Ptr result)
    {
      if(quite)return;
      std::cout << "CB completion=" << result->completion.load() << "\n";
      for(auto col : result->columns) {
        std::cout << " cid=" << col.first << ": sz=" << col.second->cells.size() << "\n";
        int num=0;
        for(auto cell : col.second->cells)
          std::cout << "  " << ++num << ":" << cell->to_string() << "\n";  
      }
    }
  );
  
  auto spec = SWC::DB::Specs::Interval::make_ptr();
  spec->flags.offset=0;
  spec->flags.limit=1;
  // spec->flags.return_deletes = true;
  select_req->specs.columns = {SWC::DB::Specs::Column::make_ptr(cid, {spec})};
  select_req->scan();
  select_req->wait();

  if(select_req->result->columns[cid]->cells.size() > 0) {
    std::cerr << "BAD, column not empty: \n" 
              << " " << spec->to_string() << "\n"
              << " expected_value=0\n"
              << "  result_value=" << select_req->result->columns[cid]->cells.size() << "\n"
              << " " << select_req->result->columns[cid]->cells[0]->to_string() << "\n";
    exit(1);
  }
}


void test_1(const std::string& col_name) {
  int num_cells = 1000000; // test require at least 12
  int batches = 1;
  int64_t took;

  // Req::Query::Update
  Query::Update::Ptr update_req = std::make_shared<Query::Update>(
    [](Query::Update::Result::Ptr result)
    {
      std::cout << "CB completion=" << result->completion.load() 
                << "err=" << result->err << "\n";
    }
  );
  
  int err = SWC::Error::OK;
  SWC::DB::SchemaPtr schema = SWC::Env::Clients::get()->schemas->get(err, col_name);
    if(err) {
    std::cerr << "err=" << err << "(" << SWC::Error::get_text(err) << ")\n";
    exit(1);
  }
  std::cout << schema->to_string() << "\n";
  update_req->columns_cells->create(schema);
  
  Cells::Cell cell;
  size_t added_count = 0;
  for(int b=0;b<batches;b++) {
  took =  SWC::Time::now_ns();
  for(int i=0;i<num_cells;i++) {
    std::string cell_number(std::to_string(b)+":"+std::to_string(i));
    cell.flag = Cells::INSERT;
    cell.set_time_order_desc(true);

    cell.key.free();
    for(uint8_t chr=97; chr<=122;chr++)
      cell.key.add(((char)chr)+cell_number);
    cell.set_value("V_OF: "+cell_number);

    update_req->columns_cells->add(schema->cid, cell);
    added_count++;
  }

  size_t bytes = update_req->columns_cells->size_bytes();
  std::cout << update_req->columns_cells->to_string() << "\n";
  

  update_req->timeout_commit = 10*num_cells;
  update_req->commit();
  update_req->wait();
  took = SWC::Time::now_ns() - took;
  std::cout << "UPDATE-INSERT-TOOK=" << took 
            << " cells=" << num_cells 
            << " bytes=" << bytes 
            << " avg="<< took/num_cells << "\n";

  }
  
  std::cout << "INSERT added_count="<<added_count<<"\n";
  //exit(1);
  // Req::Query::Select
  Query::Select::Ptr select_req = std::make_shared<Query::Select>(
    [](Query::Select::Result::Ptr result)
    {
      if(quite)return;
      std::cout << "CB completion=" << result->completion.load() << "\n";
      for(auto col : result->columns) {
        std::cout << " cid=" << col.first << ": sz=" << col.second->cells.size() << "\n";
        int num=0;
        for(auto cell : col.second->cells)
          std::cout << "  " << ++num << ":" << cell->to_string() << "\n";  
      }
    }
  );
  
  took =  SWC::Time::now_ns();
  auto spec = SWC::DB::Specs::Interval::make_ptr();
  spec->flags.offset=batches*num_cells-1;
  spec->flags.limit=1;
  select_req->specs.columns = {SWC::DB::Specs::Column::make_ptr(schema->cid, {spec})};
  select_req->scan();
  select_req->wait();

  if(select_req->result->columns[schema->cid]->cells.size() != spec->flags.limit) {
    std::cerr << "BAD, on offset, select cells count: \n" 
              << " " << spec->to_string() << "\n"
              << " expected_value=" << spec->flags.limit << "\n"
              << "   result_value=" << select_req->result->columns[schema->cid]->cells.size() << "\n";
    exit(1);
  }
  // std::this_thread::sleep_for(std::chrono::milliseconds(60000));

  Cells::Cell* cell_res = select_req->result->columns[schema->cid]->cells.front();
  std::string value((const char*)cell_res->value, cell_res->vlen);
  
  std::string expected_value(
    "V_OF: "+std::to_string(batches-1)+":"+std::to_string(num_cells-1));
  if(expected_value.compare(value) != 0) {
    std::cerr << "BAD, selected cell's value doesn't match: \n" 
              << " expected_value=" << expected_value << "\n"
              << "   result_value=" << value << "\n";
    exit(1);
  }
  took = SWC::Time::now_ns() - took;
  std::cout << "SELECT-TOOK=" << took  << "\n";

  for(int tmp=1;tmp <=3;tmp++) {
  select_req->result->columns[schema->cid]->free();
  spec =  select_req->specs.columns[0]->intervals[0];
  spec->flags.offset=10;
  spec->flags.limit=batches*num_cells-spec->flags.offset;

  took =  SWC::Time::now_ns();
  select_req->scan();
  select_req->wait();
  if(select_req->result->columns[schema->cid]->cells.size() != spec->flags.limit) {
    Cells::Cell prev;
    for(auto c : select_req->result->columns[schema->cid]->cells) {
      if(prev.flag != Cells::NONE) {
        if(c->key.equal(prev.key)) {
          std::cerr << " current  " << c->to_string() << "\n";
          std::cerr << " previous " << prev.to_string() << "\n";
        }
      }
      prev.copy(*c);
    }

    std::cerr << "\n first: " << select_req->result->columns[schema->cid]->cells.front()->to_string() << "\n";
    std::cerr <<   "  last: " << select_req->result->columns[schema->cid]->cells.back()->to_string() << "\n";
    std::cerr << "\nBAD, select cells count: \n" 
              << " " << spec->to_string() << "\n"
              << " expected_value=" << spec->flags.limit << "\n"
              << "   result_value=" << select_req->result->columns[schema->cid]->cells.size() << "\n";
    exit(1);
  }
  took = SWC::Time::now_ns() - took;
  std::cout << "SELECT-TOOK=" << took  << " probe=" << tmp << "\n";
  }

  
  select_req->result->columns[schema->cid]->free();
  spec =  select_req->specs.columns[0]->intervals[0];
  spec->free();
  spec->key_start.add("", SWC::Condition::NONE);
  spec->key_start.add("", SWC::Condition::NONE);
  spec->key_start.add("", SWC::Condition::NONE);
  spec->key_start.add("", SWC::Condition::NONE);
  std::string faction(
    "e"+std::to_string(batches-1)+":"+std::to_string(num_cells-12));
  spec->key_start.add(faction, SWC::Condition::EQ);
  spec->key_start.add("", SWC::Condition::NONE);
  spec->flags.offset=0;
  spec->flags.limit=1;
  took =  SWC::Time::now_ns();
  select_req->scan();
  select_req->wait();
  if(select_req->result->columns[schema->cid]->cells.size() != spec->flags.limit) {
    std::cerr << "BAD, match key fraction, select cells count: \n" 
              << " expected_value=" << spec->flags.limit << "\n"
              << "   result_value=" << select_req->result->columns[schema->cid]->cells.size() << "\n";
    exit(1);
  }
  if(select_req->result->columns[schema->cid]->cells[0]->key.get_string(4).compare(faction) != 0) {
    std::cerr << "BAD, select cell by key fraction: \n" 
              << "  expected_value=" << spec->key_start.get_string(4) << "\n"
              << " (4)result_value=" << select_req->result->columns[schema->cid]->cells[0]->to_string() << "\n";
    exit(1);
  }
  took = SWC::Time::now_ns() - took;
  std::cout << "SELECT-TOOK=" << took  << "\n";
  

  for(int b=0;b<batches;b++) {
  took =  SWC::Time::now_ns();
  for(int i=0;i<num_cells;i++) {
    cell.free();
    std::string cell_number(std::to_string(b)+":"+std::to_string(i));
    cell.flag = Cells::DELETE;
    cell.key.free();
    for(uint8_t chr=97; chr<=122;chr++)
      cell.key.add(((char)chr)+cell_number);
    update_req->columns_cells->add(schema->cid, cell);
    //std::cout << " add: " << cell.to_string() << "\n";
  }

  
  size_t bytes = update_req->columns_cells->size_bytes();
  std::cout << update_req->columns_cells->to_string() << "\n";
  
  took =  SWC::Time::now_ns();
  update_req->timeout_commit = 10*num_cells;
  update_req->commit();
  update_req->wait();
  if(update_req->columns_cells->size_bytes() != 0) {
    std::cerr << " ERROR, remain_bytes=" << update_req->columns_cells->size_bytes() << "\n";
    exit(1);
  }
  took = SWC::Time::now_ns() - took;
  std::cout << "UPDATE-DELETE-TOOK=" << took 
            << " cells=" << num_cells 
            << " bytes=" << bytes 
            << " remain_bytes=" << update_req->columns_cells->size_bytes() 
            << " avg="<< took/num_cells << "\n";
  }

  std::cout << "\n";

  
  select_req->result->columns[schema->cid]->free();
  spec =  select_req->specs.columns[0]->intervals[0];
  spec->free();
  spec->flags.offset=0;
  spec->flags.limit=0;
  took =  SWC::Time::now_ns();
  select_req->scan();
  select_req->wait();
  if(select_req->result->columns[schema->cid]->cells.size() != 0) {
    std::cerr << "BAD, select cells count: \n" 
              << " expected_value=0\n"
              << "   result_value=" << select_req->result->columns[schema->cid]->cells.size() << "\n";
    exit(1);
  }
  took = SWC::Time::now_ns() - took;
  std::cout << "SELECT-TOOK=" << took  << "\n";
  
  std::cout << "\n";
}



int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);
  
  SWC::Env::Clients::init(std::make_shared<SWC::client::Clients>(
    nullptr,
    std::make_shared<SWC::client::AppContext>()
  ));
  
  // PLAIN one version
  SWC::DB::SchemaPtr schema = SWC::DB::Schema::make(
    0, 
    "col-test-1", 
    SWC::Types::Column::PLAIN, 
    1, 0, 3, SWC::Types::Encoding::PLAIN, 64000000
  );

  // 1st DELETE & CREATE COLUMN
  
  // DELETE
  SWC::Protocol::Mngr::Req::ColumnMng::request(
    SWC::Protocol::Mngr::Req::ColumnMng::Func::DELETE,
    schema,
    [schema]
    (SWC::Protocol::Common::Req::ConnQueue::ReqBase::Ptr req_ptr, int err){
      if(err != SWC::Error::OK 
        && err != SWC::Error::COLUMN_SCHEMA_NAME_NOT_EXISTS) {
        req_ptr->request_again();
        return;
      }
       /**/
      // CREATE
      SWC::Protocol::Mngr::Req::ColumnMng::request(
        SWC::Protocol::Mngr::Req::ColumnMng::Func::CREATE,
        schema,
        [schema]
        (SWC::Protocol::Common::Req::ConnQueue::ReqBase::Ptr req_ptr, int err){
          if(err != SWC::Error::OK 
            && err != SWC::Error::COLUMN_SCHEMA_NAME_EXISTS) {
            req_ptr->request_again();
            return;
          }
           /**/
          expect_empty_column(
            SWC::Env::Clients::get()->schemas->get(err, schema->col_name)->cid
          );

          for(int i=0;i<10;i++) {
            test_1(schema->col_name);
            std::cout << "test_1 chk=" << i << "\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(10000));
          }
          finished = true;
        },
        10000
      );
  /*
  */
    },
    10000
  );
  
  

  while(!finished)
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  SWC::Env::IoCtx::io()->stop();
  std::cout << " ### EXIT ###\n";
  
  exit(0);

  /*

  

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

  */
}
