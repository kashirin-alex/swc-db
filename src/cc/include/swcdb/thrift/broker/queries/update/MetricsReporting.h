/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_thrift_broker_queries_update_MetricsReporting_h
#define swcdb_thrift_broker_queries_update_MetricsReporting_h


#include "swcdb/common/sys/MetricsReporting.h"


namespace SWC { namespace ThriftBroker {

//! The SWC-DB ThriftBroker Metric C++ namespace 'SWC::ThriftBroker::Metric'
namespace Metric {


using namespace Common::Query::Update::Metric;



struct Commands {
  static constexpr const uint8_t MAX = 1;

  static const char* to_string(uint8_t) noexcept {
    return "NOIMPL";
  }
};



class Reporting final : public Common::Query::Update::Metric::Reporting {
  public:

  typedef std::shared_ptr<Reporting> Ptr;

  Reporting(const Comm::IoContextPtr& a_io,
            Config::Property::Value_int32_g::Ptr a_cfg_intval);

  void configure_thriftbroker(const char*, const Comm::EndPoints& endpoints);

  virtual ~Reporting() noexcept { }

  Item_Net<Commands>* net;

};



}}} // namespace SWC::ThriftBroker::Metric


#endif // swcdb_thrift_broker_queries_update_MetricsReporting_h
