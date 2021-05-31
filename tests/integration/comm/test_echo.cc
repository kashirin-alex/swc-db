/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/core/config/Settings.h"
#include "swcdb/core/comm/Settings.h"

#include "swcdb/db/client/Clients.h"
#include "swcdb/db/Protocol/Mngr/req/Echo.h"

#include "swcdb/common/Stats/Stat.h"


namespace SWC{ namespace Config {

void Settings::init_app_options(){
  init_comm_options();
  init_client_options();
  cmdline_desc.add_options()
    ("requests", i32(1), "number of requests")
    ("batch", i32(1), "batch size of each request")
    ("threads", i32(1), "number of threads x (requests x batch)")
    ("threads_conn", i32(1), "threads a connection")
    ("buffer", i64(0), "buffer size to send and receive")
  ;
}
void Settings::init_post_cmd_args() { }
}}

using namespace SWC;

size_t buffer_sz = 0;


class Checker {
  public:
  Core::Atomic<size_t>  failures;
  Core::Atomic<ssize_t> expected;

  Checker(int num_req, int batch_sz, int threads_conn)
          : failures(0), expected(num_req * batch_sz * threads_conn),
            num_req(num_req), batch_sz(batch_sz), threads_conn(threads_conn) {
  }

  void run(Comm::ConnHandlerPtr conn, int req_n = 0){
    if(++req_n > num_req)
      return;

    for(int i=1; i<=batch_sz; ++i) {
      std::make_shared<Comm::Protocol::Mngr::Req::Echo>(
        conn,
        [this, req_n, conn, last=i==batch_sz, start_ts=std::chrono::system_clock::now()]
        (bool state){
          if(!state)
            failures.fetch_add(1);
          ssize_t sz = expected.sub_rslt(1);

          latency.add(
            std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::system_clock::now() - start_ts).count());

          if(!(sz % 100000))
            print_stats();

          if(sz <= 0) {
            std::cout << "notify_all expected=" << sz << "\n";
            cv.notify_all();
            return;
          }

          if(!last)
            return;

          run(conn, req_n);
        },
        buffer_sz
      )->run();
    }
  }

  void get_conn(){
    Env::Clients::get()->managers.queues->service->get_connection(
      Env::Clients::get()->managers.groups->get_endpoints(
        DB::Types::MngrRole::SCHEMAS, 0, 0),
      [this](Comm::ConnHandlerPtr conn){
        if(!conn || !conn->is_open()){
          get_conn();
          return;
        }

        std::vector<std::thread*> threads;
        for(int t=1; t<=threads_conn; ++t)
          std::thread([this, conn](){run(conn);}).detach();
      },
      std::chrono::milliseconds(1000),
      1
    );
  }

  void print_stats(){
    std::cout << " avg=" << latency.avg()
              << " min=" << latency.min()
              << " max=" << latency.max()
              << " count=" << latency.count()
              << " failures=" << failures
              << "\n";
  }

  void run(int num_threads){
    expected.store(expected * num_threads);

    auto start = std::chrono::system_clock::now();

    for(int t=1;t<=num_threads;++t)
      std::thread([this](){get_conn();}).detach();

    Core::UniqueLock lock_wait(lock);
    cv.wait(lock_wait, [this]{return !expected.load();});


    int64_t took = std::chrono::duration_cast<std::chrono::nanoseconds>(
                      std::chrono::system_clock::now() - start).count();

    std::cout << "# FINISHED,"
              << " threads=" << num_threads
              << " threads_conn=" << threads_conn
              << " requests=" << num_req
              << " batch=" << batch_sz
              << " took=" << took
              << " median=" << took/latency.count()
              << "\n";
    print_stats();
    SWC_ASSERT(!failures);
  }

  int num_req;
  int batch_sz;
  int threads_conn;
  std::mutex lock;
  std::condition_variable cv;
  Common::Stats::Stat latency;

};


int main(int argc, char** argv) {
  Env::Config::init(argc, argv);

  Env::Clients::init(
    client::Clients::make(
      *Env::Config::settings(),
      Comm::IoContext::make("Clients", 8),
      nullptr, // std::make_shared<client::ManagerContext>()
      nullptr  // std::make_shared<client::RangerContext>()
    )
  );

  buffer_sz = Env::Config::settings()->get_i64("buffer");

  Checker(
    Env::Config::settings()->get_i32("requests"),
    Env::Config::settings()->get_i32("batch"),
    Env::Config::settings()->get_i32("threads_conn")
  ).run(
    Env::Config::settings()->get_i32("threads")
  );

  Env::Clients::get()->stop();

  return 0;
}
