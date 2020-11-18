
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swcdb_db_client_Query_Select_h
#define swcdb_db_client_Query_Select_h


#include "swcdb/core/CompletionCounter.h"
#include "swcdb/db/Types/Range.h"
#include "swcdb/db/Cells/Result.h"
#include "swcdb/db/Cells/SpecsScan.h"

#include "swcdb/db/client/Query/Profiling.h"
#include "swcdb/db/Protocol/Mngr/req/RgrGet.h"
#include "swcdb/db/Protocol/Rgr/req/RangeLocate.h"
#include "swcdb/db/Protocol/Rgr/req/RangeQuerySelect.h"


namespace SWC { namespace client { namespace Query {


namespace Result {

struct Select final {
  typedef std::shared_ptr<Select> Ptr;

  const bool                                  notify;
  std::mutex                                  mutex;
  std::condition_variable                     cv;
  std::atomic<int>                            err;
  Profiling                                   profile;
  Core::CompletionCounter<uint64_t>           completion;

  Select(bool notify);

  ~Select();

  struct Rsp final {
    public:
    typedef std::shared_ptr<Rsp> Ptr;

    Rsp();

    ~Rsp();

    bool add_cells(const StaticBuffer& buffer, bool reached_limit, 
                   DB::Specs::Interval& interval); 
    
    void get_cells(DB::Cells::Result& cells);

    size_t get_size();

    size_t get_size_bytes();

    bool empty();

    void free();

    void error(int err);

    int error();

    private:
    Core::MutexSptd    m_mutex;
    DB::Cells::Result  m_cells;
    int                m_err;
  };


  void error(const cid_t cid, int err);

  void add_column(const cid_t cid);
  
  Rsp::Ptr get_columnn(const cid_t cid);

  bool add_cells(const cid_t cid, const StaticBuffer& buffer, 
                 bool reached_limit, DB::Specs::Interval& interval);

  void get_cells(const cid_t cid, DB::Cells::Result& cells);

  size_t get_size(const cid_t cid);

  size_t get_size_bytes();

  bool empty() const;

  std::vector<cid_t> get_cids() const;

  void free(const cid_t cid);

  void remove(const cid_t cid);
  
  private:

  std::unordered_map<cid_t, Rsp::Ptr>  m_columns;

};

}

class Select final : public std::enable_shared_from_this<Select> {
  public:

  using Result = Result::Select;

  typedef std::shared_ptr<Select>           Ptr;
  typedef std::function<void(Result::Ptr)>  Cb_t;
  
  uint32_t          buff_sz;
  uint8_t           buff_ahead;
  uint32_t          timeout;

  const Cb_t        cb;
  DB::Specs::Scan   specs;
  Result::Ptr       result;

  Select(const Cb_t& cb=0, bool rsp_partials=false);

  Select(const DB::Specs::Scan& specs, 
         const Cb_t& cb=0, bool rsp_partials=false);

  virtual ~Select();
 
  void response(int err=Error::OK);

  void response_partials();

  bool wait_on_partials() const;

  void wait();

  void scan(int& err);

  private:

  bool m_rsp_partial_runs;

  void response_partial();


  class Scanner final : public std::enable_shared_from_this<Scanner> {
    public:

    typedef std::shared_ptr<Scanner>        Ptr;

    Core::CompletionCounter<uint64_t>       completion;

    Select::Ptr                             selector;
    const DB::Types::KeySeq                 col_seq;
    DB::Specs::Interval                     interval;

    const cid_t                             master_cid;
    const cid_t                             meta_cid;
    const cid_t                             data_cid;

    cid_t                                   master_rid;
    cid_t                                   meta_rid;
    cid_t                                   data_rid;

    bool                                    master_mngr_next;
    bool                                    master_rgr_next;
    bool                                    meta_next;

    ReqBase::Ptr                            master_rgr_req_base;
    ReqBase::Ptr                            meta_req_base;
    ReqBase::Ptr                            data_req_base;

    DB::Cell::Key                           master_mngr_offset;
    DB::Cell::Key                           master_rgr_offset;
    DB::Cell::Key                           meta_offset;
    // DB::Cell::Key                        data_offset;

    Comm::EndPoints                         master_rgr_endpoints;
    Comm::EndPoints                         meta_endpoints;
    Comm::EndPoints                         data_endpoints;

    Scanner(const Select::Ptr& selector,
            const DB::Types::KeySeq col_seq, 
            DB::Specs::Interval& interval, 
            const cid_t cid);

    virtual ~Scanner();

    void print(std::ostream& out);

    bool add_cells(const StaticBuffer& buffer, bool reached_limit);

    void response_if_last();

    void next_call();


    void mngr_locate_master();

    bool mngr_located_master(
        const ReqBase::Ptr& base,
        const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp);


    void rgr_locate_master();

    void rgr_located_master(
        const ReqBase::Ptr& base,
        const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp);


    void mngr_resolve_rgr_meta();

    bool mngr_resolved_rgr_meta(
        const ReqBase::Ptr& base,
        const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp);


    void rgr_locate_meta();

    void rgr_located_meta(
        const ReqBase::Ptr& base,
        const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp);


    void mngr_resolve_rgr_select();

    bool mngr_resolved_rgr_select(
        const ReqBase::Ptr& base,
        const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp);


    void rgr_select();

    void rgr_selected(
        const ReqBase::Ptr& req,
        const Comm::Protocol::Rgr::Params::RangeQuerySelectRsp& rsp);

  };
  
};

}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Select.cc"
#endif 


#endif // swcdb_db_client_Query_Select_h
