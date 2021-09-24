/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/utils/cli/Shell_Manager.h"
#include "swcdb/db/Protocol/Mngr/req/Report.h"
#include "swcdb/db/client/sql/Reader.h"
#include "swcdb/core/StateSynchronization.h"


namespace SWC { namespace Utils { namespace shell {


Mngr::Mngr()
  : Interface("\033[32mSWC-DB(\033[36mmngr\033[32m)\033[33m> \033[00m",
              "/tmp/.swc-cli-manager-history"),
    clients(client::Clients::make(
      *Env::Config::settings(),
      Comm::IoContext::make("Mngr", 8),
      nullptr, // std::make_shared<client::ManagerContext>()
      nullptr  // std::make_shared<client::RangerContext>()
      )->init()
    ) {

  add_option(
    "cluster",
    {"report cluster-status state by error",
    "cluster;"},
    [ptr=this](std::string& cmd){return ptr->cluster_status(cmd);},
    "(?i)^(cluster)(\\s|$)"
  );
  add_option(
    "status",
    {"report managers status by mngr-endpoint, of Schema-Role or by All",
    "status [Schemas(default) | ALL | endpoint=HOST/IP(|PORT)];"},
    [ptr=this](std::string& cmd){return ptr->managers_status(cmd);},
    "(?i)^(report|status)(\\s|$)"
  );
  add_option(
    "column-status",
    {"report column status with ranges status",
    "column-status [cid=CID | name=NAME];"},
    [ptr=this](std::string& cmd){return ptr->column_status(cmd);},
    "(?i)^(report-column|column-status)"
  );
  add_option(
    "rangers-status",
    {"report rangers status by Manager of column or Rangers-Role",
    "rangers-status [without | cid=CID | name=NAME];"},
    [ptr=this](std::string& cmd){return ptr->rangers_status(cmd);},
    "(?i)^(report-rangers|rangers-status)"
  );
}

Mngr::~Mngr() {
  clients->stop();
}

bool Mngr::cluster_status(std::string&) {
  std::string message;
  client::Mngr::Hosts hosts;
  auto groups = clients->managers.groups->get_groups();
  for(auto& g : groups) {
    auto tmp =  g->get_hosts();
    hosts.insert(hosts.cend(), tmp.cbegin(), tmp.cend());
  }
  if(hosts.empty()) {
    err = Error::FAILED_EXPECTATION;
    message.append("Empty swc.mngr.host config\n");
    return error(message);
  }

  Core::StateSynchronization res;
  for(auto& endpoints : hosts) {
    Comm::Protocol::Mngr::Req::ClusterStatus::request(
      clients,
      endpoints,
      [this, await=&res]
      (const Comm::client::ConnQueue::ReqBase::Ptr&,
       const int& error) noexcept {
        err = error;
        await->acknowledge();
      }
    );
    res.wait();
    if(err) {
      SWC_PRINT << "code=" << err
                << " msg=" << Error::get_text(err) << SWC_PRINT_CLOSE;
      message.append(Error::get_text(err));
      message.append("\n");
      return error(message);
    }
    res.reset();
  }
  SWC_PRINT << "code=" << err
            << " msg=" << Error::get_text(err) << SWC_PRINT_CLOSE;
  return true;
}

bool Mngr::managers_status(std::string& cmd) {
  std::string message;
  size_t at = cmd.find_first_of(" ");
  cmd = cmd.substr(at+1);
  client::SQL::Reader reader(cmd, message);
  while(reader.found_space());

  client::Mngr::Hosts hosts;
  if(reader.found_token("endpoint", 8)) {
    while(reader.found_space());
    reader.expect_eq();
    if(reader.err)
      return error(message);

    std::string host_or_ips;
    reader.read(host_or_ips, " |");
    if(reader.err)
      return error(message);
    if(host_or_ips.empty())
      return error("Missing Endpoint");

    while(reader.found_space());
    uint16_t port;
    bool was_set = false;
    if(*(reader.ptr-1) == '|' || reader.found_token("|", 1)) {
      reader.read_uint16_t(port, was_set);
      if(reader.err)
        return error(message);
    }
    if(!was_set)
      port = Env::Config::settings()->get_i16("swc.mngr.port");

    Config::Strings ips;
    std::string host;
    if(Comm::Resolver::is_ipv4_address(host_or_ips) ||
       Comm::Resolver::is_ipv6_address(host_or_ips))
      ips.push_back(host_or_ips);
    else
      host = host_or_ips;

    hosts.emplace_back(
      Comm::Resolver::get_endpoints(
        port, ips, host, {}, false
      )
    );
    if(hosts.back().empty())
      return error("Missing Endpoint");

  } else if(reader.found_token("all", 3)) {
    auto groups = clients->managers.groups->get_groups();
    for(auto& g : groups) {
      auto tmp =  g->get_hosts();
      hosts.insert(hosts.cend(), tmp.cbegin(), tmp.cend());
    }

  } else { // reader.found_token("schemas", 7)
    hosts.emplace_back();
  }

  Core::StateSynchronization res;
  for(auto& endpoints : hosts) {
    Comm::Protocol::Mngr::Req::ManagersStatus::request(
      clients,
      endpoints,
      [endpoints, await=&res]
      (const Comm::client::ConnQueue::ReqBase::Ptr& req, const int& error,
       const Comm::Protocol::Mngr::Params::Report::RspManagersStatus& rsp) {
        SWC_PRINT << "# by Manager(";
        if(error) {
          for(auto& p : endpoints)
            SWC_LOG_OSTREAM << p << ',';
          SWC_LOG_OSTREAM << ") - \033[31mERROR\033[00m: " << error
                          << "(" << Error::get_text(error) << ")";
        } else {
          SWC_LOG_OSTREAM << (req->queue? req->queue->get_endpoint_remote()
                                        : Comm::EndPoint()) << ") ";
          rsp.display(SWC_LOG_OSTREAM);
        }
        SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
        await->acknowledge();
      }
    );
    res.wait();
    res.reset();
  }
  return true;
}

bool Mngr::column_status(std::string& cmd) {
  std::string message;

  size_t at = cmd.find_first_of(" ");
  cmd = cmd.substr(at+1);
  cid_t cid = DB::Schema::NO_CID;
  client::SQL::Reader reader(cmd, message);
  while(reader.found_space());

  if(reader.found_token("cid", 3)) {
    reader.expect_eq();
    if(!reader.err) {
      bool was_set = false;
      reader.read_uint64_t(cid, was_set);
    }
    if(reader.err)
      return error(message);

  } else if(reader.found_token("name", 4)) {
    reader.expect_eq();
    if(reader.err)
      return error(message);

    std::string col_name;
    reader.read(col_name, " ");
    if(col_name.empty())
      return error("Missing Column Name");

    auto schema = reader.get_schema(clients, col_name);
    if(reader.err)
      return error(message);
    cid = schema->cid;
  } else {
    bool token = false;
    reader.expect_token("name", 4, token);
    return error(message);
  }

  Core::StateSynchronization res;
  Comm::Protocol::Mngr::Req::ColumnStatus::request(
    clients,
    Comm::Protocol::Mngr::Params::Report::ReqColumnStatus(cid),
    [this, await=&res]
    (const Comm::client::ConnQueue::ReqBase::Ptr&, const int& error,
     const Comm::Protocol::Mngr::Params::Report::RspColumnStatus& rsp) {
      if(!(err = error)) {
        SWC_PRINT << "";
        rsp.display(SWC_LOG_OSTREAM);
        SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
      }
      await->acknowledge();
    }
  );
  res.wait();

  if(err) {
    message.append(Error::get_text(err));
    message.append("\n");
    return error(message);
  }
  return true;
}

bool Mngr::rangers_status(std::string& cmd) {
  std::string message;

  size_t at = cmd.find_first_of(" ");
  cmd = cmd.substr(at+1);
  cid_t cid = DB::Schema::NO_CID;
  client::SQL::Reader reader(cmd, message);
  while(reader.found_space());

  if(reader.found_token("cid", 3)) {
    reader.expect_eq();
    if(!reader.err) {
      bool was_set = false;
      reader.read_uint64_t(cid, was_set);
    }
    if(reader.err)
      return error(message);

  } else if(reader.found_token("name", 4)) {
    reader.expect_eq();
    if(reader.err)
      return error(message);

    std::string col_name;
    reader.read(col_name, " ");
    if(col_name.empty())
      return error("Missing Column Name");

    auto schema = reader.get_schema(clients, col_name);
    if(reader.err)
      return error(message);
    cid = schema->cid;
  }

  Core::StateSynchronization res;
  Comm::Protocol::Mngr::Req::RangersStatus::request(
    clients,
    cid,
    [this, await=&res]
    (const Comm::client::ConnQueue::ReqBase::Ptr&, const int& error,
     const Comm::Protocol::Mngr::Params::Report::RspRangersStatus& rsp) {
      if(!(err = error)) {
        SWC_PRINT << "";
        rsp.display(SWC_LOG_OSTREAM);
        SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
      }
      await->acknowledge();
    }
  );
  res.wait();

  if(err) {
    message.append(Error::get_text(err));
    message.append("\n");
    return error(message);
  }
  return true;
}


}}} // namespace Utils::shell
