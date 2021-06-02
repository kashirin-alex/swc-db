/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"

#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/Query/Select/Scanner.h"
#include "swcdb/db/client/Query/Update/Committer.h"
#include "swcdb/db/Cells/CellValueSerialFields.h"
#include "swcdb/db/client/Query/Select/Handlers/Common.h"
#include "swcdb/db/client/Query/Update/Handlers/Common.h"

#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/db/Protocol/Bkr/req/ColumnMng.h"

namespace SWC {


namespace Config {

void Settings::init_app_options(){
  init_comm_options();
  init_client_options();

  cmdline_desc.add_options()
    ("with-broker", boo(false)->zero_token(),
     "Query applicable requests with Broker")

    ("ncells", i64(1000), "number of cells, total=cells*(counter||versions)")
    ("nfractions", i32(26), "Number of Fractions per cell key")

    ("value-size", i32(256), "value in bytes or counts for a counter")

    ("col-seq",
      g_enum(
        int(DB::Types::KeySeq::LEXIC),
        nullptr,
        DB::Types::from_string_range_seq,
        DB::Types::repr_range_seq
      ),
     "Schema col-seq FC_+/LEXIC/VOLUME")

    ("col-type",
      g_enum(
        int(DB::Types::Column::PLAIN),
        nullptr,
        DB::Types::from_string_col_type,
        DB::Types::repr_col_type
      ),
     "Schema col-type PLAIN/COUNTER_I64/COUNTER_I32/COUNTER_I16/COUNTER_I8")

    ("cell-versions", i32(1), "cell key versions")

    /*
    ("cell-enc",
      g_enum(
        int(DB::Types::Encoder::PLAIN),
        0,
        Core::Encoder::from_string_encoding,
        Core::Encoder::repr_encoding
      ),
     "Cell's Value encoding ZSTD/SNAPPY/ZLIB")
     */
  ;
}
void Settings::init_post_cmd_args() { }

}


class Test {
  public:

  std::mutex                mutex;
  std::condition_variable   cv;

  bool                      with_broker;

  DB::Types::Column         col_type;
  DB::Types::KeySeq         col_seq;
  std::string               col_name;
  uint32_t                  cell_versions;

  size_t                    ncells;
  uint32_t                  nfractions;
  DB::Types::Encoder        cell_enc = DB::Types::Encoder::PLAIN;

  bool                      runs;
  DB::Schema::Ptr           schema;
  bool                      counter;

  size_t                    time_select;

  void run() {
    counter = DB::Types::is_counter(col_type);
    time_select = 0;

    SWC_PRINT << "Test::run "
              << " col_type=" << DB::Types::to_string(col_type)
              << " col_seq=" << DB::Types::to_string(col_seq)
              << " col_name=" << col_name
              << " cell_versions=" << cell_versions
              << " ncells=" << ncells
              << " nfractions=" << nfractions
              << SWC_PRINT_CLOSE;

    runs = true;
    create_column();

    Core::UniqueLock lock_wait(mutex);
    cv.wait(lock_wait, [this]{ return !runs; });
  }

  void create_column() {
    SWC_LOG(LOG_DEBUG, "create_column");

    int err = Error::OK;
    if((schema = Env::Clients::get()->get_schema(err, col_name)))
      return delete_column([this](){ create_column(); });

    schema = DB::Schema::make();
    schema->col_type = col_type;
    schema->col_name = col_name;
    schema->cid = std::hash<std::string>()(col_name);
    schema->col_seq = col_seq;
    schema->cs_size = 200000000;
    schema->blk_cells = 10000;
    schema->cell_versions = counter ? 1 : cell_versions;

    auto _cb = [this]
      (Comm::client::ConnQueue::ReqBase::Ptr req_ptr, int err) {
        if(err && err != Error::COLUMN_SCHEMA_NAME_EXISTS) {
          SWC_PRINT << "ColumnMng::CREATE err="
                    << err << "(" << Error::get_text(err) << ")"
                    << SWC_PRINT_CLOSE;
          return req_ptr->request_again();
        }
        schema = Env::Clients::get()->get_schema(err, col_name);
        query_insert();
      };

    auto func = Comm::Protocol::Mngr::Params::ColumnMng::Function::CREATE;
    with_broker
      ? Comm::Protocol::Bkr::Req::ColumnMng::request(
          Env::Clients::get(), func, schema, std::move(_cb), 10000)
      : Comm::Protocol::Mngr::Req::ColumnMng::request(
          Env::Clients::get(), func, schema, std::move(_cb), 10000);
  }

  void delete_column(const std::function<void()>& cb) {
    SWC_LOG(LOG_DEBUG, "delete_column");

    auto _cb = [this, cb]
      (Comm::client::ConnQueue::ReqBase::Ptr req_ptr, int err) {
        if(err && err != Error::COLUMN_SCHEMA_NAME_NOT_EXISTS) {
          SWC_PRINT << "ColumnMng::DELETE err="
                    << err << "(" << Error::get_text(err) << ")"
                    << SWC_PRINT_CLOSE;
          return req_ptr->request_again();
        }

        Env::Clients::get()->schemas.remove(schema->cid);
        schema = nullptr;
        cb();
      };

    auto func = Comm::Protocol::Mngr::Params::ColumnMng::Function::DELETE;
    with_broker
      ? Comm::Protocol::Bkr::Req::ColumnMng::request(
          Env::Clients::get(), func, schema, std::move(_cb), 10000)
      : Comm::Protocol::Mngr::Req::ColumnMng::request(
          Env::Clients::get(), func, schema, std::move(_cb), 10000);
  }


  void expect_empty_column() {
    SWC_LOG(LOG_DEBUG, "expect_empty_column");

    auto hdlr = client::Query::Select::Handlers::Common::make(
      Env::Clients::get(),
      nullptr,
      false,
      nullptr,
      with_broker
        ? client::Clients::BROKER
        : client::Clients::DEFAULT
    );

    auto intval = DB::Specs::Interval::make_ptr();
    intval->flags.offset = 0;
    intval->flags.limit = 1;
    DB::Specs::Scan specs;
    specs.columns.push_back(
      DB::Specs::Column::make_ptr(schema->cid, {intval}));

    int err = Error::OK;
    hdlr->scan(err, specs);
    if(err) {
      SWC_PRINT << "expect_empty_column err="
                << err << "(" << Error::get_text(err) << ")"
                << SWC_PRINT_CLOSE;
    }
    SWC_ASSERT(!err);

    hdlr->wait();
    SWC_ASSERT(!hdlr->get_size(schema->cid));

    SWC_PRINT << "expect_empty_column:  \n";
    hdlr->profile.print(SWC_LOG_OSTREAM);
    SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
  }

  void expect_one_at_offset() {
    SWC_LOG(LOG_DEBUG, "expect_one_at_offset");

    auto hdlr = client::Query::Select::Handlers::Common::make(
      Env::Clients::get(),
      nullptr,
      false,
      nullptr,
      with_broker
        ? client::Clients::BROKER
        : client::Clients::DEFAULT
    );

    auto intval = DB::Specs::Interval::make_ptr();
    intval->flags.offset = ncells * nfractions - 1;
    intval->flags.max_versions = 1;
    DB::Specs::Scan specs;
    specs.columns.push_back(
      DB::Specs::Column::make_ptr(schema->cid, {intval}));

    int err = Error::OK;
    hdlr->scan(err, std::move(specs));
    SWC_ASSERT(!err);

    hdlr->wait();
    if(hdlr->get_size(schema->cid) != 1) {
      SWC_PRINT << "get_size: " << hdlr->get_size(schema->cid) << SWC_PRINT_CLOSE;
      SWC_ASSERT(hdlr->get_size(schema->cid) == 1);
    }

    SWC_PRINT << "expect_one_at_offset:  \n";
    hdlr->profile.print(SWC_LOG_OSTREAM);
    SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
  }


  void apply_cell_key(DB::Cell::Key& key, uint32_t i, uint32_t f) {
    key.free();
    std::string cell_number(std::to_string(i));
    for(uint32_t chr=0; chr<=f; ++chr)
      key.add(char(uint8_t(chr+97))+cell_number);
  }

  void apply_cell_value(DB::Cells::Cell& cell) {
    std::string value = "V_OF:(" + cell.key.to_string() + "):(";
    for(uint32_t chr=0; chr<=255; ++chr)
      value += char(chr);
    value += ")END";

    uint8_t* data;
    uint32_t size;

    DB::Cell::Serial::Value::FieldsWriter wfields;
    if(col_type == DB::Types::Column::SERIAL) {
      wfields.ensure(value.size() * 10);
      auto t = DB::Cell::Serial::Value::Type::INT64; // roundrobin Type
      for(auto it = value.begin(); it != value.end(); ++it) {
        if(t == DB::Cell::Serial::Value::Type::INT64) {
          wfields.add(int64_t(*it));
          t = DB::Cell::Serial::Value::Type::DOUBLE;
        } else if(t == DB::Cell::Serial::Value::Type::DOUBLE) {
          long double v(*it);
          wfields.add(v);
          t = DB::Cell::Serial::Value::Type::BYTES;
        } else if(t == DB::Cell::Serial::Value::Type::BYTES) {
          const uint8_t c = *it;
          wfields.add(&c, 1);
          t = DB::Cell::Serial::Value::Type::INT64;
        }
      }
      data = wfields.base;
      size = wfields.fill();

    } else {
      data = reinterpret_cast<uint8_t*>(value.data());
      size = value.size();
    }

    if(cell_enc == DB::Types::Encoder::PLAIN) { // roundrobin encoding types
      cell.set_value(data, size, true);
      cell_enc = DB::Types::Encoder::ZLIB;

    } else if(cell_enc == DB::Types::Encoder::ZLIB) {
      cell.set_value(cell_enc, data, size);
      cell_enc = DB::Types::Encoder::SNAPPY;

    } else if(cell_enc == DB::Types::Encoder::SNAPPY) {
      cell.set_value(cell_enc, data, size);
      cell_enc = DB::Types::Encoder::ZSTD;

    } else  if(cell_enc == DB::Types::Encoder::ZSTD) {
      cell.set_value(cell_enc, data, size);
      cell_enc = DB::Types::Encoder::PLAIN;
    }
  }



  void query_insert() {
    SWC_LOG(LOG_DEBUG, "query_insert");

    expect_empty_column();

    auto hdlr = client::Query::Update::Handlers::Common::make(
      Env::Clients::get(),
      [this](const client::Query::Update::Handlers::Common::Ptr& _hdlr) {
        SWC_PRINT << "query_insert: \n";
        _hdlr->profile.print(SWC_LOG_OSTREAM);
        SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;

        SWC_ASSERT(!_hdlr->error());
        expect_one_at_offset();
        query_select(0, 0);
      },
      nullptr,
      with_broker
        ? client::Clients::BROKER
        : client::Clients::DEFAULT
    );

    hdlr->completion.increment();


    auto& col = hdlr->create(schema);

    DB::Cells::Cell cell;
    cell.flag = DB::Cells::INSERT;
    cell.set_time_order_desc(true);

    for(uint32_t i=0; i<ncells; ++i) {
      for(uint32_t v=0; v<cell_versions; ++v) {
        for(uint32_t f=0; f<nfractions; ++f) {
          apply_cell_key(cell.key, i, f);

          if(counter) {
            cell.set_counter(0, 1);
          } else {
            apply_cell_value(cell);
          }
          col->add(cell);
          hdlr->commit_or_wait(col.get(), 1);
        }
      }
    }
    hdlr->response(Error::OK);
  }


  void query_select(uint32_t i, uint32_t f) {
    SWC_LOG(LOG_DEBUG, "query_select");

    if(f == nfractions) {
      f = 0;
      ++i;
    }
    if(i == ncells && !f) {
        SWC_PRINT << "query_select: \n"
                  << " time_select=" << time_select << "ns"
                  << " cells=" << (ncells*nfractions)
                  << " avg=" << time_select/(ncells*nfractions) << "ns\n"
                  << SWC_PRINT_CLOSE;
      return query_delete();
    }

    DB::Cell::Key key;
    apply_cell_key(key, i, f);

    auto hdlr = client::Query::Select::Handlers::Common::make(
      Env::Clients::get(),
      [this, ts=Time::now_ns(), key=DB::Cell::Key(key, true), i, f]
      (const client::Query::Select::Handlers::Common::Ptr& hdlr) {
        time_select += Time::now_ns() - ts;

        SWC_LOG_OUT(LOG_DEBUG,
          SWC_LOG_OSTREAM << "query_select:  \n";
          hdlr->profile.print(SWC_LOG_OSTREAM);
        );

        SWC_ASSERT(!hdlr->state_error);
        SWC_ASSERT(hdlr->get_size(schema->cid) == counter?1:cell_versions);

        DB::Cells::Result cells;
        hdlr->get_cells(schema->cid, cells);

        for(auto cell : cells) {
          SWC_ASSERT(key.equal(cell->key));
          if(counter)
            SWC_ASSERT(cell->get_counter() == cell_versions);
        }

        query_select(i, f + 1);
      },
      false,
      nullptr,
      with_broker
        ? client::Clients::BROKER
        : client::Clients::DEFAULT
    );

    DB::Specs::Interval intval;
    auto& key_intval = intval.key_intervals.add();
    key_intval.start.set(key, Condition::EQ);
    intval.flags.offset = 0;
    intval.flags.limit = counter ? 1 : cell_versions;
    SWC_LOG(LOG_DEBUG, intval.to_string());

    hdlr->scan(schema, std::move(intval));
  }


  void query_delete() {
    SWC_LOG(LOG_DEBUG, "query_delete");

    auto hdlr = client::Query::Update::Handlers::Common::make(
      Env::Clients::get(),
      [this](const client::Query::Update::Handlers::Common::Ptr& hdlr) {
        SWC_PRINT << "query_delete: \n";
        hdlr->profile.print(SWC_LOG_OSTREAM);
        SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;

        SWC_ASSERT(!hdlr->error());
        expect_empty_column();

        delete_column([this]() noexcept {
          {
            Core::ScopedLock lock(mutex);
            runs = false;
          }
          cv.notify_all();
        });
      },
      nullptr,
      with_broker
        ? client::Clients::BROKER
        : client::Clients::DEFAULT
    );

    auto& col = hdlr->create(schema);

    DB::Cells::Cell cell;
    cell.flag = DB::Cells::DELETE;
    cell.set_time_order_desc(true);

    for(uint32_t i=0; i<ncells; ++i) {
      for(uint32_t f=0; f<nfractions; ++f) {
        apply_cell_key(cell.key, i, f);
        col->add(cell);
        hdlr->commit_or_wait(col.get());
      }
    }
    hdlr->commit_if_need();
  }
};


}

int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);

  SWC::Env::Clients::init(
    (SWC::Env::Config::settings()->get_bool("with-broker")
      ? SWC::client::Clients::make(
          *SWC::Env::Config::settings(),
          SWC::Comm::IoContext::make("Clients", 8),
          nullptr  // std::make_shared<client::BrokerContext>()
        )
      : SWC::client::Clients::make(
          *SWC::Env::Config::settings(),
          SWC::Comm::IoContext::make("Clients", 8),
          nullptr, // std::make_shared<client::ManagerContext>()
          nullptr  // std::make_shared<client::RangerContext>()
        )
    )->init()
  );

  SWC_ASSERT(
    !SWC::Env::Config::settings()->get_bool("with-broker") ||
    SWC::Env::Clients::get()->brokers.has_endpoints()
  );

  auto settings = SWC::Env::Config::settings();

  SWC::Test test;
  test.with_broker = settings->get_bool("with-broker");

  test.col_type = SWC::DB::Types::Column(settings->get_genum("col-type"));
  test.col_seq = SWC::DB::Types::KeySeq(settings->get_genum("col-seq"));

  test.ncells = settings->get_i64("ncells");
  test.nfractions = settings->get_i32("nfractions");
  test.cell_versions = settings->get_i32("cell-versions");
  //uint32_t value = settings->get_i32("value-size");

  //test.cell_enc = SWC::DB::Types::Encoder(settings->get_genum("cell-enc"));
  test.col_name = "test-"
                + std::string(SWC::DB::Types::to_string(test.col_type))
                + "-"
                + std::string(SWC::DB::Types::to_string(test.col_seq))
                + "-"
                + std::string(SWC::Core::Encoder::to_string(test.cell_enc))
                + "-v"
                + std::to_string(test.cell_versions)
                + "-c"
                + std::to_string(test.ncells)
                + "-f"
                + std::to_string(test.nfractions)
                + "-bkr"
                + std::to_string(test.with_broker);
  test.run();

  SWC::Env::Clients::get()->stop();
  std::cout << " ### EXIT ###\n";

  exit(0);
}
