/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_fs_Broker_FileSystem_h
#define swc_lib_fs_Broker_FileSystem_h

#include "swcdb/lib/fs/FileSystem.h"
#include "swcdb/lib/client/AppContext.h"
#include "swcdb/lib/core/comm/SerializedClient.h"

#include "Protocol/Commands.h"
#include "Protocol/req/Exists.h"

namespace SWC{ namespace FS {

bool apply_broker();


class FileSystemBroker: public FileSystem {
  public:
  
  static const EndPoints get_endpoints(){
    std::string host = EnvConfig::settings()->get<String>("swc.fs.broker.host", "");
    if(host.empty()){
      char hostname[256];
      gethostname(hostname, sizeof(hostname));
      host.append(hostname);
    }
    Strings addr;
    return Resolver::get_endpoints(
      EnvConfig::settings()->get<int32_t>("swc.fs.broker.port"),
      addr, host, true
    );
  }

  FileSystemBroker()
    : FileSystem(apply_broker()),
      m_io(std::make_shared<IoContext>(
        EnvConfig::settings()->get<int32_t>("swc.fs.broker.handlers"))),
      m_service(std::make_shared<client::SerializedClient>(
        "FS-BROKER", m_io->shared(), std::make_shared<client::AppContext>())),
      m_type_underlying(parse_fs_type(
        EnvConfig::settings()->get<String>("swc.fs.broker.underlying"))),
      m_endpoints(get_endpoints())
  { }

  virtual ~FileSystemBroker(){}

  void stop() override {
    m_io->stop();
  }

  Types::Fs get_type() override {
    return m_type_underlying;
  }

  const std::string to_string() override {
    return format("(type=BROKER, underlying-type=%s)", 
                    type_to_string(m_type_underlying).c_str());
  }


  bool send_request(Protocol::Req::ReqBasePtr hdlr){
    client::ClientConPtr c = m_service->get_connection(
      m_endpoints, std::chrono::milliseconds(20000), 0);
      
    if(c->send_request(hdlr->cbp, hdlr) != Error::OK) 
      return false;
    m_service->preserve(c);
    return true;
  }

  ///

  bool exists(int &err, const String &name) override {
    Protocol::Req::ExistsPtr hdlr 
      = std::make_shared<Protocol::Req::Exists>(name);

    std::promise<void> res = hdlr->promise();
    while(!send_request(hdlr));

    res.get_future().wait();
    err = hdlr->error;
    return hdlr->state;
  }

  void exists(Callback::ExistsCb_t cb, const String &name) override {
    Protocol::Req::ExistsPtr hdlr 
      = std::make_shared<Protocol::Req::Exists>(name, cb);
      
    while(!send_request(hdlr));
  }

  /// 
  
  void mkdirs(int &err, const String &name) override {
    HT_DEBUGF("mkdirs path='%s'", name.c_str());
    
  }

  void readdir(int &err, const String &name, DirentList &results) override {
    HT_DEBUGF("Readdir dir='%s'", name.c_str());
  }

  void remove(int &err, const String &name) override {
    HT_DEBUGF("remove('%s')", name.c_str());
  }

  void create(int &err, SmartFdPtr &smartfd, 
              int32_t bufsz, int32_t replication, int64_t blksz) override {

    std::string abspath = smartfd->filepath();
    HT_DEBUGF("create %s bufsz=%d replication=%d blksz=%lld",
              smartfd->to_string().c_str(), 
              bufsz, (int)replication, (Lld)blksz);
  }

  void open(int &err, SmartFdPtr &smartfd, int32_t bufsz=0) override {

    std::string abspath = smartfd->filepath();
    HT_DEBUGF("open %s, bufsz=%d", smartfd->to_string().c_str(), bufsz);
  }
  
  size_t read(int &err, SmartFdPtr &smartfd, 
              void *dst, size_t amount) override {
    
    HT_DEBUGF("read %s amount=%d", smartfd->to_string().c_str(), amount);
    ssize_t nread = 0;
    return nread;
  }

  size_t append(int &err, SmartFdPtr &smartfd, 
                StaticBuffer &buffer, Flags flags) override {
    
    HT_DEBUGF("append %s amount=%d flags=%d", 
              smartfd->to_string().c_str(), buffer.size, flags);
    
    ssize_t nwritten = 0;
    return nwritten;
  }



  void close(int &err, SmartFdPtr &smartfd) {
    HT_DEBUGF("close %s", smartfd->to_string().c_str());
  }



  private:

  IoContextPtr      m_io;
  client::ClientPtr m_service = nullptr;
  Types::Fs         m_type_underlying;
  const EndPoints   m_endpoints;

};


}}



#endif  // swc_lib_fs_Broker_FileSystem_h