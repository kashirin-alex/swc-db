/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/thrift/broker/Settings.h"
#include "swcdb/thrift/broker/AppContext.h"

#include <thrift/server/TThreadPoolServer.h>
#include <thrift/concurrency/ThreadManager.h>
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TZlibTransport.h>
#include <thrift/protocol/TBinaryProtocol.h>


namespace SWC {


/**
 * @brief The SWC-DB Application ThriftBroker C++ namespace 'SWC::ThriftBroker'
 *
 * \ingroup Applications
 */
namespace ThriftBroker {


int run() {
  SWC_TRY_OR_LOG("",

  auto settings = Env::Config::settings();

  uint32_t reactors = 1; // settings->get_i32("swc.ThriftBroker.reactors");
  int workers = settings->get_i32("swc.ThriftBroker.workers");
  uint64_t conns_max = settings->get_i64("swc.ThriftBroker.connections.max");
  uint32_t timeout_ms = settings->get_i32("swc.ThriftBroker.timeout");
  std::string transport = settings->get_str("swc.ThriftBroker.transport");

  std::shared_ptr<thrift::transport::TTransportFactory> transportFactory;

  if(!strncasecmp(transport.data(), "framed", 6)) {
    transportFactory.reset(new thrift::transport::TFramedTransportFactory());

  } else if(!strncasecmp(transport.data(), "zlib", 4)) {
    transportFactory.reset(new thrift::transport::TZlibTransportFactory());

  } else {
    SWC_LOGF(
      LOG_FATAL, "No implementation for transport=%s", transport.c_str());
    return 1;
  }

  Config::Strings addrs;
  if(settings->has("addr"))
    addrs = settings->get_strs("addr");

  std::string host;
  if(settings->has("host"))
    host = host.append(settings->get_str("host"));
  else {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    host.append(hostname);
  }

  Comm::EndPoints endpoints = Comm::Resolver::get_endpoints(
    settings->get_i16("swc.ThriftBroker.port"),
    addrs,
    host,
    {},
    true
  );

  SWC_LOGF(LOG_INFO,
    "STARTING SERVER: THRIFT-BROKER, reactors=%u workers=%d transport=%s",
    reactors, workers, transport.c_str());


  auto app_ctx = std::make_shared<AppContext>();
  std::vector<std::unique_ptr<std::thread>> threads;
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
      if(!reactor) {
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
        FCells in sql_select_rslt_on_fraction
          requirment depends on the length/depth of key-fractions
      */
      auto server = std::make_shared<thrift::server::TThreadPoolServer>(
        std::make_shared<Thrift::BrokerProcessorFactory>(app_ctx),
        socket,
        transportFactory,
        protocol,
        threadManager
      );
      server->setConcurrentClientLimit(conns_max);

      servers.push_back(server);
      threads.emplace_back(
        new std::thread([app_ctx, is_plain, endpoint, server] {
          SWC_LOG_OUT(LOG_INFO, SWC_LOG_OSTREAM
            << "Listening On: " << endpoint
            << " fd=" << server->getServerTransport()->getSocketFD()
            << ' ' << (is_plain ? "PLAIN" : "SECURE");
          );

          server->serve();

          SWC_LOG_OUT(LOG_INFO, SWC_LOG_OSTREAM
            << "Stopping to Listen On: " << endpoint
            << " fd=" << server->getServerTransport()->getSocketFD()
            << ' ' << (is_plain ? "PLAIN" : "SECURE");
          );
          app_ctx->shutting_down(std::error_code(), SIGINT);
        })
      );
    }
  }

  app_ctx->wait_while_run();

  for(auto& server : servers) {
    server->stop();
    server->getThreadManager()->stop();
  }
  servers.clear();

  for(auto& th : threads)
    th->join();
  threads.clear();

  SWC_LOG(LOG_INFO, "Exit");
  std::quick_exit(EXIT_SUCCESS);


  return 0);

  return 1;
}


}} //namespace SWC::ThriftBroker



int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv);
  SWC::Env::Config::settings()->init_process(true, "swc.ThriftBroker.port");
  return SWC::ThriftBroker::run();
}
