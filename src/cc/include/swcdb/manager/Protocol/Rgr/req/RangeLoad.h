
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_manager_Protocol_rgr_req_RangeLoad_h
#define swc_manager_Protocol_rgr_req_RangeLoad_h

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


class RangeLoad : public client::ConnQueue::ReqBase {
  public:

  RangeLoad(Manager::Ranger::Ptr rgr, Manager::Range::Ptr range,
            DB::Schema::Ptr schema);
  
  virtual ~RangeLoad();

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override;

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
