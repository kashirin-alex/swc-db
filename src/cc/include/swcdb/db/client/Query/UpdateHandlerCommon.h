
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_UpdateHandlerCommon_h
#define swcdb_db_client_Query_UpdateHandlerCommon_h


#include "swcdb/db/client/Query/UpdateHandlerBaseUnorderedMap.h"


namespace SWC { namespace client { namespace Query { namespace Update {

namespace Handlers {


class Common : public BaseUnorderedMap {
  public:
  typedef std::shared_ptr<Common>                 Ptr;
  typedef std::function<void(const Common::Ptr&)> Cb_t;

  static Ptr make(const Cb_t& cb=nullptr,
                  const Comm::IoContextPtr& io=nullptr) {
    return std::make_shared<Common>(cb, io);
  }

  using Base::error;
  
  Core::AtomicBool valid_state;

  Common(const Cb_t& cb=nullptr, const Comm::IoContextPtr& io=nullptr);

  virtual ~Common() { }


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
#include "swcdb/db/client/Query/UpdateHandlerCommon.cc"
#endif


#endif // swcdb_db_client_Query_UpdateHandlerCommon_h
