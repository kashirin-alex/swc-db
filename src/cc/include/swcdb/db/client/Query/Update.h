
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swc_db_client_Query_Update_h
#define swc_db_client_Query_Update_h

#include "swcdb/db/Cells/MapMutable.h" 
#include "swcdb/db/Types/Range.h"

#include "swcdb/db/Protocol/Mngr/req/RgrGet.h"
#include "swcdb/db/Protocol/Rgr/req/RangeLocate.h"
#include "swcdb/db/client/Query/Profiling.h"

namespace SWC { namespace client { namespace Query {

using ReqBase = client::ConnQueue::ReqBase;

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

  DB::Cells::MapMutable errored;
  Profiling             profile;
  
  uint32_t completion();

  void completion_incr();

  bool completion_final();

  int error();

  void error(int err);

  void add_resend_count(size_t count);

  size_t get_resend_count(bool reset = true);

  private:
  Mutex      m_mutex;
  uint32_t   m_completion = 0;
  int        m_err = Error::OK;
  size_t     m_resend_cells = 0;
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
  DB::Cells::MapMutable::Ptr  columns;
  DB::Cells::MapMutable::Ptr  columns_onfractions;

  Result::Ptr                 result;

  std::mutex                  m_mutex;
  std::condition_variable     cv;

  Update(const Cb_t& cb=0);

  Update(const DB::Cells::MapMutable::Ptr& columns, 
         const DB::Cells::MapMutable::Ptr& columns_onfractions, 
         const Cb_t& cb=0);

  virtual ~Update();
 
  void response(int err=Error::OK);

  void wait();

  bool wait_ahead_buffers();

  void commit_or_wait(const DB::Cells::ColCells::Ptr& col = nullptr);

  void commit_if_need();
  

  void commit();

  void commit(const cid_t cid);

  void commit(const DB::Cells::ColCells::Ptr& col);

  void commit_onfractions(const DB::Cells::ColCells::Ptr& col);

  class Locator final : public std::enable_shared_from_this<Locator> {
    public:
    const Types::Range        type;
    const cid_t               cid;
    DB::Cells::ColCells::Ptr  col;
    DB::Cell::Key::Ptr        key_start;
    Update::Ptr               updater;
    ReqBase::Ptr              parent;
    const rid_t               rid;
    const DB::Cell::Key       key_finish;
    
    Locator(const Types::Range type, const cid_t cid, 
            const DB::Cells::ColCells::Ptr& col, 
            const DB::Cell::Key::Ptr& key_start,
            const Update::Ptr& updater, const ReqBase::Ptr& parent=nullptr, 
            const rid_t rid=0, const DB::Cell::Key* key_finish=nullptr);

    virtual ~Locator();

    std::string to_string();

    void locate_on_manager();

    private:

    bool located_on_manager(const ReqBase::Ptr& base, 
                            const Protocol::Mngr::Params::RgrGetRsp& rsp);

    void locate_on_ranger(const EndPoints& endpoints);

    bool located_on_ranger(const EndPoints& endpoints, 
                           const ReqBase::Ptr& base, 
                           const Protocol::Rgr::Params::RangeLocateRsp& rsp);

    void resolve_on_manager();

    bool located_ranger(const ReqBase::Ptr& base, 
                        const Protocol::Mngr::Params::RgrGetRsp& rsp);

    bool proceed_on_ranger(const ReqBase::Ptr& base, 
                           const Protocol::Mngr::Params::RgrGetRsp& rsp);

    void commit_data(const EndPoints& endpoints, const ReqBase::Ptr& base);

  };

};


}}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Update.cc"
#endif 


#endif // swc_db_client_Query_Update_h
