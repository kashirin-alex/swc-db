
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swc_manager_Protocol_rgr_req_ColumnsUnload_h
#define swc_manager_Protocol_rgr_req_ColumnsUnload_h


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {
  

class ColumnsUnload : public client::ConnQueue::ReqBase  {
  public:

  ColumnsUnload(const Manager::Ranger::Ptr& rgr, 
                cid_t cid_begin, cid_t cid_end);
  
  virtual ~ColumnsUnload();
  
  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  void handle_no_conn() override;
  
  private:
  
  const Manager::Ranger::Ptr  rgr;
  cid_t                       cid_begin; 
  cid_t                       cid_end;

};

}}}}

#endif // swc_manager_Protocol_rgr_req_ColumnsUnload_h
