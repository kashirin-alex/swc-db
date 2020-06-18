
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */ 

#ifndef swc_manager_Protocol_rgr_req_AssignIdNeeded_h
#define swc_manager_Protocol_rgr_req_AssignIdNeeded_h

namespace SWC { namespace Protocol { namespace Rgr { namespace Req {
  

class AssignIdNeeded : public client::ConnQueue::ReqBase {
  public:

  AssignIdNeeded(const Manager::Ranger::Ptr& rs_chk, 
                 const Manager::Ranger::Ptr& rs_nxt, 
                 const Manager::Range::Ptr& range) ;
  
  virtual ~AssignIdNeeded();

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  bool valid() override;

  void handle_no_conn() override;

  void rsp(int err);

  Manager::Ranger::Ptr rs_nxt;
  Manager::Range::Ptr  range;

  private:
  Manager::Ranger::Ptr rs_chk;
};

}}}}

#endif // swc_manager_Protocol_rgr_req_AssignIdNeeded_h
