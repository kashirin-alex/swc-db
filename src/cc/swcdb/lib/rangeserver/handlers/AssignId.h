/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swc_app_rangeserver_handlers_AssignId_h
#define swc_app_rangeserver_handlers_AssignId_h


namespace SWC { namespace server { namespace RS {

namespace Handler {

class AssignId : public AppHandler {
  public:

  AssignId(ConnHandlerPtr conn, EventPtr ev, 
              Protocol::Req::MngrRsMngId::Scheduler::Ptr validator)
             : AppHandler(conn, ev), validator(validator) { }

  void run() override {

    try {
      if(Env::RsData::is_shuttingdown()){
        m_conn->send_error(Error::SERVER_SHUTTING_DOWN, "", m_ev);
        return;
      }
      HT_DEBUG(" REQ_RS_ASSIGN_ID_NEEDED");
      m_conn->response_ok(m_ev);
      Protocol::Req::MngrRsMngId::assign(validator);
    }
    catch (Exception &e) {
      HT_ERROR_OUT << e << HT_END;
    }
  
  }

  private:
  Protocol::Req::MngrRsMngId::Scheduler::Ptr validator;
};
  

}}}}

#endif // swc_app_rangeserver_handlers_AssignId_h