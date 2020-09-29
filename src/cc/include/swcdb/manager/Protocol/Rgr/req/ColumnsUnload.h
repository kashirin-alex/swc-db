
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swc_manager_Protocol_rgr_req_ColumnsUnload_h
#define swc_manager_Protocol_rgr_req_ColumnsUnload_h


namespace SWC { namespace Protocol { namespace Rgr { namespace Req {
  

class ColumnsUnload : public Comm::client::ConnQueue::ReqBase  {
  public:

  ColumnsUnload(const Manager::Ranger::Ptr& rgr, 
                cid_t cid_begin, cid_t cid_end);
  
  virtual ~ColumnsUnload();
  
  void handle(Comm::ConnHandlerPtr conn, const Comm::Event::Ptr& ev) override;

  void handle_no_conn() override;
  
  private:
  
  const Manager::Ranger::Ptr  rgr;
  cid_t                       cid_begin; 
  cid_t                       cid_end;

};

}}}}

#endif // swc_manager_Protocol_rgr_req_ColumnsUnload_h
