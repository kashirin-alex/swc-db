/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */



#include <thrift/server/TThreadPoolServer.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TZlibTransport.h>
#include <thrift/protocol/TBinaryProtocol.h>

#include "swcdb/thrift/broker/Settings.h"
#include "swcdb/thrift/broker/AppContext.h"


namespace SWC {

int run() {
  SWC_TRY_OR_LOG("", 

  auto& props = Env::Config::settings()->properties;

  uint32_t reactors = 1; // props.get<int32_t>("swc.ThriftBroker.reactors");
  int workers = props.get<int32_t>("swc.ThriftBroker.workers");
  uint32_t timeout_ms = props.get<int16_t>("swc.ThriftBroker.timeout");
  std::string transport = props.get<std::string>("swc.ThriftBroker.transport");

	std::shared_ptr<thrift::transport::TTransportFactory> transportFactory;
	if (transport.compare("framed") == 0) {
		transportFactory.reset(new thrift::transport::TFramedTransportFactory());

	} else if (transport.compare("zlib") == 0) {
		transportFactory.reset(new thrift::transport::TZlibTransportFactory());

  } else {
		SWC_LOGF(
      LOG_FATAL, "No implementation for transport=%s", transport.c_str());
		return 1;
	}

  Strings addrs = props.has("addr") ? props.get<Strings>("addr") : Strings();
  std::string host;
  if(props.has("host"))
    host = host.append(props.get<std::string>("host"));
  else {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    host.append(hostname);
  }
    
  EndPoints endpoints = Resolver::get_endpoints(
    props.get<int16_t>("swc.ThriftBroker.port"),
    addrs,
    host,
    true
  );

  SWC_LOGF(LOG_INFO, "STARTING SERVER: THRIFT-BROKER, reactors=%d workers=%d transport=%s", 
           reactors, workers, transport.c_str());


  auto app_ctx = std::make_shared<Thrift::AppContext>();
  std::vector<std::shared_ptr<thrift::server::TThreadPoolServer>> servers;

  for(uint32_t reactor=0; reactor < reactors; ++reactor) {

	  std::shared_ptr<thrift::concurrency::ThreadManager> threadManager(
		  thrift::concurrency::ThreadManager::newSimpleThreadManager(workers));
	  threadManager->threadFactory(
      std::make_shared<thrift::concurrency::ThreadFactory>());
  	threadManager->start();

	  for(auto& endpoint : endpoints) {
      bool is_plain = true; // if use_ssl && need ssl.. transportFactory.reset(..)
      std::shared_ptr<thrift::transport::TServerSocket> socket;
      if(reactor == 0) { 
        socket = std::make_shared<thrift::transport::TServerSocket>(
          endpoint.address().to_string(), endpoint.port());
      } else {
        continue;
        //1st socket->getSocketFD dup >> init socket from fd (per reactor)
      }
      socket->setSendTimeout(timeout_ms);
      socket->setRecvTimeout(timeout_ms);

      auto protocol = std::make_shared<thrift::protocol::TBinaryProtocolFactory>();
      /* 
      protocol->setRecurisionLimit(...);  
      set the DEFAULT_RECURSION_LIMIT
        FractionCells in sql_select_fraction 
          requirment depends on the length/depth of key-fractions
      */
      auto server = std::make_shared<thrift::server::TThreadPoolServer>(
        std::make_shared<Thrift::BrokerProcessorFactory>(app_ctx),
		    socket,
	      transportFactory,
  	    protocol,
	      threadManager
      );
      servers.push_back(server);
      std::thread([server]{ server->serve(); }).detach();

      SWC_LOGF(
        LOG_INFO, "Listening On: [%s]:%d fd=%d %s", 
        endpoint.address().to_string().c_str(), endpoint.port(), 
        (ssize_t)server->getServerTransport()->getSocketFD(),
        is_plain ? "PLAIN" : "SECURE"
      );
    }
  }

  app_ctx->wait_while_run();

  for(auto& server : servers)
    server->getThreadManager()->stop();

  return 0);

  return 1;
}

} //namespace SWC


int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);
  SWC::Env::Config::settings()->init_process();
  return SWC::run();
}
