/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_db_client_mngr_Groups_h
#define swcdb_db_client_mngr_Groups_h

#include "swcdb/core/MutexSptd.h"
#include "swcdb/db/Types/Identifiers.h"
#include "swcdb/db/Types/MngrRole.h"
#include "swcdb/core/comm/Resolver.h"
#include "swcdb/db/client/Settings.h"


namespace SWC { namespace client { namespace Mngr {

typedef Core::Vector<Comm::EndPoints>  Hosts;

class Group final : private Hosts {
  public:

  typedef std::shared_ptr<Group>  Ptr;
  const uint8_t role;
  const cid_t   cid_begin;
  const cid_t   cid_end;


  Group(uint8_t role, cid_t cid_begin, cid_t cid_end,
        const Comm::EndPoints& endpoints);

  Group(uint8_t role, cid_t cid_begin, cid_t cid_end,
        const Hosts& hosts);

  ~Group() { }

  Ptr copy();

  void add_host(Comm::EndPoints& new_endpoints);

  Hosts get_hosts();

  bool is_in_group(const Comm::EndPoint& endpoint) noexcept;

  void print(std::ostream& out);

  void apply_endpoints(Comm::EndPoints& to_endpoints);

  private:

  void _get_host(const Comm::EndPoint& point,
                 Comm::EndPoints*& found_host) noexcept;

  Core::MutexSptd m_mutex;
};




class Groups final : private Core::Vector<Group::Ptr>,
                     public std::enable_shared_from_this<Groups>{

  public:

  struct GroupHost final {
    uint8_t         role;
    cid_t           cid_begin;
    cid_t           cid_end;
    Comm::EndPoints endpoints;
    ~GroupHost() { }
  };
  typedef std::shared_ptr<Groups> Ptr;
  typedef Core::Vector<Group::Ptr> Vec;

  Groups(const Config::Settings& settings);

  Groups(const Groups& other, Groups::Vec&&);

  ~Groups() { }

  Ptr init();

  Ptr copy();

  void on_cfg_update();

  Vec get_groups();

  void hosts(uint8_t role, cid_t cid, Hosts& hosts, GroupHost &group_host);

  Vec get_groups(const Comm::EndPoints& endpoints);

  Comm::EndPoints get_endpoints(uint8_t role=0, cid_t cid_begin=0,
                                cid_t cid_end=0);

  void print(std::ostream& out);

  void add(GroupHost&& g_host);

  void remove(const Comm::EndPoints& endpoints);

  void select(const cid_t& cid, Comm::EndPoints& endpoints);

  void select(const uint8_t& role, Comm::EndPoints& endpoints);

  private:

  void _add_host(uint8_t role, cid_t cid_begin, cid_t cid_end,
                 uint16_t port, std::string host_or_ips);

  Config::Property::V_GSTRINGS::Ptr cfg_hosts;
  const uint16_t                    cfg_port;

  Core::MutexSptd                   m_mutex;
  Core::Vector<GroupHost>           m_active_g_host;
  Comm::Networks                    m_nets;

};

}}}

#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/client/service/mngr/Groups.cc"
#endif


#endif // swcdb_db_client_mngr_Groups_h
