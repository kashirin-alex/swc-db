/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_fsbroker_queries_update_MetricsReporting_h
#define swcdb_fsbroker_queries_update_MetricsReporting_h


#include "swcdb/common/sys/MetricsReporting.h"


namespace SWC { namespace FsBroker { namespace Metric {


using namespace Common::Query::Update::Metric;


class Reporting final : public Common::Query::Update::Metric::Reporting {
  public:

  typedef std::shared_ptr<Reporting> Ptr;

  Reporting(const Comm::IoContextPtr& io,
            Config::Property::V_GINT32::Ptr cfg_intval_ms)
            : Common::Query::Update::Metric::Reporting(io, cfg_intval_ms) {
  }

  void configure_fsbroker(const char* host, const Comm::EndPoints& endpoints) {
    Common::Query::Update::Metric::Reporting::configure(
      "swcdb", "fsbroker", host, endpoints,
      Comm::Protocol::FsBroker::Command::MAX_CMD
    );

    // ++ FsBroker Metrics
  }

  virtual ~Reporting() { }

};



}}} // namespace SWC::FsBroker::Metric


#endif // swcdb_fsbroker_queries_update_MetricsReporting_h
