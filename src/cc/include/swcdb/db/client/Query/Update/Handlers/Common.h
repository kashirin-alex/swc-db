/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_Update_Handlers_Common_h
#define swcdb_db_client_Query_Update_Handlers_Common_h


#include "swcdb/db/client/Query/Update/Handlers/BaseUnorderedMap.h"


namespace SWC { namespace client { namespace Query { namespace Update {

namespace Handlers {


class Common : public BaseUnorderedMap {
  public:
  typedef std::shared_ptr<Common>                 Ptr;
  typedef std::function<void(const Common::Ptr&)> Cb_t;

  SWC_CAN_INLINE
  static Ptr make(const Clients::Ptr& clients,
                  Cb_t&& cb=nullptr, const Comm::IoContextPtr& io=nullptr,
                  Clients::Flag executor=Clients::DEFAULT) {
    return Ptr(new Common(clients, std::move(cb), io, executor));
  }

  using Base::error;

  Core::AtomicBool valid_state;

  Common(const Clients::Ptr& clients,
         Cb_t&& cb, const Comm::IoContextPtr& io=nullptr,
         Clients::Flag executor=Clients::DEFAULT) noexcept;

  virtual ~Common() { }

  virtual bool requires_commit() noexcept override;

  virtual bool valid() noexcept override;

  virtual void response(int err=Error::OK) override;


  void wait();

  bool wait_ahead_buffers(uint64_t from=0);

  void commit_or_wait(Base::Column* colp=nullptr, uint64_t from=0);

  void commit_if_need();


  private:
  const Cb_t                  m_cb;
  Comm::IoContextPtr          m_dispatcher_io;
  std::mutex                  m_mutex;
  std::condition_variable     m_cv;
};


}}}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Update/Handlers/Common.cc"
#endif


#endif // swcdb_db_client_Query_Update_Handlers_Common_h
