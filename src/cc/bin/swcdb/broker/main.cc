/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/broker/Settings.h"
#include "swcdb/core/comm/SerializedServer.h"
#include "swcdb/broker/AppContext.h"


namespace SWC {


/**
 * @brief The SWC-DB Application Broker C++ namespace 'SWC::Broker'
 *
 * \ingroup Applications
 */
namespace Broker {



SWC_SHOULD_NOT_INLINE
Comm::server::SerializedServer::Ptr make_service() {
  auto app_ctx = AppContext::make();
  Comm::server::SerializedServer::Ptr srv(
    new Comm::server::SerializedServer(
      *Env::Config::settings(),
      "BROKER",
      Env::Config::settings()->get_bool("swc.bkr.concurrency.relative"),
      Env::Config::settings()->get_i32("swc.bkr.reactors"),
      Env::Config::settings()->get_i32("swc.bkr.workers"),
      Env::Config::settings()->get_i16("swc.bkr.port"),
      app_ctx
    )
  );
  app_ctx->set_srv(srv);
  return srv;
}

SWC_SHOULD_NOT_INLINE
int exiting() {
  SWC_LOG(LOG_INFO, "Exit");
  SWC_CAN_QUICK_EXIT(EXIT_SUCCESS);

  Env::Config::reset();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  return 0;
}

SWC_SHOULD_NOT_INLINE
int run() {
  SWC_TRY_OR_LOG("",
    make_service()->run();
    return exiting();
  );
  return 1;
}



}} // namespace SWC::Broker



int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv, &SWC::Config::init_app_options, nullptr);
  SWC::Env::Config::settings()->init_process(true, "swc.bkr.port");
  return SWC::Broker::run();
}
