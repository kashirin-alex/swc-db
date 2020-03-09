
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_client_requests_Query_Select_h
#define swc_client_requests_Query_Select_h

#include "swcdb/db/Cells/Vector.h"
#include "swcdb/db/Cells/SpecsScan.h"

#include "swcdb/db/client/Query/Update.h"

#include "swcdb/db/Protocol/Rgr/req/RangeQuerySelect.h"

namespace SWC { namespace client { namespace Query {
 
namespace Result{

struct Select final {
  typedef std::shared_ptr<Select> Ptr;

  std::atomic<uint32_t>  completion = 0;
  std::atomic<int>       err = Error::OK;
  std::atomic<bool>      notify;

  Select(std::condition_variable& cv, bool notify);

  ~Select();

  struct Rsp final {
    public:
    typedef std::shared_ptr<Rsp> Ptr;

    Rsp();

    ~Rsp();

    const bool add_cells(const StaticBuffer& buffer, bool reached_limit, 
                         DB::Specs::Interval& interval); 
    
    void get_cells(DB::Cells::Vector& cells);

    const size_t get_size();

    const size_t get_size_bytes();

    void free();
  
    private:
    std::mutex         m_mutex;
    DB::Cells::Vector  m_cells;
    size_t             m_counted = 0;
    size_t             m_size_bytes = 0;
  };


  void add_column(const int64_t cid);
  
  const bool add_cells(const int64_t cid, const StaticBuffer& buffer, 
                       bool reached_limit, DB::Specs::Interval& interval);

  void get_cells(const int64_t cid, DB::Cells::Vector& cells);

  const size_t get_size(const int64_t cid);

  const size_t get_size_bytes();

  const std::vector<int64_t> get_cids() const;

  const void free(const int64_t cid);

  void remove(const int64_t cid);
  
  private:

  std::unordered_map<int64_t, Rsp::Ptr> m_columns;
  std::condition_variable&              m_cv;

};

}

class Select : public std::enable_shared_from_this<Select> {
  public:

  using Result = Result::Select;

  typedef std::shared_ptr<Select>           Ptr;
  typedef std::function<void(Result::Ptr)>  Cb_t;
  
  uint32_t          buff_sz;
  uint8_t           buff_ahead;
  uint32_t          timeout;

  Cb_t              cb;
  DB::Specs::Scan   specs;
  Result::Ptr       result;

  Select(Cb_t cb=0, bool rsp_partials=false);

  Select(const DB::Specs::Scan& specs, Cb_t cb=0, bool rsp_partials=false);

  virtual ~Select();
 
  void response(int err=Error::OK);

  void response_partials();

  const bool wait_on_partials() const;

  void response_partial();

  void wait();

  void scan();

  private:
  
  const bool                  rsp_partials;
  std::mutex                  m_mutex;
  bool                        m_rsp_partial_runs = false;
  std::condition_variable     m_cv;


  class ScannerColumn : public std::enable_shared_from_this<ScannerColumn> {
    public:

    typedef std::shared_ptr<ScannerColumn>  Ptr;
    const int64_t                           cid;
    DB::Specs::Interval                     interval;
    Select::Ptr                             selector;

    ScannerColumn(const int64_t cid, DB::Specs::Interval& interval,
                  Select::Ptr selector);

    virtual ~ScannerColumn();

    void run();

    const bool add_cells(const StaticBuffer& buffer, bool reached_limit);

    void next_call(bool final=false);

    void add_call(const std::function<void()>& call);

    const std::string to_string();

    private:

    std::vector<std::function<void()>> next_calls;

  };

  class Scanner: public std::enable_shared_from_this<Scanner> {
    public:
    const Types::Range        type;
    const int64_t             cid;
    ScannerColumn::Ptr        col;

    ReqBase::Ptr              parent;
    const int64_t             rid;
    DB::Cell::Key             range_offset;

    Scanner(const Types::Range type, const int64_t cid, ScannerColumn::Ptr col,
            ReqBase::Ptr parent=nullptr, 
            const DB::Cell::Key* range_offset=nullptr, const int64_t rid=0);

    virtual ~Scanner();

    const std::string to_string();
    
    void locate_on_manager(bool next_range=false);

    void resolve_on_manager();

    bool located_on_manager(const ReqBase::Ptr& base, 
                            const Protocol::Mngr::Params::RgrGetRsp& rsp, 
                            bool next_range=false);
    
    bool proceed_on_ranger(const ReqBase::Ptr& base, 
                           const Protocol::Mngr::Params::RgrGetRsp& rsp);

    void locate_on_ranger(const EndPoints& endpoints, bool next_range=false);

    bool located_on_ranger(const EndPoints& endpoints, 
                           const ReqBase::Ptr& base, 
                           const Protocol::Rgr::Params::RangeLocateRsp& rsp, 
                           bool next_range=false);

    void select(EndPoints endpoints, uint64_t rid, const ReqBase::Ptr& base);

  };
  
};

}}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/Query/Select.cc"
#endif 


#endif // swc_client_requests_Query_Select_h
