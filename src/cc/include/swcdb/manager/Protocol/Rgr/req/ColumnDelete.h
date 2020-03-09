
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */ 

#ifndef swc_manager_Protocol_rgr_req_ColumnDelete_h
#define swc_manager_Protocol_rgr_req_ColumnDelete_h


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {
  

class ColumnDelete : public client::ConnQueue::ReqBase  {
  public:

  ColumnDelete(Manager::Ranger::Ptr rgr, int64_t cid);
  
  virtual ~ColumnDelete();
  
  void handle(ConnHandlerPtr conn, Event::Ptr& ev) override;

  void handle_no_conn() override;
  
  void remove(int err);

  private:

  Manager::Ranger::Ptr   rgr;
  int64_t                cid;
};

}}}}

#endif // swc_manager_Protocol_rgr_req_ColumnDelete_h
