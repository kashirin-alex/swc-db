/*
 * Copyright (C) 2020 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */
 
#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"

#include "swcdb/client/Clients.h"
#include "swcdb/client/AppContext.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"
#include "swcdb/db/Protocol/Common/req/Query.h"


namespace SWC{ 
  
namespace Config {
void Settings::init_app_options() {
  cmdline_desc.get_default("logging-level") 
    ->get_ptr<gEnumExt>()->set_value(LOG_ERROR); // default level

  init_comm_options();
  init_client_options();
  
  cmdline_desc.definition(usage_str(
    "SWC-DB(load_generator) Usage: swcdb_load_generator [options]\n\nOptions:")
  )
  .add_options()
    ("gen-progress", i32(100000), 
      "display progress every N cells or 0 for quite") 
    ("gen-col-counter", boo(false), 
      "Counter column") 
    ("gen-cell-versions", i32(1), 
      "cell key versions") 
    ("gen-key-fractions", i32(10), 
      "Number of Fractions per cell key") 
    ("gen-key-tree", boo(true), 
      "Key Fractions in a tree form [1], [1, 2] ") 
    ("gen-fraction-size", i32(10), 
      "fraction size in bytes at least") 
    ("gen-cells", i32(1000), 
      "number of cells, total=cells*versions*(key-tree? key-fractions : 1)")
    ("gen-value-size", i32(256), 
      "cell value in bytes or counts for a col-counter")
  ;
}

void Settings::init_post_cmd_args() { }

} // namespace Config


void quit_error(int err) {
  if(!err)
    return;
  std::cout << "Error " << err << "(" << Error::get_text(err) << ")\n";
  exit(1);
}


void load_data(DB::Schema::Ptr schema) {
  auto& props = Env::Config::settings()->properties;

  uint32_t versions = props.get<int32_t>("gen-cell-versions");
  uint32_t fractions = props.get<int32_t>("gen-key-fractions");
  uint32_t fraction_size = props.get<int32_t>("gen-fraction-size");
  
  bool tree = props.get<bool>("gen-key-tree");
  uint32_t cells = props.get<int32_t>("gen-cells");
  uint32_t value = props.get<int32_t>("gen-value-size");
  uint32_t progress = props.get<int32_t>("gen-progress");

  auto req = std::make_shared<Protocol::Common::Req::Query::Update>();
  req->columns->create(schema);

  auto col = req->columns->get_col(schema->cid);
  
  size_t added_count = 0;
  size_t added_bytes = 0;
  DB::Cells::Cell cell;
  cell.flag = DB::Cells::INSERT;
  cell.set_time_order_desc(true);

  
  bool is_counter = props.get<bool>("gen-col-counter");
  std::string value_data;
  if(is_counter) {
    value_data = "+1";
  } else {
    uint8_t c=122;
    for(uint32_t n=0; n<value;++n)
      value_data += (char)(c == 122 ? c = 97 : ++c);
  }

  std::string fraction_value;
  uint64_t ts = Time::now_ns();
  size_t col_sz;
  //uint64_t key_count = 0;

  for(uint32_t v=0; v<versions; ++v) {  
    for(uint32_t count=is_counter ? value : 1; count > 0; --count) {
      for(uint32_t i=0; i<cells; ++i) { 
          
        for(uint32_t f=(tree ? 1 : fractions); f<=fractions; ++f) {
          cell.key.free();

          for(uint32_t fn=0; fn<f; ++fn) {
            fraction_value = std::to_string(fn ? fn : added_count);
            for(uint32_t len = fraction_value.length(); 
                fraction_value.length() < fraction_size; 
                fraction_value.insert(0, "0")
            );
            cell.key.add(fraction_value);
          }
      
          cell.set_value(value_data);
              
          col->add(cell);

          added_count++;
          added_bytes += cell.encoded_length();

          col_sz = col->size_bytes();
          if(req->result->completion && col_sz > req->buff_sz*3)
            req->wait();
          if(!req->result->completion && col_sz >= req->buff_sz)
            req->commit(col);

          if(progress && (added_count % progress) == 0)
            std::cout << " progress(cells=" << added_count 
                      << " bytes=" << added_bytes << ")\n";
        }
      }
    }
  }

  if(col->size_bytes() && !req->result->completion)
    req->commit(col);  
  req->wait();

  assert(added_count && added_bytes);

  auto took = Time::now_ns() - ts;
  std::cout << "Statistics: \n"
            << " added:      " << added_count             << " cells\n"
            << " added:      " << added_bytes             << " bytes\n"
            << " avg. cell:  " << added_bytes/added_count << " cell/bytes\n"
            << " time:       " << took                    << " ns total \n"
            << " avg. time:  " << took/added_count        << " ns/cell\n"
            << " avg. bytes: " << took/added_bytes        << " bytes/ns\n"
            << "";
}


void load_generator() {
  auto& props = Env::Config::settings()->properties;

  int err = Error::OK;
  auto schema = Env::Clients::get()->schemas->get(err, "load_generator");
  if(schema != nullptr) {
    quit_error(err);
    load_data(schema);
    return;
  }

  err = Error::OK;
  schema = SWC::DB::Schema::make(
    0, 
    "load_generator", 
    props.get<bool>("gen-col-counter") 
      ? Types::Column::COUNTER_I64 
      : Types::Column::PLAIN,
    props.get<int32_t>("gen-cell-versions"),
    0, // cell_ttl
    SWC::Types::Encoding::DEFAULT,
    0, // blk-size
    0, // blk-cells
    3, // cs_replication
    0, // cs-size
    2, // cs-max
    0 // compact-%
  );


  // CREATE COLUMN
  std::promise<int>  res;
  Protocol::Mngr::Req::ColumnMng::request(
    Protocol::Mngr::Req::ColumnMng::Func::CREATE,
    schema,
    [await=&res]
    (Protocol::Common::Req::ConnQueue::ReqBase::Ptr req_ptr, int err) {
      await->set_value(err);
    },
    10000
  );
  quit_error(res.get_future().get());

  schema = Env::Clients::get()->schemas->get(err, "load_generator");
  quit_error(err);
  load_data(schema);
}

} // namespace SWC



int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);
  
  SWC::Env::Clients::init(
    std::make_shared<SWC::client::Clients>(
      nullptr,
      std::make_shared<SWC::client::AppContext>()
    )
  );

  SWC::load_generator();

  SWC::Env::IoCtx::io()->stop();

  return 0;
}
