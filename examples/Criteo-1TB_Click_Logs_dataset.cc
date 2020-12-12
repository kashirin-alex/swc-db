/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


/* 
THE DATA INPUT IS WITH DATA-SAMPLES AVAILABLE AT:
  https://ailab.criteo.com/download-criteo-1tb-click-logs-dataset/
  steps:
    1) download the day_[1-28] files
    2) extract to {criteo-samples-path}
    *  After SWC-DB is running
    3) run the example,
        * set cmd-arg --criteo-samples-path="samples-path"
          if {criteo-samples-path} is other than ./criteo_samples
        * optional, set cmd-arg --criteo-separated-days=BOOL 
          add Day# fraction at end of a Key
*/


#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/Query/Update.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/common/Stats/FlowRate.h"


namespace SWC { namespace Config {

void Settings::init_app_options() {
  init_comm_options();
  init_client_options();

  cmdline_desc.add_options()
   ("criteo-samples-path",  str("./criteo_samples"), 
    "Path to Criteo data-samples 'day_[1-28]` "
    "files from https://ailab.criteo.com/download-criteo-1tb-click-logs-dataset/")
   ("criteo-separated-days",  boo(false), 
    "add Day# fraction at end of a Key")
  ;
}

void Settings::init_post_cmd_args() { }

}} // namespace SWC::Config



namespace Examples {

SWC::DB::Schema::Ptr create_column();


void generate_criteo_logs() {
  bool separate_days = SWC::Env::Config::settings()->get_bool("criteo-separated-days");

  uint64_t ts = SWC::Time::now_ns();
  uint64_t ts_progress = ts;

  size_t added_cells = 0;
  size_t added_bytes = 0;
  size_t resend_cells = 0;

  SWC::DB::Cells::Cell cell;
  cell.flag = SWC::DB::Cells::INSERT;
  cell.set_time_order_desc(true);

  std::vector<std::string> fractions;
  // [13-features + 26-categories]
  fractions.resize(39 + separate_days);
  // value (clicks)

  
  auto req = std::make_shared<SWC::client::Query::Update>();
  auto schema = create_column();
  req->columns->create(schema);
  auto col = req->columns->get_col(schema->cid);


  // day_1 == cells(199,563,535) unique(199,555,996)
  // data-samples from https://ailab.criteo.com/download-criteo-1tb-click-logs-dataset/
  std::string data_path(SWC::Env::Config::settings()->get_str("criteo-samples-path"));
  
  for(int day=1;day<29;++day) {

  std::ifstream buffer(data_path + "/day_" + std::to_string(day));
  if(!buffer.is_open())
    break;

  std::string line;
  const char* pline;
  const char* pline_s;
  int n_frac ;
  while (std::getline(buffer, line)) {
    SWC_ASSERT(!line.empty());
    n_frac = -1;
    for(pline = pline_s = line.data(); ; ++pline) {
      if(*pline == '\t' || !*pline) {
        std::string v(pline_s, pline - pline_s);
        if(n_frac == -1) {
          cell.set_counter(0, std::strtoull(v.data(), 0, 10));
        } else {
          fractions[n_frac] = v;
        }
        if(!*pline)
          break;
        pline_s = pline  + 1;
        SWC_ASSERT(++n_frac < 40);
      }
    }
    if(separate_days)
      fractions[39] = std::to_string(day);

    cell.key.free();
    cell.key.add(fractions);
    cell.set_revision(SWC::Time::now_ns()); // accumulate duplicates & days
    col->add(cell);

    req->commit_or_wait(col);

    /*
      on classifications groupings count 
        - more combinations of sequential permutations
          and/or with std::next_permutation
    for(int n1=0; n1<40; ++n1) {
      auto it = fractions.cbegin() + n1;
      for(int n2=n1; n2<40; ) {
        cell.key.free();
        cell.key.add(it, fractions.begin() + (++n2));
        col->add(cell); 
        req->commit_or_wait(col);
      }
    }
    */

    resend_cells += req->result->get_resend_count();

    added_bytes += cell.encoded_length();
    if(!(++added_cells % 100000)) {
      SWC_PRINT 
        << "progress day="<< day
        << " cells=" << added_cells
        << " avg=" << ((SWC::Time::now_ns() - ts_progress) / 100000)
        << "ns/cell) ";
      req->result->profile.print(SWC_LOG_OSTREAM << ' ');
      // SWC_LOG_OSTREAM << cell.to_string()
      SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
      ts_progress = SWC::Time::now_ns();
    }
  }
  buffer.close();

  } // for days

  req->commit_if_need();
  req->wait();

  resend_cells += req->result->get_resend_count();
  SWC_ASSERT(added_cells && added_bytes);
  
  SWC::Common::Stats::FlowRate::Data rate(
    added_bytes, SWC::Time::now_ns() - ts);
  SWC_PRINT << std::endl << std::endl;
  rate.print_cells_statistics(SWC_LOG_OSTREAM, added_cells, resend_cells);
  req->result->profile.display(SWC_LOG_OSTREAM);
  SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
}


SWC::DB::Schema::Ptr create_column() {
  auto schema = SWC::DB::Schema::make();

  schema->col_name = "Criteo-1TB-Click-Logs-Samples";
  schema->col_seq = SWC::DB::Types::KeySeq::VOLUME;
  schema->col_type = SWC::DB::Types::Column::COUNTER_I64;
  schema->cell_versions = 1;
  schema->cell_ttl = 0;
  schema->cs_size = 209715200;
  /* by default ranger-cfg
  schema->blk_encoding = SWC::DB::Types::Encoder::ZSTD;
  schema->blk_size = 33554432;
  schema->blk_cells = 100000;
  schema->cs_replication = 1;
  schema->cs_max = 2;
  schema->log_rollout_ratio = 10;
  schema->compact_percent = 33;
  */

  // CREATE COLUMN
  std::promise<int>  res;
  SWC::Comm::Protocol::Mngr::Req::ColumnMng::request(
    SWC::Comm::Protocol::Mngr::Req::ColumnMng::Func::CREATE,
    schema,
    [await=&res] (const SWC::Comm::client::ConnQueue::ReqBase::Ptr&, int err) {
      if(err == SWC::Error::COLUMN_SCHEMA_NAME_EXISTS)
        err = SWC::Error::OK; 
      await->set_value(err);
    },
    10000
  );
  SWC_ASSERT(!res.get_future().get());

  int err = SWC::Error::OK;
  schema = SWC::Env::Clients::get()->schemas->get(err, schema->col_name);
  SWC_ASSERT(!err);
  SWC_ASSERT(schema);
  SWC_ASSERT(schema->cid);
  return schema;
}





}


int main(int argc, char** argv) {
    
  SWC::Env::Config::init(argc, argv);

  SWC::Env::Clients::init(
    std::make_shared<SWC::client::Clients>(
      nullptr, // Env::IoCtx::io(),
      nullptr, // std::make_shared<client::ManagerContext>()
      nullptr  // std::make_shared<client::RangerContext>()
    )
  );

  Examples::generate_criteo_logs();

  SWC::Env::IoCtx::io()->stop();

  return 0;
}