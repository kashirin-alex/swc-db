
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swcdb_manager_Protocol_rgr_req_ColumnDelete_h
#define swcdb_manager_Protocol_rgr_req_ColumnDelete_h


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {
  

class ColumnDelete : public client::ConnQueue::ReqBase  {
  public:

  ColumnDelete(const Manager::Ranger::Ptr& rgr, cid_t cid);
  
  virtual ~ColumnDelete();
  
  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

  void handle_no_conn() override;
  
  void remove(int err);

  private:

  Manager::Ranger::Ptr  rgr;
  cid_t                 cid;
};

}}}}}

#endif // swcdb_manager_Protocol_rgr_req_ColumnDelete_h
