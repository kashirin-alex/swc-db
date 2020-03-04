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
  );

  gEnumExt blk_enc((int)Types::Encoding::DEFAULT);
  blk_enc.set_from_string(Types::from_string_encoding);
  blk_enc.set_repr(Types::repr_encoding);

  gEnumExt col_typ((int)Types::Column::PLAIN);
  col_typ.set_from_string(Types::from_string_col_type);
  col_typ.set_repr(Types::repr_col_type);

  cmdline_desc.add_options()
    ("gen-progress", i32(100000), 
      "display progress every N cells or 0 for quite") 

    ("gen-cells", i32(1000), 
      "number of cells, total=cells*versions*(key-tree? key-fractions : 1)")

    ("gen-key-fractions", i32(10), 
      "Number of Fractions per cell key") 
    ("gen-key-tree", boo(true), 
      "Key Fractions in a tree form [1], [1, 2] ") 
    ("gen-fraction-size", i32(10), 
      "fraction size in bytes at least") 

    ("gen-value-size", i32(256), 
      "cell value in bytes or counts for a col-counter")

    ("gen-col-name", str("load_generator"), "Gen. load column name") 
    ("gen-col-type", g_enum_ext(col_typ), 
     "Schema col-type PLAIN/COUNTER_I64/COUNTER_I32/COUNTER_I16/COUNTER_I8")  
    
    ("gen-cell-versions", i32(1), "cell key versions") 

    ("gen-cs-count", i8(0), "Schema cs-count")  
    ("gen-cs-size", i32(0), "Schema cs-size")    
    ("gen-cs-replication", i8(0), "Schema cs-replication")    
     
    ("gen-blk-size", i32(0), "Schema blk-size")    
    ("gen-blk-cells", i32(0), "Schema blk-cells")    
    ("gen-blk-encoding", g_enum_ext(blk_enc), 
     "Schema blk-encoding NONE/SNAPPY/ZLIB")  

    ("gen-compaction-percent", i8(0), 
     "Compaction threshold in % applied over size of either by cellstore or block")
  ;
}

void Settings::init_post_cmd_args() { }

} // namespace Config


void quit_error(int err) {
  if(!err)
    return;
  SWC_PRINT << "Error " << err << "(" << Error::get_text(err) << ")" 
            << SWC_PRINT_CLOSE;
  exit(1);
}

void display_stats(size_t took, size_t bytes, size_t cells_count) {      
  double took_base;
  double bytes_base;

  std::string time_base("n");
  if(took < 100000) {
    took_base = took;
  } else if(took < 10000000) { 
    took_base = (double)took/1000;
    time_base = "u";
  } else if(took <= 10000000000) { 
    took_base = (double)(took/1000)/1000;
    time_base = "m";
  } else if(took > 10000000000) {
    took_base = (double)(took/1000000)/1000;
    time_base = "";
  }

  std::string byte_base;
  if(bytes < 1000000) {
    bytes_base = bytes;
  } else if(bytes <= 1000000000) {
    bytes_base = (double)bytes/1000;
    byte_base = "K";
  } else if(bytes > 1000000000) {
    bytes_base = (double)(bytes/1000)/1000;
    byte_base = "M";
  }
  
  SWC_PRINT << "\n\nStatistics:\n"
    << " Total Time Took:        " << took_base << " " << time_base  << "s\n"
    << " Total Cells Count:      " << cells_count                    << "\n"
    << " Total Cells Size:       " << bytes_base << " " << byte_base << "B\n"
    << " Average Transfer Rate:  " << bytes_base/took_base 
                            << " " << byte_base << "B/" << time_base << "s\n" 
    << " Average Cells Rate:     " << (cells_count?cells_count/took_base:0)
                                            << " cell/" << time_base << "s" 
    << SWC_PRINT_CLOSE;
  ;

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

  
  bool is_counter = Types::is_counter(schema->col_type);
  std::string value_data;
  if(!is_counter) {
    uint8_t c=122;
    for(uint32_t n=0; n<value;++n)
      value_data += (char)(c == 122 ? c = 97 : ++c);
  }

  std::string fraction_value;
  uint64_t ts = Time::now_ns();
  uint64_t ts_progress = ts;
  size_t col_sz;
  //uint64_t key_count = 0;

  for(uint32_t v=0; v<versions; ++v) {
    for(uint32_t count=is_counter ? value : 1; count > 0; --count) {
      for(uint32_t i=0; i<cells; ++i) { 
        for(uint32_t f=(tree ? 1 : fractions); f<=fractions; ++f) {
          cell.key.free();

          for(uint32_t fn=0; fn<f; ++fn) {
            fraction_value = std::to_string(fn ? fn : i);
            for(uint32_t len = fraction_value.length(); 
                fraction_value.length() < fraction_size; 
                fraction_value.insert(0, "0")
            );
            cell.key.add(fraction_value);
          }
          if(is_counter)
            cell.set_counter(0, 1, schema->col_type);
          else
            cell.set_value(value_data);
          
          col->add(cell);

          added_count++;
          added_bytes += cell.encoded_length();

          req->commit_or_wait(col);

          if(progress && (added_count % progress) == 0) {
            ts_progress = Time::now_ns() - ts_progress;
            SWC_PRINT << " progress(cells=" << added_count 
                      << " bytes=" << added_bytes 
                      << " cell/ns=" << ts_progress/progress
                      << ")" << SWC_PRINT_CLOSE;
            ts_progress = Time::now_ns();
          }
        }
      }
    }
  }

  req->commit_if_need();
  req->wait();

  assert(added_count && added_bytes);
  
  display_stats(Time::now_ns() - ts, added_bytes, added_count);
}


void load_generator() {
  auto& props = Env::Config::settings()->properties;

  std::string col_name(props.get<std::string>("gen-col-name"));
  
  int err = Error::OK;
  auto schema = Env::Clients::get()->schemas->get(err, col_name);
  if(schema != nullptr) {
    quit_error(err);
    load_data(schema);
    return;
  }

  err = Error::OK;
  schema = SWC::DB::Schema::make(
    0, 
    col_name, 
    (Types::Column)props.get<gEnumExt>("gen-col-type").get(),
    props.get<int32_t>("gen-cell-versions"),
    0, // cell_ttl
    (Types::Encoding)props.get<gEnumExt>("gen-blk-encoding").get(),
    props.get<int32_t>("gen-blk-size"), 
    props.get<int32_t>("gen-blk-cells"),
    props.get<int8_t>("gen-cs-replication"), 
    props.get<int32_t>("gen-cs-size"), 
    props.get<int8_t>("gen-cs-count"),
    props.get<int8_t>("gen-compaction-percent") 
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

  schema = Env::Clients::get()->schemas->get(err, col_name);
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
