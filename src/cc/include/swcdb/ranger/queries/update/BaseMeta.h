/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_ranger_queries_update_BaseMeta_h
#define swcdb_ranger_queries_update_BaseMeta_h


#include "swcdb/db/client/Query/UpdateHandlerBaseSingleColumn.h"


namespace SWC { namespace Ranger { namespace Query { namespace Update {


class BaseMeta : public client::Query::Update::Handlers::BaseSingleColumn {
  public:
  typedef std::shared_ptr<BaseMeta> Ptr;

  RangePtr range;

  BaseMeta(const RangePtr& range);

  virtual ~BaseMeta() { }

  virtual bool valid() noexcept override;

  virtual void callback() = 0;

  bool is_last_rsp(int err);

};


}}}}

#endif // swcdb_ranger_queries_update_BaseMeta_h
