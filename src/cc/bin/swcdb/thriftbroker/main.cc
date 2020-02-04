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

#include "swcdb/thriftbroker/Settings.h"
#include "swcdb/thriftbroker/AppContext.h"


namespace SWC {

int run() {
  SWC_TRY_OR_LOG("", 

  auto& props = Env::Config::settings()->properties;

  int workers = props.get<int32_t>("swc.ThriftBroker.workers");
  uint16_t port = props.get<int16_t>("swc.ThriftBroker.port");
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

	

	std::shared_ptr<thrift::concurrency::ThreadManager> threadManager(
		thrift::concurrency::ThreadManager::newSimpleThreadManager(workers));
	threadManager->threadFactory(
    std::make_shared<thrift::concurrency::ThreadFactory>());
	threadManager->start();
    
  auto app_ctx = std::make_shared<Thrift::AppContext>();

	thrift::server::TThreadPoolServer server(
    std::make_shared<Thrift::BrokerProcessorFactory>(app_ctx),
		std::make_shared<thrift::transport::TServerSocket>(
      port, timeout_ms, timeout_ms
    ),
		transportFactory,
		std::make_shared<thrift::protocol::TBinaryProtocolFactory>(),
	  threadManager
  );
  
  SWC_LOGF(LOG_INFO, "STARTING SERVER: THRIFT-BROKER, workers=%d transport=%s", 
           workers, transport.c_str());

	server.serve();
  

  return 0);

  return 1;
}

} //namespace SWC


int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);
  SWC::Env::Config::settings()->init_process();
  return SWC::run();
}
