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
#include "Protocol/req/Remove.h"
#include "Protocol/req/Length.h"
#include "Protocol/req/Mkdirs.h"
#include "Protocol/req/Readdir.h"
#include "Protocol/req/Rmdir.h"
#include "Protocol/req/Create.h"
#include "Protocol/req/Append.h"
#include "Protocol/req/Open.h"
#include "Protocol/req/Read.h"
#include "Protocol/req/Close.h"

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

  /// File/Dir name actions

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

  void remove(int &err, const String &name) override {
    Protocol::Req::RemovePtr hdlr 
      = std::make_shared<Protocol::Req::Remove>(name);

    std::promise<void> res = hdlr->promise();
    while(!send_request(hdlr));

    res.get_future().wait();
    err = hdlr->error;
  }

  void remove(Callback::RemoveCb_t cb, const String &name) override {
    Protocol::Req::RemovePtr hdlr 
      = std::make_shared<Protocol::Req::Remove>(name, cb);
      
    while(!send_request(hdlr));
  }
  
  size_t length(int &err, const String &name) override {
    Protocol::Req::LengthPtr hdlr 
      = std::make_shared<Protocol::Req::Length>(name);

    std::promise<void> res = hdlr->promise();
    while(!send_request(hdlr));

    res.get_future().wait();
    err = hdlr->error;
    return hdlr->length;
  }

  void length(Callback::LengthCb_t cb, const String &name) override {
    Protocol::Req::LengthPtr hdlr 
      = std::make_shared<Protocol::Req::Length>(name, cb);
      
    while(!send_request(hdlr));
  }

  void mkdirs(int &err, const String &name) override {
    Protocol::Req::MkdirsPtr hdlr 
      = std::make_shared<Protocol::Req::Mkdirs>(name);

    std::promise<void> res = hdlr->promise();
    while(!send_request(hdlr));

    res.get_future().wait();
    err = hdlr->error;
  }

  void mkdirs(Callback::MkdirsCb_t cb, const String &name) override {
    Protocol::Req::MkdirsPtr hdlr 
      = std::make_shared<Protocol::Req::Mkdirs>(name, cb);
      
    while(!send_request(hdlr));
  }

  void readdir(int &err, const String &name, DirentList &results) override {
    Protocol::Req::ReaddirPtr hdlr 
      = std::make_shared<Protocol::Req::Readdir>(name);

    std::promise<void> res = hdlr->promise();
    while(!send_request(hdlr));

    res.get_future().wait();
    err = hdlr->error;
    results = hdlr->listing;
  }

  void readdir(Callback::ReaddirCb_t cb, const String &name) override {
    Protocol::Req::ReaddirPtr hdlr 
      = std::make_shared<Protocol::Req::Readdir>(name, cb);
      
    while(!send_request(hdlr));
  }

  void rmdir(int &err, const String &name) override {
    Protocol::Req::RmdirPtr hdlr 
      = std::make_shared<Protocol::Req::Rmdir>(name);

    std::promise<void> res = hdlr->promise();
    while(!send_request(hdlr));

    res.get_future().wait();
    err = hdlr->error;
  }

  void rmdir(Callback::RmdirCb_t cb, const String &name) override {
    Protocol::Req::RmdirPtr hdlr 
      = std::make_shared<Protocol::Req::Rmdir>(name, cb);
      
    while(!send_request(hdlr));
  }

  /// SmartFd actions

  void create(int &err, SmartFdPtr &smartfd,
              int32_t bufsz, int32_t replication, int64_t blksz) override {
    Protocol::Req::CreatePtr hdlr 
      = std::make_shared<Protocol::Req::Create>(
                                    smartfd, bufsz, replication, blksz);

    std::promise<void> res = hdlr->promise();
    while(!send_request(hdlr));

    res.get_future().wait();
    err = hdlr->error;
  }

  void create(Callback::CreateCb_t cb, SmartFdPtr &smartfd,
              int32_t bufsz, int32_t replication, int64_t blksz) override {
    Protocol::Req::CreatePtr hdlr 
      = std::make_shared<Protocol::Req::Create>(
                                    smartfd, bufsz, replication, blksz, cb);
      
    while(!send_request(hdlr));
  }
  
  size_t append(int &err, SmartFdPtr &smartfd, 
                StaticBuffer &buffer, Flags flags) override {
    Protocol::Req::AppendPtr hdlr 
      = std::make_shared<Protocol::Req::Append>(smartfd, buffer, flags);

    std::promise<void> res = hdlr->promise();
    while(!send_request(hdlr));

    res.get_future().wait();
    err = hdlr->error;
    return hdlr->amount;
  }
   
  void append(Callback::AppendCb_t cb, SmartFdPtr &smartfd, 
              StaticBuffer &buffer, Flags flags) override {
    Protocol::Req::AppendPtr hdlr 
      = std::make_shared<Protocol::Req::Append>(smartfd, buffer, flags, cb);
      
    while(!send_request(hdlr));
  }

  void open(int &err, SmartFdPtr &smartfd, int32_t bufsz) override {
    Protocol::Req::OpenPtr hdlr 
      = std::make_shared<Protocol::Req::Open>(smartfd, bufsz);

    std::promise<void> res = hdlr->promise();
    while(!send_request(hdlr));

    res.get_future().wait();
    err = hdlr->error;
  }

  void open(Callback::OpenCb_t cb, SmartFdPtr &smartfd, 
            int32_t bufsz) override {
    Protocol::Req::OpenPtr hdlr 
      = std::make_shared<Protocol::Req::Open>(smartfd, bufsz, cb);
      
    while(!send_request(hdlr));
  }
  
  size_t read(int &err, SmartFdPtr &smartfd, 
              void *dst, size_t amount) override {
    Protocol::Req::ReadPtr hdlr 
      = std::make_shared<Protocol::Req::Read>(smartfd, dst, amount);

    std::promise<void> res = hdlr->promise();
    while(!send_request(hdlr));

    res.get_future().wait();
    err = hdlr->error;
    return hdlr->amount;
  }
   
  void read(Callback::ReadCb_t cb, SmartFdPtr &smartfd, 
            size_t amount) override {
    Protocol::Req::ReadPtr hdlr 
      = std::make_shared<Protocol::Req::Read>(smartfd, nullptr, amount, cb);
      
    while(!send_request(hdlr));
  }


  void close(int &err, SmartFdPtr &smartfd) override {
    Protocol::Req::ClosePtr hdlr 
      = std::make_shared<Protocol::Req::Close>(smartfd);

    std::promise<void> res = hdlr->promise();
    while(!send_request(hdlr));

    res.get_future().wait();
    err = hdlr->error;
  }

  void close(Callback::CreateCb_t cb, SmartFdPtr &smartfd) override {
    Protocol::Req::ClosePtr hdlr 
      = std::make_shared<Protocol::Req::Close>(smartfd, cb);
      
    while(!send_request(hdlr));
  }

  //



  private:

  IoContextPtr      m_io;
  client::ClientPtr m_service = nullptr;
  Types::Fs         m_type_underlying;
  const EndPoints   m_endpoints;

};


}}



#endif  // swc_lib_fs_Broker_FileSystem_h