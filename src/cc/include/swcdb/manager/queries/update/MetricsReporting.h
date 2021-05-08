/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_queries_update_MetricsReporting_h
#define swcdb_manager_queries_update_MetricsReporting_h


#include "swcdb/common/sys/MetricsReporting.h"


namespace SWC { namespace Manager { namespace Metric {


using namespace Common::Query::Update::Metric;


class Reporting final : public Common::Query::Update::Metric::Reporting {
  public:

  typedef std::shared_ptr<Reporting> Ptr;

  Reporting(const Comm::IoContextPtr& io,
            Config::Property::V_GINT32::Ptr cfg_intval_ms)
            : Common::Query::Update::Metric::Reporting(io, cfg_intval_ms) {
  }

  void configure_mngr(const char*, const Comm::EndPoints& endpoints) {
    char hostname[256];
    if(gethostname(hostname, sizeof(hostname)) == -1)
      SWC_THROW(errno, "gethostname");

    auto level = Common::Query::Update::Metric::Reporting::configure(
      "swcdb", "mngr", hostname, endpoints,
      Comm::Protocol::Mngr::Command::MAX_CMD
    );

    auto fs = Env::FsInterface::fs();
    if(fs->statistics.enabled)
      level->metrics.emplace_back(new Item_FS(fs));
    // ++ Manager Metrics
  }

  virtual ~Reporting() { }

};



}}} // namespace SWC::Manager::Metric


#endif // swcdb_manager_queries_update_MetricsReporting_h
