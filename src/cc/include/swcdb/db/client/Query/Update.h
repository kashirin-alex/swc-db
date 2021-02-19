
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_Query_Update_h
#define swcdb_db_client_Query_Update_h


#include "swcdb/core/CompletionCounter.h"
#include "swcdb/db/Types/Range.h"
#include "swcdb/db/Cells/MutableMap.h"

#include "swcdb/db/client/Query/Profiling.h"
#include "swcdb/db/Protocol/Mngr/req/RgrGet.h"
#include "swcdb/db/Protocol/Rgr/req/RangeLocate.h"


namespace SWC { namespace client { namespace Query {


/*
range-master:
  req-mngr.   cid({1,4}) + [n(cid), next_key_start]
              => cid({1,4}) + rid + rgr(endpoints) + range_begin + range_end
    req-rgr.  cid({1,4}) + rid + [cid(n), next_key_start]
              => cid({5,8}) + rid + range_begin + range_end
range-meta:
  req-mngr.   cid({5,8}) + rid
              => cid({5,8}) + rid + rgr(endpoints)
    req-rgr.  cid({5,8}) + rid + [cid(n), next_key_start]
              => cid(n) + rid + range_begin + range_end
range-data:
  req-mngr.   cid(n) + rid
              => cid(n) + rid + rgr(endpoints)
    req-rgr.  cid(n) + rid + Specs::Interval
              => results
*/

namespace Result {

struct Update final {
  public:

  typedef std::shared_ptr<Update> Ptr;

  DB::Cells::MutableMap              errored;
  Profiling                          profile;
  Core::CompletionCounter<uint64_t>  completion;

  Update() noexcept;

  int error() noexcept;

  void error(int err) noexcept;

  void add_resend_count(size_t count) noexcept;

  size_t get_resend_count(bool reset = true) noexcept;

  private:
  Core::Atomic<int>    m_err;
  Core::Atomic<size_t> m_resend_cells;
};

}


class Update final : public std::enable_shared_from_this<Update> {
  public:

  using Result = Result::Update;

  typedef std::shared_ptr<Update>                  Ptr;
  typedef std::function<void(const Result::Ptr&)>  Cb_t;

  uint32_t                    buff_sz;
  uint8_t                     buff_ahead;
  uint32_t                    timeout;
  uint32_t                    timeout_ratio;

  const Cb_t                  cb;
  Comm::IoContextPtr          dispatcher_io;

  DB::Cells::MutableMap::Ptr  columns;
  DB::Cells::MutableMap::Ptr  columns_onfractions;

  Result::Ptr                 result;

  std::mutex                  m_mutex;
  std::condition_variable     cv;

  Update(const Cb_t& cb=nullptr, const Comm::IoContextPtr& io=nullptr);

  Update(const DB::Cells::MutableMap::Ptr& columns,
         const DB::Cells::MutableMap::Ptr& columns_onfractions,
         const Cb_t& cb=nullptr, const Comm::IoContextPtr& io=nullptr);

  virtual ~Update();

  void response(int err=Error::OK);

  void wait();

  bool wait_ahead_buffers(uint64_t from=0);

  void commit_or_wait(const DB::Cells::ColCells::Ptr& col = nullptr,
                      uint64_t from=0);

  void commit_if_need();


  void commit();

  void commit(const cid_t cid);

  void commit(const DB::Cells::ColCells::Ptr& col);

  void commit_onfractions(const DB::Cells::ColCells::Ptr& col);

  class Locator final : public std::enable_shared_from_this<Locator> {
    public:
    const DB::Types::Range    type;
    const cid_t               cid;
    DB::Cells::ColCells::Ptr  col;
    DB::Cell::Key::Ptr        key_start;
    Update::Ptr               updater;
    ReqBase::Ptr              parent;
    const rid_t               rid;
    const DB::Cell::Key       key_finish;

    Locator(const DB::Types::Range type,
            const cid_t cid,
            const DB::Cells::ColCells::Ptr& col,
            const DB::Cell::Key::Ptr& key_start,
            const Update::Ptr& updater,
            const ReqBase::Ptr& parent=nullptr,
            const rid_t rid=0) noexcept;

    Locator(const DB::Types::Range type,
            const cid_t cid,
            const DB::Cells::ColCells::Ptr& col,
            const DB::Cell::Key::Ptr& key_start,
            const Update::Ptr& updater,
            const ReqBase::Ptr& parent,
            const rid_t rid,
            const DB::Cell::Key& key_finish);

    virtual ~Locator();

    void print(std::ostream& out);

    void locate_on_manager();

    private:

    void located_on_manager(
        const ReqBase::Ptr& base,
        const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp);

    void locate_on_ranger(
        const Comm::EndPoints& endpoints);

    void located_on_ranger(
        const Comm::EndPoints& endpoints,
        const ReqBase::Ptr& base,
        const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp);

    void resolve_on_manager();

    void located_ranger(
        const ReqBase::Ptr& base,
        const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp);

    void proceed_on_ranger(
        const ReqBase::Ptr& base,
        const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp);

    void commit_data(
        const Comm::EndPoints& endpoints,
        const ReqBase::Ptr& base);

  };

};


}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Update.cc"
#endif


#endif // swcdb_db_client_Query_Update_h
