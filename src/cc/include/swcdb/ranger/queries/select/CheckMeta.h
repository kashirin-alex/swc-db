/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_queries_select_CheckMeta_h
#define swcdb_ranger_queries_select_CheckMeta_h


#include "swcdb/db/client/Query/SelectHandlerBaseSingleColumn.h"


namespace SWC { namespace Ranger { namespace Query { namespace Select {


class CheckMeta final :
          public client::Query::Select::Handlers::BaseSingleColumn {
  public:

  static void run(const RangePtr& range, const Callback::RangeLoad::Ptr& req);

  typedef std::shared_ptr<CheckMeta>   Ptr;

  RangePtr                  range;
  Callback::RangeLoad::Ptr  req;
  DB::Specs::Interval       spec;

  CheckMeta(const RangePtr& range, const Callback::RangeLoad::Ptr& req);

  virtual ~CheckMeta() { }

  bool valid(const client::Query::ReqBase::Ptr& _req) noexcept override;

  void response(int err) override;

};


}}}}


#endif // swcdb_ranger_queries_select_CheckMeta_h
