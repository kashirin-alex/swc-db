/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_queries_update_MetricsReporting_h
#define swcdb_manager_queries_update_MetricsReporting_h


#include "swcdb/common/sys/MetricsReporting.h"


namespace SWC { namespace Manager {

//! The SWC-DB Manager Metric C++ namespace 'SWC::Manager::Metric'
namespace Metric {


using namespace Common::Query::Update::Metric;


class Reporting final : public Common::Query::Update::Metric::Reporting {
  public:

  typedef std::shared_ptr<Reporting> Ptr;

  Reporting(const Comm::IoContextPtr& io);

  void configure_mngr(const char*, const Comm::EndPoints& endpoints);

  virtual ~Reporting() noexcept { }

  Item_Net<Comm::Protocol::Mngr::Commands>* net;

};



}}} // namespace SWC::Manager::Metric


#endif // swcdb_manager_queries_update_MetricsReporting_h
