/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/ranger/Settings.h"
#include "swcdb/core/comm/SerializedServer.h"
#include "swcdb/ranger/AppContext.h"


namespace SWC {


/**
 * @brief The SWC-DB Application Ranger C++ namespace 'SWC::Ranger'
 *
 * \ingroup Applications
 */
namespace Ranger {



SWC_SHOULD_NOT_INLINE
Comm::server::SerializedServer::Ptr make_service() {
  auto app_ctx = AppContext::make();
  Comm::server::SerializedServer::Ptr srv(
    new Comm::server::SerializedServer(
      *Env::Config::settings(),
      "RANGER",
      Env::Config::settings()->get_bool("swc.rgr.concurrency.relative"),
      Env::Config::settings()->get_i32("swc.rgr.reactors"),
      Env::Config::settings()->get_i32("swc.rgr.workers"),
      Env::Config::settings()->get_i16("swc.rgr.port"),
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



}} // namespace SWC::Ranger



int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv, &SWC::Config::init_app_options, nullptr);
  SWC::Env::Config::settings()->init_process(true, "swc.rgr.port");
  return SWC::Ranger::run();
}
