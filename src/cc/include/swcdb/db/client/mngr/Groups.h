/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_client_mngr_Groups_h
#define swc_client_mngr_Groups_h

#include <memory>
#include "swcdb/core/comm/Resolver.h"


namespace SWC { namespace client { namespace Mngr {

typedef std::vector<EndPoints>  Hosts;

class Group final {
  public:

  typedef std::shared_ptr<Group>  Ptr;
  size_t col_begin;
  size_t col_end;

  Group(size_t cbegin, size_t cend, const EndPoints& endpoints);

  Group(size_t cbegin, size_t cend, Hosts hosts);
  
  ~Group();
  
  Ptr copy();

  void add_host(EndPoints& new_endpoints);

  Hosts get_hosts();

  bool is_in_group(const EndPoint& endpoint);

  std::string to_string();

  void apply_endpoints(EndPoints& to_endpoints);

  private:

  void get_host(const EndPoint& point, EndPoints*& found_host);

  Hosts         m_hosts;
  std::mutex    m_mutex;
};




class Groups : public std::enable_shared_from_this<Groups>{

  public:
  
  struct GroupHost final {
    int64_t   col_begin;
    int64_t   col_end;
    EndPoints endpoints;
  };
  typedef std::shared_ptr<Groups> Ptr;
  typedef std::vector<Group::Ptr> Selected;

  Groups();

  Groups(Selected groups);

  virtual ~Groups();

  Ptr init();

  operator Ptr();

  Ptr copy();

  void on_cfg_update();

  void add_host(size_t col_begin, size_t col_end, 
                uint16_t port, std::string host_or_ips);

  Selected get_groups();

  void hosts(size_t cid, Hosts& hosts, GroupHost &group_host);

  Selected get_groups(const EndPoints& endpoints);

  EndPoints get_endpoints(size_t col_begin=0, size_t col_end=0);

  std::string to_string();

  void add(GroupHost& g_host);

  void remove(EndPoints& endpoints);

  void select(int64_t cid, EndPoints& endpoints);

  private:
  std::mutex              m_mutex;
  Selected                m_groups;
  std::vector<GroupHost>  m_active_g_host;
};

}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/mngr/Groups.cc"
#endif 


#endif // swc_client_mngr_Groups_h