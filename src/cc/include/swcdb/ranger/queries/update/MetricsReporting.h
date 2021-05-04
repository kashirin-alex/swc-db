/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_queries_update_MetricsReporting_h
#define swcdb_ranger_queries_update_MetricsReporting_h


#include "swcdb/common/sys/MetricsReporting.h"


namespace SWC { namespace Ranger { namespace Metric {


using namespace Common::Query::Update::Metric;


class Reporting final : public Common::Query::Update::Metric::Reporting {
  public:

  typedef std::shared_ptr<Reporting> Ptr;

  Reporting(const Comm::IoContextPtr& io,
            Config::Property::V_GINT32::Ptr cfg_intval_ms)
            : Common::Query::Update::Metric::Reporting(io, cfg_intval_ms) {
  }

  void configure_rgr(const char* host, const Comm::EndPoints& endpoints) {
    Common::Query::Update::Metric::Reporting::configure(
      "swcdb", "rgr", host, endpoints,
      Comm::Protocol::Rgr::Command::MAX_CMD
    );

    // ++ Ranger Metrics
  }

  virtual ~Reporting() { }

};



}}} // namespace SWC::Ranger::Metric


#endif // swcdb_ranger_queries_update_MetricsReporting_h
