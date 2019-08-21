
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_MngColumn_h
#define swc_lib_db_protocol_req_MngColumn_h


#include "swcdb/lib/db/Protocol/Commands.h"
#include "swcdb/lib/db/Columns/Schema.h"

#include "ActiveMngrBase.h"
#include "../params/MngColumn.h"
#include "../rsp/CodeHandler.h"


namespace SWC {
namespace Protocol {
namespace Req {


class MngColumn: public ActiveMngrBase {
  public:

  using Function = Params::MngColumn::Function;

  struct Req : public Rsp::CodeHandler::Req {
    public:

    using BasePtr = Rsp::CodeHandler::Req::Ptr;
    typedef std::shared_ptr<Req> Ptr;

    static Ptr add(DB::SchemaPtr schema){
      return std::make_shared<Req>(Function::ADD, schema);
    }

    Req(Function function, DB::SchemaPtr schema)
       : function(function), schema(schema) {}

    Req(Function function, DB::SchemaPtr schema, 
        Rsp::CodeHandler::Req::Cb_t cb)
       : function(function), schema(schema), Rsp::CodeHandler::Req(cb) {}

    virtual ~Req(){}

    Function      function;
    DB::SchemaPtr schema;
  };


  MngColumn(std::atomic<int> &progress)
            : ActiveMngrBase(1, 1), 
              m_running(false), m_conn(nullptr), progress(progress) { }

  virtual ~MngColumn(){}


  void create(DB::SchemaPtr schema, Rsp::CodeHandler::Req::Cb_t cb){
    return make(std::make_shared<Req>(Function::ADD, schema, cb));
  }


  void run(EndPoints endpoints) override {

    if(m_conn != nullptr && m_conn->is_open()){
      make_requests(nullptr, false);
      return;
    }
    EnvClients::get()->mngr_service->get_connection(
      endpoints, 
      [ptr=shared_from_this()]
      (client::ClientConPtr conn){
        std::dynamic_pointer_cast<MngColumn>(ptr)->make_requests(conn, true);
      },
      std::chrono::milliseconds(timeout_ms), 
      1
    );
  }

  void make(Req::Ptr req) {
    {
      std::lock_guard<std::mutex> lock(m_mutex_queue);
      m_queue.push(req);
    }

    if(!m_running) {
      m_running = true;

      make_requests(nullptr, false);
    }
  }

  void make_requests(client::ClientConPtr conn, bool new_conn) {
    if(new_conn)
      m_conn = conn;

    if(m_conn == nullptr || !m_conn->is_open()){
      m_conn = nullptr;
      if(!new_conn)
        ActiveMngrBase::run();
      else
        run_within(EnvClients::get()->mngr_service->io(), 200);
      return;
    }
    
    Req::Ptr req;
    uint32_t sz;
    for(;;) {
      {
        std::lock_guard<std::mutex> lock(m_mutex_queue);
        sz = m_queue.size();
        m_running = m_queue.size() > 0;
        if(!m_running)
          break;
        req = m_queue.front();
        m_queue.pop();
      }
      std::cout << " make_requests queue sz=" << sz << " progress= " << progress << ")\n";

      Protocol::Params::MngColumn params(req->function, req->schema);

      CommHeader header(Protocol::Command::CLIENT_REQ_MNG_COLUMN, sz*60000);
      CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
      params.encode(cbp->get_data_ptr_address());
      
      if(m_conn->send_request(cbp, std::make_shared<Rsp::CodeHandler>(req))
         == Error::OK)
        continue;

      {
        std::lock_guard<std::mutex> lock(m_mutex_queue);
        m_queue.push(req);
      }

      run_within(EnvClients::get()->mngr_service->io(), 500);
      return;
    }

    // m_conn->close(); pending-rsps?
    //EnvClients::get()->mngr_service->preserve(conn);
    
  }

  private:
  
  std::atomic<bool>     m_running;
  client::ClientConPtr  m_conn;
  
  std::mutex            m_mutex_queue;
  std::queue<Req::Ptr>  m_queue;

  std::atomic<int> &progress;

};
typedef std::shared_ptr<MngColumn> MngColumnPtr;


}}}

#endif // swc_lib_db_protocol_req_MngColumn_h
