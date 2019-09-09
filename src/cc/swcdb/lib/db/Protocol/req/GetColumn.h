
/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_db_protocol_req_GetColumn_h
#define swc_lib_db_protocol_req_GetColumn_h


#include "swcdb/lib/db/Protocol/Commands.h"

#include "ActiveMngrBase.h"
#include "CodeHandler.h"
#include "../params/GetColumn.h"


namespace SWC {
namespace Protocol {
namespace Req {


class GetColumn: public ActiveMngrBase {
  public:

  using Flag = Params::GetColumnReq::Flag;

  struct Req : public DispatchHandler {
    public:

    typedef std::shared_ptr<Req> Ptr;
    typedef std::function<void(Ptr, int, Params::GetColumnRsp)> Cb_t;

    Req(Protocol::Params::GetColumnReq params, Cb_t cb)
        : params(params), cb(cb) {}

    virtual ~Req(){}

    void handle(ConnHandlerPtr conn, EventPtr &ev) override {
      if(was_called)
        return;

      if(ev->type == Event::Type::DISCONNECT){
        //if(!was_called)
        // put(cbp);
        return;
      }

      was_called = true;

      Protocol::Params::GetColumnRsp rsp_params;
      int err = ev->error != Error::OK? ev->error: Protocol::response_code(ev);

      if(err == Error::OK){
        try{
          const uint8_t *ptr = ev->payload+4;
          size_t remain = ev->payload_len-4;
          rsp_params.decode(&ptr, &remain);
        } catch (Exception &e) {
          HT_ERROR_OUT << e << HT_END;
          err = e.code();
        }
      }

      cb(
        std::dynamic_pointer_cast<Req>(shared_from_this()), 
        err,  
        rsp_params
      );

    }

    std::atomic<bool>     was_called = false;

    
    Protocol::Params::GetColumnReq params;
    Cb_t        cb;
  };


  GetColumn(uint32_t timeout=60000)
            : ActiveMngrBase(1, 1), 
              m_conn(nullptr), m_timeout(timeout) { }

  virtual ~GetColumn(){}

  void get_scheme_by_name(std::string& name, Req::Cb_t cb) {
    request(Flag::SCHEMA_BY_NAME, name, cb);
  }
  
  void get_scheme_by_id(int64_t cid, Req::Cb_t cb) {
    request(Flag::SCHEMA_BY_ID, cid, cb);
  }

  void get_id_by_name(std::string& name, Req::Cb_t cb) {
    request(Flag::ID_BY_NAME, name, cb);
  }

  void request(Flag flag, std::string& name, Req::Cb_t cb){
    return make(std::make_shared<Req>(Protocol::Params::GetColumnReq(flag, name), cb));
  }
  void request(Flag flag, int64_t cid, Req::Cb_t cb){
    return make(std::make_shared<Req>(Protocol::Params::GetColumnReq(flag, cid), cb));
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
        std::dynamic_pointer_cast<GetColumn>(ptr)->make_requests(conn, true);
      },
      std::chrono::milliseconds(timeout_ms), 
      1,
      true
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

      CommHeader header(Protocol::Command::CLIENT_REQ_GET_COLUMN, sz*m_timeout);
      CommBufPtr cbp = std::make_shared<CommBuf>(header, req->params.encoded_length());
      req->params.encode(cbp->get_data_ptr_address());
      
      if(m_conn->send_request(cbp, req) == Error::OK){

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
typedef std::shared_ptr<GetColumn> GetColumnPtr;


}}}

#endif // swc_lib_db_protocol_req_GetColumn_h
