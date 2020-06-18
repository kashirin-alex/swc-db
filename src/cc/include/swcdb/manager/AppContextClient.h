/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_manager_AppContextClient_h
#define swc_manager_AppContextClient_h

namespace SWC { namespace client { namespace Mngr { 

class AppContext final : public SWC::AppContext {
  public:

  AppContext();

  virtual ~AppContext();

  void disconnected(const ConnHandlerPtr& conn);

  void handle(ConnHandlerPtr conn, const Event::Ptr& ev) override;

};

}}}

#endif // swc_manager_AppContextClient_h