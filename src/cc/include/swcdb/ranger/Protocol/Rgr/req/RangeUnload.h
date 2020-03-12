
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */ 

#ifndef swc_ranger_protocol_rgr_req_RangeUnload_h
#define swc_ranger_protocol_rgr_req_RangeUnload_h



namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


class RangeUnload : public client::ConnQueue::ReqBase {
  public:

  RangeUnload(Ranger::RangePtr range, ResponseCallback::Ptr cb,
              uint32_t timeout=60000);

  virtual ~RangeUnload();

  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override;

  bool valid() override;
  
  void handle_no_conn() override;

  void unloaded(int err, ResponseCallback::Ptr cb);

  private:

  ResponseCallback::Ptr cb;
  Ranger::RangePtr      range;
   
};

}}}}

#endif // swc_ranger_protocol_rgr_req_RangeUnload_h
