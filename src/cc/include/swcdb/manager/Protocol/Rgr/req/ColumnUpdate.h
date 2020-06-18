
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */ 

#ifndef swc_manager_Protocol_rgr_req_ColumnUpdate_h
#define swc_manager_Protocol_rgr_req_ColumnUpdate_h



namespace SWC { namespace Protocol { namespace Rgr { namespace Req {

class ColumnUpdate : public client::ConnQueue::ReqBase {
  public:

  ColumnUpdate(const Manager::Ranger::Ptr& rgr, 
               const DB::Schema::Ptr& schema);
  
  virtual ~ColumnUpdate();

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;
  
  void handle_no_conn() override;

  void updated(int err, bool failure);

  private:

  Manager::Ranger::Ptr   rgr;
  DB::Schema::Ptr        schema; 
   
};

}}}}

#endif // swc_manager_Protocol_rgr_req_ColumnUpdate_h
