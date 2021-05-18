/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_broker_queries_update_MetricsReporting_h
#define swcdb_broker_queries_update_MetricsReporting_h


#include "swcdb/common/sys/MetricsReporting.h"


namespace SWC { namespace Broker {

//! The SWC-DB Broker Metric C++ namespace 'SWC::Broker::Metric'
namespace Metric {


using namespace Common::Query::Update::Metric;


class Reporting final : public Common::Query::Update::Metric::Reporting {
  public:

  typedef std::shared_ptr<Reporting> Ptr;

  Reporting(const Comm::IoContextPtr& io,
            Config::Property::V_GINT32::Ptr cfg_intval);

  void configure_bkr(const char*, const Comm::EndPoints& endpoints);

  virtual ~Reporting() { }

  Item_Net<Comm::Protocol::Bkr::Commands>* net;

};



}}} // namespace SWC::Broker::Metric


#endif // swcdb_broker_queries_update_MetricsReporting_h
