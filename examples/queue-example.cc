/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnMng_Sync.h"
#include "swcdb/db/Protocol/Bkr/req/ColumnMng_Sync.h"
#include "swcdb/db/client/Query/Update/Handlers/Common.h"
#include "swcdb/db/client/Query/Select/Handlers/Common.h"



namespace Examples {

SWC::Core::AtomicBool continue_running(true);

SWC::DB::Schema::Ptr create_column() {
  int err = SWC::Error::OK;
  auto schema = SWC::Env::Clients::get()->get_schema(err, "queue_example");
  if(!err && schema)
    return schema;

  schema = SWC::DB::Schema::make();
  schema->col_name = "queue_example";
  schema->col_seq = SWC::DB::Types::KeySeq::VOLUME;
  schema->col_type = SWC::DB::Types::Column::PLAIN;
  schema->cell_versions = 1;
  schema->cell_ttl = 0;
  /* by default ranger-cfg
  schema->cs_size = 209715200;
  schema->blk_encoding = SWC::DB::Types::Encoder::ZSTD;
  schema->blk_size = 33554432;
  schema->blk_cells = 100000;
  schema->cs_replication = 1;
  schema->cs_max = 2;
  schema->log_rollout_ratio = 10;
  schema->compact_percent = 33;
  */

  // CREATE COLUMN
  err = SWC::Error::OK;
  auto func = SWC::Comm::Protocol::Mngr::Params::ColumnMng::Function::CREATE;
  SWC::Env::Config::settings()->get_bool("with-broker")
    ? SWC::Comm::Protocol::Bkr::Req::ColumnMng_Sync::request(
        func, schema, 10000, SWC::Env::Clients::get(), err)
    : SWC::Comm::Protocol::Mngr::Req::ColumnMng_Sync::request(
        func, schema, 10000, SWC::Env::Clients::get(), err);
  SWC_ASSERT(!err);

  schema = SWC::Env::Clients::get()->get_schema(err, schema->col_name);
  SWC_ASSERT(!err);
  SWC_ASSERT(schema);
  SWC_ASSERT(schema->cid);
  return schema;
}


// Producer
void run_queue_producer() {
  auto pid = std::to_string(getpid());
  SWC_PRINT << " Running Queue Producer Example with pid=" << pid << ':' << SWC_PRINT_CLOSE;

  uint32_t cells_a_second =
    SWC::Env::Config::settings()->get_i32("produce-cells");
  uint32_t priorities =
    SWC::Env::Config::settings()->get_i32("produce-priorities");

  auto schema = create_column();

  auto hdlr = SWC::client::Query::Update::Handlers::Common::make(
    SWC::Env::Clients::get(),
    nullptr,
    nullptr,
    SWC::Env::Config::settings()->get_bool("with-broker")
      ? SWC::client::Clients::BROKER
      : SWC::client::Clients::DEFAULT
  );

  auto& col = hdlr->create(schema);


  SWC::DB::Cells::Cell cell;
  cell.flag = SWC::DB::Cells::INSERT;
  cell.set_time_order_desc(true);

  SWC::Core::Vector<std::string> fractions;
  fractions.resize(3);

  size_t total_count = 1;
  SWC::Time::Measure_ns t_progress;

  while(continue_running) {
    t_progress.restart();
    for(uint32_t pr = 1; pr <= priorities; ++pr) {
      for(uint32_t n = cells_a_second; n; --n, ++total_count) {

        cell.key.free();
        // key[priority, id/ts, pid]
        fractions[0] = std::to_string(pr);
        fractions[1] = std::to_string(SWC::Time::now_ns());
        fractions[2] = pid;
        cell.key.add(fractions);

        // value => pid-count
        std::string v(pid + "-" + std::to_string(total_count));
        cell.set_value(v, false);
        col->add(cell);
        hdlr->commit_or_wait(col.get());
      }
    }
    hdlr->commit_if_need();
    hdlr->wait();

    SWC_PRINT
      << "progress cells=" << total_count
      << " avg=" << (t_progress.elapsed() / (priorities * cells_a_second))
      << "ns/cell";
      hdlr->profile.print(SWC_LOG_OSTREAM << ' ');
      SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;

    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  SWC_PRINT
    << "Total cells produced=" << total_count;
    hdlr->profile.print(SWC_LOG_OSTREAM << ' ');
    SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
}


// Consumer
void run_queue_consumer() {
  auto pid = std::to_string(getpid());
  SWC_PRINT << " Running Queue Consumer Example with pid=" << pid << ':' << SWC_PRINT_CLOSE;

  auto schema = create_column();

  auto hdlr = SWC::client::Query::Select::Handlers::Common::make(
    SWC::Env::Clients::get(),
    nullptr, false, nullptr,
    SWC::Env::Config::settings()->get_bool("with-broker")
      ? SWC::client::Clients::BROKER
      : SWC::client::Clients::DEFAULT
  );

  SWC::DB::Specs::Interval intval;
  intval.set_opt__deleting();
  intval.flags.limit = SWC::Env::Config::settings()->get_i32("consume-cells");

  auto col = hdlr->get_columnn(schema->cid);
  SWC::DB::Cells::Result cells;
  size_t total_count = 0;
  while(continue_running) {

    hdlr->scan(schema, intval);
    hdlr->wait();

    if(col->empty()) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      continue;
    }

    col->get_cells(cells);
    for(auto cell : cells) {
      SWC_PRINT
      << "processing ";
      cell->print(SWC_LOG_OSTREAM, schema->col_type);
      SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
    }
    total_count += cells.size();
    cells.free();
  }

  SWC_PRINT
    << "Total cells consumed=" << total_count;
    hdlr->profile.print(SWC_LOG_OSTREAM << ' ');
    SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
}



void init_app_options(SWC::Config::Settings* settings) {
  SWC::Config::init_comm_options(settings);
  SWC::Config::init_client_options(settings);

  settings->cmdline_desc
  .definition(
    settings->usage_str(
      "queue-example Usage: %s [options]\n\nOptions:")
  )
  .add_options()
   ("with-broker",
    SWC::Config::boo(false)->zero_token(),
    "Query with Broker")

   ("producer",
    SWC::Config::boo(false)->zero_token(),
    "run as Producer default Consumer")

   ("produce-cells",
    SWC::Config::i32(100),
    "Number of Cells to produce a second")

   ("produce-priorities",
    SWC::Config::i32(10),
    "Number of Priorities to produce")

   ("consume-cells",
    SWC::Config::i32(10),
    "Number of Cells to consume a request");
}


} // namespace Examples


// Ready for Many on Many of Consumers and Producers
int main(int argc, char** argv) {

  SWC::Env::Config::init(argc, argv, &Examples::init_app_options, nullptr);

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

  SWC::Env::Clients::get()->get_io()->set_signals();
  SWC::Env::Clients::get()->get_io()->signals->async_wait(
    [](const std::error_code& ec, const int &sig) {
      if(!sig || ec == asio::error::operation_aborted)
        return;
      SWC_LOGF(SWC::LOG_INFO,
        "Received signal, sig=%d ec=%s", sig, ec.message().c_str());
      Examples::continue_running.store(false);
    }
  );

  SWC::Env::Config::settings()->get_bool("producer")
    ? Examples::run_queue_producer()
    : Examples::run_queue_consumer();

  SWC::Env::Clients::get()->stop();
  return 0;
}
