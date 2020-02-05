/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_thrift_client_Client_h
#define swc_app_thrift_client_Client_h

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/transport/TZlibTransport.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "swcdb/thrift/gen-cpp/Service.h"

namespace SWC { 
namespace thrift = apache::thrift;
namespace Thrift {

class CommClient {
  public:

  CommClient(const std::string& host, const uint16_t port, 
             const uint32_t timeout, bool open=true) {
    auto sock = new thrift::transport::TSocket(host, port);
  	sock->setConnTimeout(timeout);
	  sock->setSendTimeout(timeout);
		sock->setRecvTimeout(timeout);
    socket.reset(sock);
    transport.reset(new thrift::transport::TFramedTransport(socket));
    if(open)
      transport->open();
    protocol.reset(new thrift::protocol::TBinaryProtocol(transport));
  }
  
  void open() {
    if(!transport->isOpen())
      transport->open();
  }

  void close() {
    if(transport->isOpen())
      transport->close();
  }

  ~CommClient() { 
    if(transport == nullptr)
      return;
    try { close(); } catch(...) {}
  }

  protected:
	std::shared_ptr<thrift::transport::TTransport>  socket;
  std::shared_ptr<thrift::transport::TTransport>  transport;
	std::shared_ptr<thrift::protocol::TProtocol>    protocol;
};


class Client : public CommClient, public ServiceClient {
  public:
  typedef std::shared_ptr<Client> Ptr;

  static Ptr make(const std::string& host, const uint16_t port, 
                  const uint32_t timeout_ms=900000) {
    return std::make_shared<Client>(host, port, timeout_ms);
  }

  Client(const std::string& host, const uint16_t port, 
         const uint32_t timeout_ms=900000, bool open = true)
        : CommClient(host, port, timeout_ms, open), 
          ServiceClient(protocol) {
  }

  virtual ~Client() { }

};

}} // namespace SWC::Thrift



#ifdef SWC_IMPL_SOURCE
#include "swcdb/thrift/gen-cpp/Service.cpp"
#include "swcdb/thrift/gen-cpp/Service_constants.cpp"
#include "swcdb/thrift/gen-cpp/Service_types.cpp"
//#include "swcdb/thrift/Converters.cc"
#endif 


#endif // swc_app_thrift_client_Client_h