/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_queries_update_MetricsReporting_h
#define swcdb_ranger_queries_update_MetricsReporting_h


#include "swcdb/common/sys/MetricsReporting.h"


namespace SWC { namespace Ranger {

//! The SWC-DB Ranger Metric C++ namespace 'SWC::Ranger::Metric'
namespace Metric {


using namespace Common::Query::Update::Metric;


class Reporting final : public Common::Query::Update::Metric::Reporting {
  public:

  typedef std::shared_ptr<Reporting> Ptr;

  Reporting(const Comm::IoContextPtr& a_io);

  Reporting(const Reporting&) = delete;
  Reporting(Reporting&&) = delete;
  Reporting& operator=(const Reporting&) = delete;
  Reporting& operator=(Reporting&&) = delete;

  void configure_rgr(const char*, const Comm::EndPoints& endpoints);

  virtual ~Reporting() noexcept { }

  Item_Net<Comm::Protocol::Rgr::Commands>* net;

};



}}} // namespace SWC::Ranger::Metric


#endif // swcdb_ranger_queries_update_MetricsReporting_h
