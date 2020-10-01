
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */ 

#ifndef swcdb_manager_Protocol_rgr_req_ColumnUpdate_h
#define swcdb_manager_Protocol_rgr_req_ColumnUpdate_h



namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Req {

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

}}}}}

#endif // swcdb_manager_Protocol_rgr_req_ColumnUpdate_h
