/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fsbroker/Settings.h"
#include "swcdb/core/comm/SerializedServer.h"
#include "swcdb/fsbroker/AppContext.h"


namespace SWC {


/**
 * @brief The SWC-DB Application FsBroker C++ namespace 'SWC::FsBroker'
 *
 * \ingroup Applications
 */
namespace FsBroker {


int run() {
  SWC_TRY_OR_LOG("",

  {
  AppContext::Ptr app_ctx(new AppContext());

  Comm::server::SerializedServer::Ptr srv(
    new Comm::server::SerializedServer(
      *Env::Config::settings(),
      "FS-BROKER",
      Env::Config::settings()->get_i32("swc.FsBroker.reactors"),
      Env::Config::settings()->get_i32("swc.FsBroker.workers"),
      Env::Config::settings()->get_i16("swc.fs.broker.port"),
      app_ctx
    )
  );
  app_ctx->set_srv(srv);
  srv->run();

  SWC_LOG(LOG_INFO, "Exit");
  SWC_CAN_QUICK_EXIT(EXIT_SUCCESS);

  Env::Config::reset();
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));
  return 0;
  );

  return 1;
}


}} // namespace SWC::FsBroker



int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);
  SWC::Env::Config::settings()->init_process(true, "swc.fs.broker.port");
  return SWC::FsBroker::run();
}
