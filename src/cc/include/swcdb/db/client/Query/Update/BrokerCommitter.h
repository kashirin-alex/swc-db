/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_client_Query_Update_BrokerCommitter_h
#define swcdb_db_client_Query_Update_BrokerCommitter_h


#include "swcdb/db/client/Query/Update/Handlers/Base.h"
#include "swcdb/db/Protocol/Bkr/params/CellsUpdate.h"



namespace SWC { namespace client { namespace Query { namespace Update {



class BrokerCommitter final
    : public std::enable_shared_from_this<BrokerCommitter> {
  public:

  SWC_CAN_INLINE
  static void execute(const Handlers::Base::Ptr& hdlr,
                      Handlers::Base::Column* colp) {
    Ptr(new BrokerCommitter(hdlr, colp))->commit();
  }


  typedef std::shared_ptr<BrokerCommitter>  Ptr;
  Query::Update::Handlers::Base::Ptr        hdlr;
  Query::Update::Handlers::Base::Column*    colp;
  Core::CompletionCounter<>                 workload;

  SWC_CAN_INLINE
  BrokerCommitter(const Query::Update::Handlers::Base::Ptr& a_hdlr,
                  Query::Update::Handlers::Base::Column* a_colp) noexcept
                  : hdlr(a_hdlr), colp(a_colp) {
  }

  //~BrokerCommitter() { }

  void print(std::ostream& out);

  private:

  void commit();

  void committed(ReqBase::Ptr req,
                 const Comm::Protocol::Bkr::Params::CellsUpdateRsp& rsp,
                 const DynamicBuffer& cells_buff);

};


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Update/BrokerCommitter.cc"
#endif


#endif // swcdb_db_client_Query_Update_BrokerCommitter_h
