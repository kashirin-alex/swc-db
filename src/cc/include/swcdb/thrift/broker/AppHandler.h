/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_app_thriftbroker_AppHandler_h
#define swc_app_thriftbroker_AppHandler_h

namespace SWC { 
namespace thrift = apache::thrift;
namespace Thrift {


class AppHandler : virtual public BrokerIf {
  public:

  virtual ~AppHandler() { }

  void select_sql(Result& _return, const std::string& sql) {
    dummy++;
    _return = std::to_string(dummy);

    SWC_LOGF(LOG_INFO, " AppHandler::query c=%d %s", dummy, sql.c_str());
  }

  int dummy = 0;


};




}}

#endif // swc_app_thriftbroker_AppHandler_h