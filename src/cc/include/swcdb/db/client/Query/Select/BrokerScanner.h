/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_Select_BrokerScanner_h
#define swcdb_db_client_Query_Select_BrokerScanner_h


#include "swcdb/db/client/Query/Select/Handlers/Base.h"
#include "swcdb/db/Protocol/Bkr/params/CellsSelect.h"


namespace SWC { namespace client { namespace Query { namespace Select {



class BrokerScanner : public std::enable_shared_from_this<BrokerScanner>  {
  public:

  SWC_CAN_INLINE
  static void execute(const Handlers::Base::Ptr& hdlr,
                      cid_t cid, const DB::Specs::Interval& intval) {
    Ptr(new BrokerScanner(hdlr, intval, cid))->select();
  }

  SWC_CAN_INLINE
  static void execute(const Handlers::Base::Ptr& hdlr,
                      cid_t cid, DB::Specs::Interval&& intval) {
    Ptr(new BrokerScanner(hdlr, std::move(intval), cid))->select();
  }


  typedef std::shared_ptr<BrokerScanner>  Ptr;
  Handlers::Base::Ptr                     selector;
  DB::Specs::Interval                     interval;
  const cid_t                             cid;

  SWC_CAN_INLINE
  BrokerScanner(const Handlers::Base::Ptr& hdlr,
                DB::Specs::Interval&& interval,
                const cid_t cid) noexcept
                : selector(hdlr), interval(std::move(interval)), cid(cid) {
  }

  SWC_CAN_INLINE
  BrokerScanner(const Handlers::Base::Ptr& hdlr,
                const DB::Specs::Interval& interval,
                const cid_t cid)
                : selector(hdlr), interval(interval), cid(cid) {
  }

  ~BrokerScanner();

  void print(std::ostream& out);

  private:

  void select();

  void selected(const ReqBase::Ptr& req,
                Comm::Protocol::Bkr::Params::CellsSelectRsp& rsp);

};


}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Select/BrokerScanner.cc"
#endif


#endif // swcdb_db_client_Query_Select_BrokerScanner_h
