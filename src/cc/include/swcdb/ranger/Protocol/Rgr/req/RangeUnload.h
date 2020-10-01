
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swcdb_ranger_protocol_rgr_req_RangeUnload_h
#define swcdb_ranger_protocol_rgr_req_RangeUnload_h



namespace SWC { namespace Protocol { namespace Rgr { namespace Req {


class RangeUnload : public Comm::client::ConnQueue::ReqBase {
  public:

  RangeUnload(const Ranger::RangePtr& range, 
              const Comm::ResponseCallback::Ptr& cb,
              uint32_t timeout=60000);

  virtual ~RangeUnload();

  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  bool valid() override;
  
  void handle_no_conn() override;

  void unloaded(int err);

  private:

  Comm::ResponseCallback::Ptr cb;
  Ranger::RangePtr            range;
   
};

}}}}

#endif // swcdb_ranger_protocol_rgr_req_RangeUnload_h
