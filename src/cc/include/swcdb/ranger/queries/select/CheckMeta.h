/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_queries_select_CheckMeta_h
#define swcdb_ranger_queries_select_CheckMeta_h


#include "swcdb/db/client/Query/Select/Handlers/BaseSingleColumn.h"


namespace SWC { namespace Ranger {



//! The SWC-DB Ranger Query C++ namespace 'SWC::Ranger::Query'
namespace Query {



//! The SWC-DB Ranger Select Query C++ namespace 'SWC::Ranger::Query::Select'
namespace Select {



class CheckMeta final :
          public client::Query::Select::Handlers::BaseSingleColumn {
  public:

  static void run(const RangePtr& range, const Callback::RangeLoad::Ptr& req);

  typedef std::shared_ptr<CheckMeta>   Ptr;

  RangePtr                  range;
  Callback::RangeLoad::Ptr  req;
  DB::Specs::Interval       spec;

  CheckMeta(const RangePtr& range, const Callback::RangeLoad::Ptr& req);

  virtual ~CheckMeta() noexcept { }

  bool valid() noexcept override;

  void response(int err) override;

};


}}}}


#endif // swcdb_ranger_queries_select_CheckMeta_h
