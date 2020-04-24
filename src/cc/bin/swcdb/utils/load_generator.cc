/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */
 
#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"

#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/AppContext.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"
#include "swcdb/db/client/Query/Select.h"

#include "swcdb/core/FlowRate.h"

namespace SWC{ 
  
namespace Config {
void Settings::init_app_options() {
  ((Property::V_GENUM*) cmdline_desc.get_default("swc.logging.level") 
    )->set(LOG_ERROR); // default level

  init_comm_options();
  init_client_options();
  
  cmdline_desc.definition(usage_str(
    "SWC-DB(load_generator) Usage: swcdb_load_generator [options]\n\nOptions:")
  );

  cmdline_desc.add_options()
    ("gen-progress", i32(100000), 
      "display progress every N cells or 0 for quite") 

    ("gen-cells", i64(1000), 
      "number of cells, total=cells*versions*(key-tree? key-fractions : 1)")

    ("gen-key-fractions", i32(10), 
      "Number of Fractions per cell key") 
    ("gen-key-tree", boo(true), 
      "Key Fractions in a tree form [1], [1, 2] ") 
    ("gen-fraction-size", i32(10), 
      "fraction size in bytes at least") 
    ("gen-reverse", boo(false), 
      "Generate in reverse, always writes to 1st range") 

    ("gen-value-size", i32(256), 
      "cell value in bytes or counts for a col-counter")

    ("gen-col-name", str("load_generator"), "Gen. load column name") 
    
    ("gen-col-seq", 
      g_enum(
        (int)Types::KeySeq::BITWISE,
        0,
        Types::from_string_range_seq,
        Types::repr_range_seq
      ), 
     "Schema col-seq BITWISE/VOLUME/+_FCOUNT")  

    ("gen-col-type", 
      g_enum(
        (int)Types::Column::PLAIN,
        0,
        Types::from_string_col_type,
        Types::repr_col_type
      ), 
     "Schema col-type PLAIN/COUNTER_I64/COUNTER_I32/COUNTER_I16/COUNTER_I8")  
    
    ("gen-cell-versions", i32(1), "cell key versions") 

    ("gen-cs-count", i8(0), "Schema cs-count")  
    ("gen-cs-size", i32(0), "Schema cs-size")    
    ("gen-cs-replication", i8(0), "Schema cs-replication")    
     
    ("gen-blk-size", i32(0), "Schema blk-size")    
    ("gen-blk-cells", i32(0), "Schema blk-cells")    
    ("gen-blk-encoding", 
      g_enum(
        (int)Types::Encoding::DEFAULT,
        0,
        Types::from_string_encoding,
        Types::repr_encoding
      ), 
     "Schema blk-encoding NONE/ZSTD/SNAPPY/ZLIB")  

    ("gen-log-rollout", i8(0), 
     "CommitLog rollout block ratio")
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

void display_stats(size_t took, size_t bytes, 
                   size_t cells_count, size_t resend_cells) {  
    FlowRate::Data rate(bytes, took);
    SWC_PRINT;
    rate.print_cells_statistics(std::cout, cells_count, resend_cells);
    std::cout << SWC_PRINT_CLOSE;
}

class CountIt {
  public:

  enum SEQ {
    REGULAR,
    REVERSE
  };

  CountIt(SEQ seq, ssize_t min, ssize_t max) 
          : min(min), max(max), seq(seq) {
    reset();
  }
  
  bool next(ssize_t* nxt) {
    switch(seq) {
      case REVERSE: {
        if(pos == min)
          return false;
        *nxt = --pos;
        return true;
      }
      default: {
        if(pos == max)
          return false;
        *nxt = pos;
        ++pos;
        return true;
      }
    }
  }

  void reset() {
    switch(seq) {
      case REVERSE: {
        pos = max;
        break;
      } 
      default: {
        pos = min;
        break;
      }
    }
  }

  SEQ     seq;
  ssize_t min; 
  ssize_t max;
  ssize_t pos; 
};

void load_data(DB::Schema::Ptr schema) {
  auto settings = Env::Config::settings();

  uint32_t versions = settings->get_i32("gen-cell-versions");
  uint32_t fractions = settings->get_i32("gen-key-fractions");
  uint32_t fraction_size = settings->get_i32("gen-fraction-size");
  
  bool tree = settings->get_bool("gen-key-tree");
  uint64_t cells = settings->get_i64("gen-cells");
  bool reverse = settings->get_bool("gen-reverse");
  auto seq = reverse ? CountIt::REVERSE : CountIt::REGULAR;
  
  uint32_t value = settings->get_i32("gen-value-size");
  uint32_t progress = settings->get_i32("gen-progress");

  auto req = std::make_shared<client::Query::Update>();
  req->columns->create(schema);

  auto col = req->columns->get_col(schema->cid);
  
  size_t added_count = 0;
  size_t resend_cells = 0;
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
  CountIt cell_num(seq, 0, cells);
  CountIt f_num(seq, (tree ? 1 : fractions), fractions+1);
  ssize_t i;
  ssize_t f;

  for(uint32_t v=0; v<versions; ++v) {
    for(uint32_t count=is_counter ? value : 1; count > 0; --count) {
      cell_num.reset();
      while(cell_num.next(&i)) {
        f_num.reset();
        while(f_num.next(&f)) {

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

          ++added_count;
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
        resend_cells += req->result->get_resend_count();
      }
    }
  }

  req->commit_if_need();
  req->wait();

  resend_cells += req->result->get_resend_count();
  assert(added_count && added_bytes);
  
  display_stats(Time::now_ns() - ts, added_bytes, added_count, resend_cells);
}


void load_generator() {
  auto settings = Env::Config::settings();

  std::string col_name(settings->get_str("gen-col-name"));
  
  int err = Error::OK;
  auto schema = Env::Clients::get()->schemas->get(err, col_name);
  if(schema != nullptr) {
    quit_error(err);
    load_data(schema);
    return;
  }
  err = Error::OK;

  schema = SWC::DB::Schema::make();
  schema->col_name = col_name;
  schema->col_seq = (Types::KeySeq)settings->get_genum("gen-col-seq");
  schema->col_type = (Types::Column)settings->get_genum("gen-col-type");
  schema->cell_versions = settings->get_i32("gen-cell-versions");
  schema->cell_ttl = 0;
  schema->blk_encoding = (Types::Encoding)settings->get_genum("gen-blk-encoding");
  schema->blk_size = settings->get_i32("gen-blk-size");
  schema->blk_cells = settings->get_i32("gen-blk-cells");
  schema->cs_replication = settings->get_i8("gen-cs-replication");
  schema->cs_size = settings->get_i32("gen-cs-size");
  schema->cs_max = settings->get_i8("gen-cs-count");
  schema->log_rollout_ratio = settings->get_i8("gen-log-rollout");
  schema->compact_percent = settings->get_i8("gen-compaction-percent");

  // CREATE COLUMN
  std::promise<int>  res;
  Protocol::Mngr::Req::ColumnMng::request(
    Protocol::Mngr::Req::ColumnMng::Func::CREATE,
    schema,
    [await=&res]
    (client::ConnQueue::ReqBase::Ptr req_ptr, int err) {
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
