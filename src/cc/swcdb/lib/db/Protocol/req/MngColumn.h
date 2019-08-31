
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_MngColumn_h
#define swc_lib_db_protocol_req_MngColumn_h


#include "swcdb/lib/db/Protocol/Commands.h"

#include "ActiveMngrBase.h"
#include "CodeHandler.h"
#include "../params/MngColumn.h"


namespace SWC {
namespace Protocol {
namespace Req {


class MngColumn: public ActiveMngrBase {
  public:

  using Function = Params::MngColumn::Function;

  struct Req : public CodeHandler::Req {
    public:

    using BasePtr = CodeHandler::Req::Ptr;
    typedef std::shared_ptr<Req> Ptr;

    static Ptr add(DB::SchemaPtr schema){
      return std::make_shared<Req>(Function::CREATE, schema);
    }

    Req(Function function, DB::SchemaPtr schema)
       : function(function), schema(schema) {}

    Req(Function function, DB::SchemaPtr schema, 
        CodeHandler::Req::Cb_t cb)
       : function(function), schema(schema), CodeHandler::Req(cb) {}

    virtual ~Req(){}

    Function      function;
    DB::SchemaPtr schema;
  };


  MngColumn(uint32_t timeout=60000)
            : ActiveMngrBase(1, 1), 
              m_conn(nullptr), m_timeout(timeout) { }

  virtual ~MngColumn(){}

  
  void request(Function func, DB::SchemaPtr schema, CodeHandler::Req::Cb_t cb){
    return make(std::make_shared<Req>(func, schema, cb));
  }

  void create(DB::SchemaPtr schema, CodeHandler::Req::Cb_t cb){
    return request(Function::CREATE, schema, cb);
  }

  void remove(DB::SchemaPtr schema, CodeHandler::Req::Cb_t cb){
    return request(Function::DELETE, schema, cb);
  }


  void run(const EndPoints& endpoints) override {

    if(m_conn != nullptr && m_conn->is_open()){
      make_requests(nullptr, false);
      return;
    }
    Env::Clients::get()->mngr_service->get_connection(
      endpoints, 
      [ptr=shared_from_this()]
      (client::ClientConPtr conn){
        std::dynamic_pointer_cast<MngColumn>(ptr)->make_requests(conn, true);
      },
      std::chrono::milliseconds(timeout_ms), 
      1
    );
  }

  inline void make(Req::Ptr req) {
    {
      std::lock_guard<std::mutex> lock(m_mutex_queue);
      m_queue.push(req);
      if(m_queue.size() > 1)
        return;
    }
    make_requests(nullptr, false);  
  }

  void make_requests(client::ClientConPtr conn, bool new_conn) {
    if(new_conn)
      m_conn = conn;

    if(m_conn == nullptr || !m_conn->is_open()){
      m_conn = nullptr;
      if(!new_conn)
        ActiveMngrBase::run();
      else
        run_within(Env::Clients::get()->mngr_service->io(), 200);
      return;
    }
    
    Req::Ptr req;
    uint32_t sz = 0;
    uint32_t len = 0;
    for(;;) {
      {
        std::lock_guard<std::mutex> lock(m_mutex_queue);
        sz = m_queue.size();
        if(sz == 0)
          break;
        req = m_queue.front();
      }
      

      Protocol::Params::MngColumn params(req->function, req->schema);

      CommHeader header(Protocol::Command::CLIENT_REQ_MNG_COLUMN, sz*m_timeout);
      CommBufPtr cbp = std::make_shared<CommBuf>(header, params.encoded_length());
      params.encode(cbp->get_data_ptr_address());
      
      if(m_conn->send_request(
        cbp, std::make_shared<CodeHandler>(req)) == Error::OK){

        std::lock_guard<std::mutex> lock(m_mutex_queue);
        m_queue.pop();
        if(m_queue.empty())
          return;
        continue;  
      }

      run_within(Env::Clients::get()->mngr_service->io(), 500);
      return;
    }

    if(!due()) 
      m_conn->close();
  }

  size_t pending_write(){
    if(m_conn == nullptr) return 0;
    return m_conn->pending_write();
  }

  size_t pending_read(){
    if(m_conn == nullptr) return 0;
    return m_conn->pending_read();
  }

  size_t queue(){
    std::lock_guard<std::mutex> lock(m_mutex_queue);
    return m_queue.size();
  }

  bool due(){
    return  queue() > 0 || pending_read() > 0 || pending_write() > 0;
  }

  void close(){
    if(m_conn != nullptr)
      m_conn->close();
  }

  private:
  
  client::ClientConPtr  m_conn;
  uint32_t              m_timeout;
  
  std::mutex            m_mutex_queue;
  std::queue<Req::Ptr>  m_queue;
};
typedef std::shared_ptr<MngColumn> MngColumnPtr;


}}}

#endif // swc_lib_db_protocol_req_MngColumn_h
