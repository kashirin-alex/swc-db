
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swc_manager_Protocol_rgr_req_RangeLoad_h
#define swc_manager_Protocol_rgr_req_RangeLoad_h

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


class RangeLoad : public Comm::client::ConnQueue::ReqBase {
  public:

  RangeLoad(const Manager::Ranger::Ptr& rgr, 
            const Manager::Range::Ptr& range,
            const DB::Schema::Ptr& schema);
  
  virtual ~RangeLoad();

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  bool valid() override;
  
  void handle_no_conn() override;

  void loaded(int err, bool failure, const DB::Cells::Interval& intval);


  private:

  Manager::Ranger::Ptr rgr;
  Manager::Range::Ptr  range;
  int64_t              schema_revision;
   
};

}}}}

#endif // swc_manager_Protocol_rgr_req_RangeLoad_h
