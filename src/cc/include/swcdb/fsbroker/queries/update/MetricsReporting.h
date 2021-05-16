/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_fsbroker_queries_update_MetricsReporting_h
#define swcdb_fsbroker_queries_update_MetricsReporting_h


#include "swcdb/common/sys/MetricsReporting.h"


namespace SWC { namespace FsBroker {

//! The SWC-DB FsBroker Metric C++ namespace 'SWC::FsBroker::Metric'
namespace Metric {


using namespace Common::Query::Update::Metric;


class Reporting final : public Common::Query::Update::Metric::Reporting {
  public:

  typedef std::shared_ptr<Reporting> Ptr;

  Reporting(const Comm::IoContextPtr& io,
            Config::Property::V_GINT32::Ptr cfg_intval);

  void configure_fsbroker(const char*, const Comm::EndPoints& endpoints);

  virtual ~Reporting() { }

  Item_Net<Comm::Protocol::FsBroker::Commands>* net;
  Item_CountVolume*                             fds;

};



}}} // namespace SWC::FsBroker::Metric


#endif // swcdb_fsbroker_queries_update_MetricsReporting_h
