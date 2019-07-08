/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_client_Clients_h
#define swc_lib_client_Clients_h


#include "swcdb/lib/core/comm/AppContext.h"
#include "swcdb/lib/client/mngr/Groups.h"
#include "swcdb/lib/core/comm/SerializedClient.h"
#include "swcdb/lib/client/rs/AppContext.h"

#include <memory>

namespace SWC { 
  
  typedef asio::executor_work_guard<asio::io_context::executor_type> IO_DoWork;
  typedef std::shared_ptr<IO_DoWork> IO_DoWorkPtr;

  namespace client { 

class Clients;
typedef std::shared_ptr<Clients> ClientsPtr;

class Clients : public std::enable_shared_from_this<Clients> {
  public:

  Clients(IOCtxPtr ioctx, AppContextPtr mngr_client_ctx)
          : mngr_ctx(mngr_client_ctx), m_run(true), 
            m_ioctx(
              ioctx == nullptr?std::make_shared<asio::io_context>(8):ioctx
            )
  {
    
    if(ioctx == nullptr){
      m_wrk = std::make_shared<IO_DoWork>(asio::make_work_guard(*m_ioctx.get()));
      (new std::thread(
        [io_ptr=m_ioctx, run=&m_run]{ 
          do{
            io_ptr->run();
            HT_DEBUG("IO stopped, restarting");
            io_ptr->restart();
          }while(run->load());
          HT_DEBUG("IO exited");
        }
      ))->detach();
    }

    mngrs_groups = std::make_shared<Mngr::Groups>()->init();

    mngr_service = std::make_shared<SerializedClient>(
      "RS-MANAGER", m_ioctx, mngr_ctx
    );

    rs_service = std::make_shared<SerializedClient>(
      "RANGESERVER", m_ioctx, std::make_shared<RS::AppContext>()
    );

  }
  operator ClientsPtr(){
    return shared_from_this();
  }

  virtual ~Clients(){}

  void stop(){
    m_run.store(false);
    if(m_wrk != nullptr)
      m_wrk->reset();
  }
  
  Mngr::GroupsPtr mngrs_groups;
  ClientPtr       mngr_service = nullptr;
  AppContextPtr   mngr_ctx = nullptr;

  ClientPtr       rs_service   = nullptr;

  IO_DoWorkPtr    m_wrk = nullptr;

  private:
  std::atomic<bool> m_run;
  IOCtxPtr          m_ioctx;
};

}}

#endif // swc_lib_client_Clients_h