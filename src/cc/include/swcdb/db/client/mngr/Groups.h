/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_db_client_mngr_Groups_h
#define swc_db_client_mngr_Groups_h

#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/Types/MngrRole.h"
#include "swcdb/core/comm/Resolver.h"


namespace SWC { namespace client { namespace Mngr {

typedef std::vector<EndPoints>  Hosts;

class Group final : private Hosts {
  public:

  typedef std::shared_ptr<Group>  Ptr;
  const uint8_t role;
  const cid_t   cid_begin;
  const cid_t   cid_end;


  Group(uint8_t role, cid_t cid_begin, cid_t cid_end, 
        const EndPoints& endpoints);

  Group(uint8_t role, cid_t cid_begin, cid_t cid_end, 
        const Hosts& hosts);
  
  ~Group();
  
  Ptr copy();

  void add_host(EndPoints& new_endpoints);

  Hosts get_hosts();

  bool is_in_group(const EndPoint& endpoint);

  std::string to_string();

  void apply_endpoints(EndPoints& to_endpoints);

  private:

  void _get_host(const EndPoint& point, EndPoints*& found_host);

  std::mutex    m_mutex;
};




class Groups final : private std::vector<Group::Ptr>,
                     public std::enable_shared_from_this<Groups>{

  public:
  
  struct GroupHost final {
    uint8_t role;
    cid_t   cid_begin;
    cid_t   cid_end;
    EndPoints endpoints;
  };
  typedef std::shared_ptr<Groups> Ptr;
  typedef std::vector<Group::Ptr> Vec;

  Groups();

  Groups(const Vec& groups);

  ~Groups();

  Ptr init();

  Ptr copy();

  void on_cfg_update();

  Vec get_groups();

  void hosts(uint8_t role, cid_t cid, Hosts& hosts, GroupHost &group_host);

  Vec get_groups(const EndPoints& endpoints);

  EndPoints get_endpoints(uint8_t role=0, cid_t cid_begin=0, 
                                          cid_t cid_end=0);

  std::string to_string();

  void add(GroupHost& g_host);

  void remove(const EndPoints& endpoints);

  void select(uint8_t role, cid_t cid, EndPoints& endpoints);

  private:

  void _add_host(uint8_t role, cid_t cid_begin, cid_t cid_end, 
                 uint16_t port, std::string host_or_ips);

  std::mutex              m_mutex;
  std::vector<GroupHost>  m_active_g_host;
};

}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/mngr/Groups.cc"
#endif 


#endif // swc_db_client_mngr_Groups_h