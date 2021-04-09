/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_Select_Handler_Common_h
#define swcdb_db_client_Query_Select_Handler_Common_h


#include "swcdb/db/client/Query/SelectHandlerBaseUnorderedMap.h"
#include "swcdb/db/Cells/SpecsScan.h"


namespace SWC { namespace client { namespace Query { namespace Select {

namespace Handlers {


class Common : public BaseUnorderedMap {
  public:

  typedef std::shared_ptr<Common>   Ptr;
  typedef std::function<void(Ptr)>  Cb_t;

  static Ptr make(Cb_t&& cb=nullptr, bool rsp_partials=false,
                  const Comm::IoContextPtr& io=nullptr) {
    return std::make_shared<Common>(std::move(cb), rsp_partials, io);
  }

  Core::AtomicBool valid_state;

  Common(Cb_t&& cb, bool rsp_partials=false,
         const Comm::IoContextPtr& io=nullptr) noexcept;

  virtual ~Common() { }

  virtual bool valid() noexcept override;

  virtual bool add_cells(const cid_t cid, const StaticBuffer& buffer,
                         bool reached_limit,
                         DB::Specs::Interval& interval) override;

  virtual void get_cells(const cid_t cid, DB::Cells::Result& cells) override;

  virtual void free(const cid_t cid) override;

  virtual void response(int err) override;

  void wait();

  protected:
  std::mutex                           m_mutex;

  private:

  void response_partials();

  bool wait_on_partials();

  void send_result();

  const Cb_t                           m_cb;
  Comm::IoContextPtr                   m_dispatcher_io;
  const bool                           m_notify;
  std::condition_variable              m_cv;
  Core::StateRunning                   m_sending_result;

};


}}}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/SelectHandlerCommon.cc"
#endif


#endif // swcdb_db_client_Query_Select_Handler_Common_h
