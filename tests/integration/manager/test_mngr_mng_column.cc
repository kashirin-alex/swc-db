/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/Semaphore.h"
#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"

#include "swcdb/db/client/Clients.h"

#include "swcdb/db/Cells/SpecsScan.h"

#if !defined(CLIENT_EXECUTOR)
#define CLIENT_EXECUTOR 1
#include "swcdb/db/Protocol/Mngr/req/ColumnMng.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnGet.h"
namespace ProtocolExecutor = SWC::Comm::Protocol::Mngr;
#endif

#include "swcdb/common/Stats/Stat.h"


namespace SWC{ namespace Config {

void init_app_options(Settings* settings) {
  init_comm_options(settings);
  init_client_options(settings);
  // file_desc.add_options();
}

}}


using namespace SWC;

struct ExpctedRsp final {
  public:
  ExpctedRsp(std::string a_name, DB::Types::Encoder a_blk_encoding,
             bool a_exists)
            : name(a_name), blk_encoding(a_blk_encoding),
              exists(a_exists), chks(0) { }

  std::string name;
  DB::Types::Encoder  blk_encoding;
  bool                exists;
  Core::Atomic<int>   chks;
};

struct ReqDataBase {
  SWC_CAN_INLINE
  client::Clients::Ptr& get_clients() noexcept {
    return Env::Clients::get();
  }
  SWC_CAN_INLINE
  bool valid() {
    return true;
  }
};

std::string get_name(int n, bool modified=false){
  std::string name("column-");
  name.append(std::to_string(n));
  if(modified)
    name.append("-modified");
  return name;
}


void check_delete(int num_of_cols, bool modified) {
  Core::Semaphore sem(num_of_cols, num_of_cols);

  struct ReqData : ReqDataBase {
    Core::Semaphore* sem;
    SWC_CAN_INLINE
    ReqData(Core::Semaphore* a_sem) noexcept : sem(a_sem) { }
    SWC_CAN_INLINE
    void callback(const Comm::client::ConnQueue::ReqBase::Ptr& req_ptr,
                  int err,
                  const Comm::Protocol::Mngr::Params::ColumnGetRsp& rsp) {
      if(err)
        SWC_PRINT << "ColumnGet err=" << err
                  << "(" << Error::get_text(err) << ")"
                  << SWC_PRINT_CLOSE;
      if(err == Error::REQUEST_TIMEOUT)
        return req_ptr->request_again();
      if(err) {
        sem->release();
        return;
      }
      ProtocolExecutor::Req::ColumnMng
        <ProtocolExecutor::Req::Functional_ColumnMng>
          ::request(
            Comm::Protocol::Mngr::Params::ColumnMng::Function::DELETE,
            rsp.schema,
            300000,
            Env::Clients::get(),
            [sem=sem]
            (void*, Comm::client::ConnQueue::ReqBase::Ptr _req_ptr, int _err) {
              if(_err)
              SWC_PRINT << "ColumnMng DELETE err=" << _err
                        << "(" << Error::get_text(_err) << ")"
                        << SWC_PRINT_CLOSE;
              if(_err == Error::REQUEST_TIMEOUT)
                return _req_ptr->request_again();
              sem->release();
            }
          );
    }
  };

  for(int n=1; n<=num_of_cols; ++n) {
    ProtocolExecutor::Req::ColumnGet<ReqData>::schema(
      get_name(n, modified), 300000, &sem);
  }
  sem.wait_all();
}

void check_get(size_t num_of_cols, bool modified,
               DB::Types::Encoder blk_encoding,
               bool exist = true, bool verbose=false){
  std::cout << "########### get_schema_by_name ###########\n";
  auto latency = std::make_shared<Common::Stats::Stat>();

  std::vector<std::shared_ptr<ExpctedRsp>> expected;

  for(size_t n=1; n<=num_of_cols;++n){
    expected.push_back(std::make_shared<ExpctedRsp>(
      get_name(n, modified),
      blk_encoding,
      exist
    ));
  }

  struct ReqDataByName : ReqDataBase {
    std::shared_ptr<ExpctedRsp>           req;
    std::shared_ptr<Common::Stats::Stat>  latency;
    bool                                  verbose;
    int64_t                               start_ts;
    SWC_CAN_INLINE
    ReqDataByName(const std::shared_ptr<ExpctedRsp>& a_req,
                  const std::shared_ptr<Common::Stats::Stat>& a_latency,
                  bool a_verbose) noexcept
            : req(a_req), latency(a_latency),
              verbose(a_verbose), start_ts(Time::now_ns()) {
    }
    SWC_CAN_INLINE
    void callback(const Comm::client::ConnQueue::ReqBase::Ptr& req_ptr,
                  int err,
                  const Comm::Protocol::Mngr::Params::ColumnGetRsp& rsp) {
      if(err == Error::REQUEST_TIMEOUT) {
        std::cout << " err=" << err << "(" << Error::get_text(err) << ") \n";
        req_ptr->request_again();
        return;
      }

      uint64_t took  = Time::now_ns() - start_ts;
      latency->add(took);
      if(verbose)
        std::cout << "ColumnGetRsp: exists="<< req->exists << " took=" << took
                  << " count=" << latency->count()
                  << " err=" << err << "(" << Error::get_text(err) << ") "
                  << " " << (err==Error::OK?rsp.schema->to_string().c_str():"NULL")
                   << "\n";

      if(err==Error::OK){
        if(!req->exists) {
          std::cerr << " SHOULDN'T exist name=" << req->name << "\n";
          exit(1);
        }
        if(req->blk_encoding != rsp.schema->blk_encoding) {
          std::cerr << " blk_encoding don't match \n";
          exit(1);
        }
        if(!Condition::str_eq(req->name, rsp.schema->col_name)) {
          std::cerr << " name don't match \n";
          exit(1);
        }
      } else if(req->exists){
        std::cerr << " SHOULD exist name=" << req->name << "\n";
        exit(1);
      }
      req->chks.fetch_add(1);
    }
  };

  for(auto& req : expected) {
    ProtocolExecutor::Req::ColumnGet<ReqDataByName>::schema(
      req->name,
      300000,
      req, latency, verbose
    );
  }

  while(latency->count() < num_of_cols) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << "get_schema_by_name"
            << " avg=" << latency->avg()
            << " min=" << latency->min()
            << " max=" << latency->max()
            << " count=" << latency->count()
            << "\n";


  std::cout << "########### get_id_by_name ###########\n";
  latency = std::make_shared<Common::Stats::Stat>();

  struct ReqDataByCID : ReqDataBase {
    std::shared_ptr<ExpctedRsp>           req;
    std::shared_ptr<Common::Stats::Stat>  latency;
    bool                                  verbose;
    int64_t                               start_ts;
    SWC_CAN_INLINE
    ReqDataByCID(const std::shared_ptr<ExpctedRsp>& a_req,
                 const std::shared_ptr<Common::Stats::Stat>& a_latency,
                 bool a_verbose) noexcept
            : req(a_req), latency(a_latency),
              verbose(a_verbose), start_ts(Time::now_ns()) {
    }
    SWC_CAN_INLINE
    void callback(const Comm::client::ConnQueue::ReqBase::Ptr& req_ptr,
                  int err,
                  const Comm::Protocol::Mngr::Params::ColumnGetRsp& rsp) {
      if(err == Error::REQUEST_TIMEOUT) {
        std::cout << " err=" << err << "(" << Error::get_text(err) << ") \n";
        req_ptr->request_again();
        return;
      }

      uint64_t took = Time::now_ns() - start_ts;
      latency->add(took);

      if(verbose)
        std::cout << "ColumnGetRsp: exists="<< req->exists
                  << " took=" << took
                  << " count=" << latency->count()
                  << " err=" << err << "(" << Error::get_text(err) << ") "
                  << " " << (err==Error::OK?rsp.cid:-1) << "\n";

      if(err==Error::OK){
        if(!req->exists) {
          std::cerr << " SHOULDN'T exist name=" << req->name << "\n";
          exit(1);
        }
      } else if(req->exists){
        std::cerr << " SHOULD exist name=" << req->name << "\n";
        exit(1);
      }
      req->chks.fetch_add(1);
    }
  };

  for(auto& req : expected) {
    ProtocolExecutor::Req::ColumnGet<ReqDataByCID>::cid(
      req->name,
      300000,
      req, latency, verbose
    );
  }

  while(latency->count() < num_of_cols) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << "get get_id_by_name"
            << " avg=" << latency->avg()
            << " min=" << latency->min()
            << " max=" << latency->max()
            << " count=" << latency->count()
            << "\n";

  for(auto& req : expected) {
    if(req->chks != 2){

      std::cerr << " chks="<< req->chks.load()
                << " name=" << req->name
                << " exists=" << req->exists
                << " \n";
      exit(1);
    }
  }
}


void chk(Comm::Protocol::Mngr::Params::ColumnMng::Function func,
         size_t num_of_cols,
         DB::Types::Encoder blk_encoding,
         bool modified, bool verbose=false) {

  std::cout << "########### chk func=" << func << " ###########\n";
  auto latency = std::make_shared<Common::Stats::Stat>();

  auto clients = Env::Clients::get();
  for(size_t n=1;n<=num_of_cols;++n) {

    auto schema = DB::Schema::make();
    schema->col_name = get_name(n, modified);
    schema->col_type = DB::Types::Column::COUNTER_I64;
    schema->cell_versions = 10;
    schema->cell_ttl = 1234;
    schema->blk_encoding = blk_encoding;
    schema->blk_size = 3;
    schema->blk_cells = 9876543;

    ProtocolExecutor::Req::ColumnMng
      <ProtocolExecutor::Req::Functional_ColumnMng>
    ::request(
      func, schema, 300000,
      clients,
      [func, latency, verbose, start_ts=Time::now_ns()]
      (void*, Comm::client::ConnQueue::ReqBase::Ptr req_ptr, int err) {

        uint64_t took = Time::now_ns() - start_ts;

        if(err != Error::OK && (
            (func == Comm::Protocol::Mngr::Params::ColumnMng::Function::CREATE
              && err != Error::COLUMN_SCHEMA_NAME_EXISTS)
            ||
            (func == Comm::Protocol::Mngr::Params::ColumnMng::Function::DELETE
              && err != Error::COLUMN_SCHEMA_NAME_NOT_EXISTS
              && err != Error::COLUMN_SCHEMA_NAME_NOT_CORRES)
            ||
            (func == Comm::Protocol::Mngr::Params::ColumnMng::Function::MODIFY
              && err != Error::COLUMN_SCHEMA_NAME_NOT_EXISTS
              && err != Error::COLUMN_SCHEMA_NOT_DIFFERENT )
          )) {
          SWC_PRINT << " func = " << func
                    << " err="<<err<< "(" << Error::get_text(err) << ")"
                    << SWC_PRINT_CLOSE;
          req_ptr->request_again();
        } else {
          latency->add(took);
        }

        if(verbose)
          SWC_PRINT << " func = " << func
            << " err="<<err<< "(" << Error::get_text(err) << ")"
            << " took=" << took
            << " avg=" << latency->avg()
            << " min=" << latency->min()
            << " max=" << latency->max()
            << " count=" << latency->count()
            << SWC_PRINT_CLOSE;
      }
    );

  }

  while(latency->count() < num_of_cols) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  std::cout << " func="<< func
            << " num_of_cols="<< num_of_cols
            << " avg=" << latency->avg()
            << " min=" << latency->min()
            << " max=" << latency->max()
            << " count=" << latency->count()
            << "\n";
}

void chk_rename(size_t num_of_cols, bool verbose=false){
  std::cout << "########### chk_rename ###########\n";
  auto latency = std::make_shared<Common::Stats::Stat>();

  std::vector<std::shared_ptr<ExpctedRsp>> expected;

  struct ReqData : ReqDataBase {
    size_t                                n;
    std::shared_ptr<Common::Stats::Stat>  latency;
    bool                                  verbose;
    int64_t                               start_ts;
    SWC_CAN_INLINE
    ReqData(size_t a_n,
            const std::shared_ptr<Common::Stats::Stat>& a_latency,
            bool a_verbose) noexcept
            : n(a_n), latency(a_latency),
              verbose(a_verbose), start_ts(Time::now_ns()) {
    }
    SWC_CAN_INLINE
    void callback(const Comm::client::ConnQueue::ReqBase::Ptr& req_ptr,
                  int err,
                  const Comm::Protocol::Mngr::Params::ColumnGetRsp& rsp) {
      if(err == Error::REQUEST_TIMEOUT) {
        SWC_PRINT << " err=" << err << "(" << Error::get_text(err) << ")"
                  << SWC_PRINT_CLOSE;
        req_ptr->request_again();
        return;
      }

      auto new_schema = DB::Schema::make(rsp.schema);
      new_schema->col_name = get_name(n, true);
      new_schema->revision = 2;

      if(verbose)
        SWC_PRINT << "modify name: \n"
          << "from " << rsp.schema->to_string() << "\n"
          << "to   " << new_schema->to_string() << SWC_PRINT_CLOSE;

      ProtocolExecutor::Req::ColumnMng
        <ProtocolExecutor::Req::Functional_ColumnMng>
      ::request(
        Comm::Protocol::Mngr::Params::ColumnMng::Function::MODIFY,
        new_schema,
        300000,
        Env::Clients::get(),
        [latency=latency, start_ts=start_ts]
        (void*, Comm::client::ConnQueue::ReqBase::Ptr _req_ptr, int _err) {
          if(_err != Error::OK
            && _err != Error::COLUMN_SCHEMA_NAME_NOT_EXISTS
            && _err != Error::COLUMN_SCHEMA_NOT_DIFFERENT){

            _req_ptr->request_again();
            return;
          }
          latency->add(Time::now_ns() - start_ts);
        }
      );
    }
  };

  for(size_t n=1; n<=num_of_cols;++n){
    ProtocolExecutor::Req::ColumnGet<ReqData>::schema(
      get_name(n, false), 300000,
      n, latency, verbose
    );
  }

  while(latency->count() < num_of_cols) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  SWC_PRINT << " modify name "
            << " num_of_cols="<< num_of_cols
            << " avg=" << latency->avg()
            << " min=" << latency->min()
            << " max=" << latency->max()
            << " count=" << latency->count()
            << SWC_PRINT_CLOSE;
}


int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv, &SWC::Config::init_app_options, nullptr);

  Env::Clients::init(
    client::Clients::make(
      *Env::Config::settings(),
      Comm::IoContext::make("Clients", 8),
      #if CLIENT_EXECUTOR == 2
        nullptr  // std::make_shared<client::BrokerContext>()
      #else
        nullptr, // std::make_shared<client::ManagerContext>()
        nullptr  // std::make_shared<client::RangerContext>()
      #endif
    )->init()
  );


  int num_of_cols = 1000;

  check_delete(num_of_cols, false);
  check_delete(num_of_cols, true);

  chk(Comm::Protocol::Mngr::Params::ColumnMng::Function::CREATE,
      num_of_cols, DB::Types::Encoder::PLAIN, false);
  check_get(num_of_cols, false, DB::Types::Encoder::PLAIN);
  std::cout << "\n";
  std::this_thread::sleep_for(std::chrono::milliseconds(5000));


  chk_rename(num_of_cols, true);

  check_get(num_of_cols, true, DB::Types::Encoder::PLAIN, true);
  std::cout << "\n";

  chk(Comm::Protocol::Mngr::Params::ColumnMng::Function::MODIFY,
      num_of_cols, DB::Types::Encoder::SNAPPY, true);
  check_get(num_of_cols, true, DB::Types::Encoder::SNAPPY);
  std::cout << "\n";



  check_delete(num_of_cols, true);
  check_get(num_of_cols, true, DB::Types::Encoder::SNAPPY, false);
  std::cout << "\n";

  std::cout << " OK! \n \n";


  exit(0);


  Env::Clients::get()->stop();
  std::cout << " ### EXIT ###\n";
  return 0;
}
