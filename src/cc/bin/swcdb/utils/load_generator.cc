/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */
 
#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"

#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"
#include "swcdb/db/client/Query/Select.h"
#include "swcdb/db/client/Query/Update.h"

#include "swcdb/common/Stats/FlowRate.h"

namespace SWC {
  
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
    ("gen-insert", boo(true),   "Generate new data") 
    ("gen-select", boo(false),  "Select generated data")
    ("gen-delete", boo(false),  "Delete generated data") 

    ("gen-select-empty", boo(false),  "Expect empty select results") 
    ("gen-delete-column", boo(false),  "Delete Column after") 

    ("gen-progress", i32(100000), 
      "display progress every N cells or 0 for quiet") 
    ("gen-cell-a-time", boo(false), "Write one cell at a time") 

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
        (int)DB::Types::KeySeq::LEXIC,
        0,
        DB::Types::from_string_range_seq,
        DB::Types::repr_range_seq
      ), 
     "Schema col-seq FC_+/LEXIC/VOLUME")  

    ("gen-col-type", 
      g_enum(
        (int)DB::Types::Column::PLAIN,
        0,
        DB::Types::from_string_col_type,
        DB::Types::repr_col_type
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
        (int)DB::Types::Encoder::DEFAULT,
        0,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding
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




namespace Utils { 


/**
 * @brief The SWC-DB Load-Generator C++ namespace 'SWC::Utils::LoadGenerator'
 *
 * \ingroup Applications
 */
namespace LoadGenerator {


void quit_error(int err) {
  if(!err)
    return;
  SWC_PRINT << "Error " << err << "(" << Error::get_text(err) << ")" 
            << SWC_PRINT_CLOSE;
  exit(1);
}


class CountIt {
  public:

  enum SEQ : uint8_t {
    REGULAR,
    REVERSE
  };

  CountIt(SEQ seq, ssize_t min, ssize_t max) 
          : seq(seq), min(min), max(max) {
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



void apply_key(ssize_t i, ssize_t f, uint32_t fraction_size, 
               DB::Cell::Key& key) {
  std::vector<std::string> fractions;
  fractions.resize(f);

  for(ssize_t fn=0; fn<f; ++fn) {
    std::string& fraction = fractions[fn];
    fraction.append(std::to_string(fn ? fn : i));
    for(; fraction.length() < fraction_size; fraction.insert(0, "0"));
  }
  key.free();
  key.add(fractions);
}

void apply_key(ssize_t i, ssize_t f, uint32_t fraction_size, 
               DB::Specs::Key& key, Condition::Comp comp) {
  key.free();
  key.resize(f);

  for(ssize_t fn=0; fn<f; ++fn) {
    DB::Specs::Fraction& fraction = key[fn];
    fraction.comp = comp;
    fraction.append(std::to_string(fn ? fn : i));
    for(; fraction.length() < fraction_size; fraction.insert(0, "0"));
  }
}


void update_data(DB::Schema::Ptr& schema, uint8_t flag) {
  auto settings = Env::Config::settings();

  uint32_t versions = flag == DB::Cells::INSERT 
    ? settings->get_i32("gen-cell-versions")
    : 1;

  uint32_t fractions = settings->get_i32("gen-key-fractions");
  uint32_t fraction_size = settings->get_i32("gen-fraction-size");
  
  bool tree = settings->get_bool("gen-key-tree");
  uint64_t cells = settings->get_i64("gen-cells");
  bool reverse = settings->get_bool("gen-reverse");
  auto seq = reverse ? CountIt::REVERSE : CountIt::REGULAR;
  
  uint32_t value = flag == DB::Cells::INSERT 
    ? settings->get_i32("gen-value-size")
    : 1;

  uint32_t progress = settings->get_i32("gen-progress");
  bool cellatime = settings->get_bool("gen-cell-a-time");

  auto req = std::make_shared<client::Query::Update>();
  req->columns->create(schema);

  auto col = req->columns->get_col(schema->cid);
  
  size_t added_count = 0;
  size_t resend_cells = 0;
  size_t added_bytes = 0;
  DB::Cells::Cell cell;
  cell.flag = flag;
  cell.set_time_order_desc(true);

  
  bool is_counter = DB::Types::is_counter(schema->col_type);
  std::string value_data;
  if(flag == DB::Cells::INSERT && !is_counter) {
    uint8_t c=122;
    for(uint32_t n=0; n<value;++n)
      value_data += (char)(c == 122 ? c = 97 : ++c);
  }

  uint64_t ts = Time::now_ns();
  uint64_t ts_progress = ts;
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

          apply_key(i, f, fraction_size, cell.key);

          if(flag == DB::Cells::INSERT) { 
            if(is_counter)
              cell.set_counter(0, 1, schema->col_type);
            else
              cell.set_value(value_data);
          }
          
          col->add(cell);

          ++added_count;
          added_bytes += cell.encoded_length();
          if(cellatime) {
            req->commit(col);
            req->wait();
          } else {
            req->commit_or_wait(col);
          }

          if(progress && !(added_count % progress)) {
            ts_progress = Time::now_ns() - ts_progress;
            SWC_PRINT 
              << "update-progress(time_ns=" <<  Time::now_ns()
              << " cells=" << added_count 
              << " bytes=" << added_bytes
              << " avg=" << ts_progress/progress << "ns/cell) ";
            req->result->profile.print(SWC_LOG_OSTREAM);
            SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;

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
  SWC_ASSERT(added_count && added_bytes);
  
  Common::Stats::FlowRate::Data rate(added_bytes, Time::now_ns() - ts);
  SWC_PRINT << std::endl << std::endl;
  rate.print_cells_statistics(SWC_LOG_OSTREAM, added_count, resend_cells);
  req->result->profile.display(SWC_LOG_OSTREAM);
  SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
}


void select_data(DB::Schema::Ptr& schema) {
  auto settings = Env::Config::settings();

  bool expect_empty = settings->get_bool("gen-select-empty");

  uint32_t versions = settings->get_i32("gen-cell-versions");
  uint32_t fractions = settings->get_i32("gen-key-fractions");
  uint32_t fraction_size = settings->get_i32("gen-fraction-size");
  
  bool tree = settings->get_bool("gen-key-tree");
  uint64_t cells = settings->get_i64("gen-cells");
  bool reverse = settings->get_bool("gen-reverse");
  auto seq = reverse ? CountIt::REVERSE : CountIt::REGULAR;
  
  uint32_t progress = settings->get_i32("gen-progress");
  bool cellatime = settings->get_bool("gen-cell-a-time");

  size_t select_count = 0;
  size_t select_bytes = 0;

  client::Query::Select::Ptr req;
  if(cellatime)
    req = std::make_shared<client::Query::Select>();
  else
    req = std::make_shared<client::Query::Select>(
      [&select_bytes, &select_count, cid=schema->cid] 
      (const client::Query::Select::Result::Ptr& result) {
        DB::Cells::Result cells;
        result->get_cells(cid, cells);
        select_count += cells.size();
        select_bytes += cells.size_bytes();
      },
      true
    );

  if(DB::Types::is_counter(schema->col_type))
    versions = 1;
  
  int err; 
  uint64_t ts = Time::now_ns();
  uint64_t ts_progress = ts;

  if(cellatime) {
    CountIt cell_num(seq, 0, cells);
    CountIt f_num(seq, (tree ? 1 : fractions), fractions+1);
    ssize_t i;
    ssize_t f;
    while(cell_num.next(&i)) {
      f_num.reset();
      while(f_num.next(&f)) {

        auto intval = DB::Specs::Interval::make_ptr();
        intval->set_opt__key_equal();
        auto& key_intval = intval->key_intervals.add();
        apply_key(i, f, fraction_size, key_intval->start, Condition::EQ);
        intval->flags.limit = versions;
        req->specs.columns = {
          DB::Specs::Column::make_ptr(schema->cid, {intval})
        };

        req->scan(err = Error::OK);
        SWC_ASSERT(!err);

        req->wait();

        SWC_ASSERT(
          expect_empty 
          ? req->result->empty()
          : req->result->get_size(schema->cid) == versions
        );

        select_bytes += req->result->get_size_bytes();
        ++select_count;
        req->result->free(schema->cid);

        if(progress && !(select_count % progress)) {
          ts_progress = Time::now_ns() - ts_progress;
          SWC_PRINT 
            << "select-progress(time_ns=" << Time::now_ns()
            << " cells=" << select_count
            << " avg=" << ts_progress/progress << "ns/cell) ";
          req->result->profile.print(SWC_LOG_OSTREAM);
          SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;

          ts_progress = Time::now_ns();
        }
      }
    }
    if(expect_empty)
      select_count = 0;

  } else {
    req->specs.columns = { DB::Specs::Column::make_ptr(
      schema->cid, { DB::Specs::Interval::make_ptr() }
    )};
    req->scan(err = Error::OK);
    SWC_ASSERT(!err);

    req->wait();
    SWC_ASSERT(
      expect_empty
      ? req->result->empty()
      : select_count == versions * (tree ? fractions : 1) * cells
    );
  }

  Common::Stats::FlowRate::Data rate(select_bytes, Time::now_ns() - ts);
  SWC_PRINT << std::endl << std::endl;
  rate.print_cells_statistics(SWC_LOG_OSTREAM, select_count, 0);
  req->result->profile.display(SWC_LOG_OSTREAM);
  SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
}


void make_work_load(DB::Schema::Ptr& schema) {
  auto settings = Env::Config::settings();

  if(settings->get_bool("gen-insert"))
    update_data(schema, DB::Cells::INSERT);
    
  if(settings->get_bool("gen-select"))
    select_data(schema);

  if(settings->get_bool("gen-delete"))
    update_data(schema, DB::Cells::DELETE);

  if(settings->get_bool("gen-delete-column")) { 
    std::promise<int>  res;
    Comm::Protocol::Mngr::Req::ColumnMng::request(
      Comm::Protocol::Mngr::Req::ColumnMng::Func::DELETE,
      schema,
      [await=&res]
      (const Comm::client::ConnQueue::ReqBase::Ptr&, int err) {
        await->set_value(err);
      },
      10000
    );
    quit_error(res.get_future().get());
  }
    
}


void generate() {
  auto settings = Env::Config::settings();

  std::string col_name(settings->get_str("gen-col-name"));
  
  int err = Error::OK;
  auto schema = Env::Clients::get()->schemas->get(err, col_name);
  if(schema) {
    quit_error(err);
    make_work_load(schema);
    return;
  }
  err = Error::OK;

  schema = DB::Schema::make();
  schema->col_name = col_name;
  schema->col_seq = (DB::Types::KeySeq)settings->get_genum("gen-col-seq");
  schema->col_type = (DB::Types::Column)settings->get_genum("gen-col-type");
  schema->cell_versions = settings->get_i32("gen-cell-versions");
  schema->cell_ttl = 0;
  schema->blk_encoding = (DB::Types::Encoder)settings->get_genum("gen-blk-encoding");
  schema->blk_size = settings->get_i32("gen-blk-size");
  schema->blk_cells = settings->get_i32("gen-blk-cells");
  schema->cs_replication = settings->get_i8("gen-cs-replication");
  schema->cs_size = settings->get_i32("gen-cs-size");
  schema->cs_max = settings->get_i8("gen-cs-count");
  schema->log_rollout_ratio = settings->get_i8("gen-log-rollout");
  schema->compact_percent = settings->get_i8("gen-compaction-percent");

  // CREATE COLUMN
  std::promise<int>  res;
  Comm::Protocol::Mngr::Req::ColumnMng::request(
    Comm::Protocol::Mngr::Req::ColumnMng::Func::CREATE,
    schema,
    [await=&res]
    (const Comm::client::ConnQueue::ReqBase::Ptr&, int err) {
      await->set_value(err);
    },
    10000
  );
  quit_error(res.get_future().get());

  schema = Env::Clients::get()->schemas->get(err, col_name);
  quit_error(err);
  make_work_load(schema);
}


}}} // namespace SWC::Utils::LoadGenerator



int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);
  
  SWC::Env::Clients::init(
    std::make_shared<SWC::client::Clients>(
      nullptr, //Env::IoCtx::io()->shared(),
      nullptr, // std::make_shared<client::ManagerContext>()
      nullptr  // std::make_shared<client::RangerContext>()
    )
  );

  SWC::Utils::LoadGenerator::generate();

  SWC::Env::IoCtx::io()->stop();

  return 0;
}
