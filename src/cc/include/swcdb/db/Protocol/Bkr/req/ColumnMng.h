/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_protocol_bkr_req_ColumnMng_h
#define swcdb_db_protocol_bkr_req_ColumnMng_h


#include "swcdb/db/Protocol/Bkr/req/ColumnMng_Base.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Bkr { namespace Req {


class ColumnMng: public ColumnMng_Base {
  public:

  typedef std::function<void(const client::ConnQueue::ReqBase::Ptr&,
                             int)> Cb_t;

  static void create(const SWC::client::Clients::Ptr& clients,
                     const DB::Schema::Ptr& schema, Cb_t&& cb,
                     const uint32_t timeout = 10000);

  static void modify(const SWC::client::Clients::Ptr& clients,
                     const DB::Schema::Ptr& schema, Cb_t&& cb,
                     const uint32_t timeout = 10000);

  static void remove(const SWC::client::Clients::Ptr& clients,
                     const DB::Schema::Ptr& schema, Cb_t&& cb,
                     const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      Func func,
                      const DB::Schema::Ptr& schema, Cb_t&& cb,
                      const uint32_t timeout = 10000);

  static void request(const SWC::client::Clients::Ptr& clients,
                      const Mngr::Params::ColumnMng& params, Cb_t&& cb,
                      const uint32_t timeout = 10000);

  ColumnMng(const SWC::client::Clients::Ptr& clients,
            const Mngr::Params::ColumnMng& params, Cb_t&& cb,
            const uint32_t timeout);

  virtual ~ColumnMng() { }

  protected:
  virtual void callback(int err) override;

  private:
  const Cb_t                cb;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Protocol/Bkr/req/ColumnMng.cc"
#endif

#endif // swcdb_db_protocol_bkr_req_ColumnMng_h
