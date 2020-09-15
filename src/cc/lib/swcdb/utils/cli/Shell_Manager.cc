/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */
 

#include "swcdb/utils/cli/Shell_Manager.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/AppContext.h"
#include "swcdb/db/Protocol/Mngr/req/Report.h"
#include "swcdb/db/client/sql/Reader.h"


namespace SWC { namespace Utils { namespace shell {


Mngr::Mngr() 
  : Interface("\033[32mSWC-DB(\033[36mmngr\033[32m)\033[33m> \033[00m",
              "/tmp/.swc-cli-manager-history") {

  options.push_back(
    new Option(
      "cluster", 
      {"report cluster-status state by error",
      "cluster;"},
      [ptr=this](std::string& cmd){return ptr->cluster_status(cmd);}, 
      new re2::RE2("(?i)^(cluster)(\\s|$)")
    )
  );

  options.push_back(
    new Option(
      "status", 
      {"report managers status by mngr-endpoint, of Schema-Role or by All",
      "status [Schemas(default) | ALL | endpoint=HOST/IP(|PORT)];"},
      [ptr=this](std::string& cmd){return ptr->managers_status(cmd);}, 
      new re2::RE2("(?i)^(report|status)(\\s|$)")
    )
  );

  options.push_back(
    new Option(
      "column-status", 
      {"report column status with ranges status",
      "column-status [cid=CID | name=NAME];"},
      [ptr=this](std::string& cmd){return ptr->column_status(cmd);}, 
      new re2::RE2("(?i)^(report-column|column-status)")
    )
  );

  options.push_back(
    new Option(
      "rangers-status", 
      {"report rangers status by Manager of column or Rangers-Role",
      "rangers-status [without | cid=CID | name=NAME];"},
      [ptr=this](std::string& cmd){return ptr->rangers_status(cmd);}, 
      new re2::RE2("(?i)^(report-rangers|rangers-status)")
    )
  );

  Env::Clients::init(
    std::make_shared<client::Clients>(
      nullptr,
      std::make_shared<client::AppContext>()
    )
  );
}

bool Mngr::cluster_status(std::string&) {
  std::string message;

  client::Mngr::Hosts hosts;
  auto groups = Env::Clients::get()->mngrs_groups->get_groups();
  for(auto& g : groups) {
    auto tmp =  g->get_hosts();
    hosts.insert(hosts.end(), tmp.begin(), tmp.end());
  }
  if(hosts.empty()) {
    err = Error::FAILED_EXPECTATION;
    message.append("Empty swc.mngr.host config\n");
    return error(message);
  }

  for(auto& endpoints : hosts) {
    std::promise<void>  r_promise;
    Protocol::Mngr::Req::ClusterStatus::request(
      endpoints,
      [this, await=&r_promise] 
      (const client::ConnQueue::ReqBase::Ptr&, const int& error) {
        err = error;
        await->set_value();
      }
    );
    r_promise.get_future().wait();
    if(err) {
      SWC_PRINT << "code=" << err 
                << " msg=" << Error::get_text(err) << SWC_PRINT_CLOSE;
      message.append(Error::get_text(err));
      message.append("\n");
      return error(message);
    }
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
    
    std::vector<std::string> ips;
    std::string host;
    if(Resolver::is_ipv4_address(host_or_ips) || 
       Resolver::is_ipv6_address(host_or_ips))
      ips.push_back(host_or_ips);
    else
      host = host_or_ips; 

    hosts.push_back(
      Resolver::get_endpoints(
        port, ips, host, {}, false
      )
    );
    if(hosts.back().empty())
      return error("Missing Endpoint");

  } else if(reader.found_token("all", 3)) {
    auto groups = Env::Clients::get()->mngrs_groups->get_groups();
    for(auto& g : groups) {
      auto tmp =  g->get_hosts();
      hosts.insert(hosts.end(), tmp.begin(), tmp.end());
    }

  } else { // reader.found_token("schemas", 7)
    hosts.push_back({});
  }

  for(auto& endpoints : hosts) {
    std::promise<void>  r_promise;
    Protocol::Mngr::Req::ManagersStatus::request(
      endpoints,
      [this, endpoints, await=&r_promise] 
      (const client::ConnQueue::ReqBase::Ptr& req, const int& error,
       const Protocol::Mngr::Params::Report::RspManagersStatus& rsp) {
        SWC_PRINT << "# by Manager(";
        if(error) {
          for(auto& p : endpoints)
            SWC_LOG_OSTREAM << p << ',';
          SWC_LOG_OSTREAM << ") - \033[31mERROR\033[00m: " << error
                          << "(" << Error::get_text(error) << ")";
        } else {
          SWC_LOG_OSTREAM << (req->queue? req->queue->get_endpoint_remote()
                                        : EndPoint()) << ") ";
          rsp.display(SWC_LOG_OSTREAM);
        }
        SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
        await->set_value();
      }
    );
    r_promise.get_future().wait();
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
      int64_t _cid;
      bool was_set = false;
      reader.read_int64_t(_cid, was_set);
      if(was_set)
        cid = _cid;
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
    
    auto schema = reader.get_schema(col_name);
    if(reader.err)
      return error(message);
    cid = schema->cid;
  } else {
    bool token = false;
    reader.expect_token("name", 4, token);
    return error(message);
  }

  std::promise<void>  r_promise;
  Protocol::Mngr::Req::ColumnStatus::request(
    Protocol::Mngr::Params::Report::ReqColumnStatus(cid),
    [this, await=&r_promise] 
    (const client::ConnQueue::ReqBase::Ptr&, const int& error,
     const Protocol::Mngr::Params::Report::RspColumnStatus& rsp) {
      if(!(err = error)) {
        SWC_PRINT << "";
        rsp.display(SWC_LOG_OSTREAM);
        SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
      }
      await->set_value();
    }
  );
  r_promise.get_future().wait();

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
      int64_t _cid;
      bool was_set = false;
      reader.read_int64_t(_cid, was_set);
      if(was_set)
        cid = _cid;
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
    
    auto schema = reader.get_schema(col_name);
    if(reader.err)
      return error(message);
    cid = schema->cid;
  }

  std::promise<void>  r_promise;
  Protocol::Mngr::Req::RangersStatus::request(
    cid,
    [this, await=&r_promise] 
    (const client::ConnQueue::ReqBase::Ptr&, const int& error,
     const Protocol::Mngr::Params::Report::RspRangersStatus& rsp) {
      if(!(err = error)) {
        SWC_PRINT << "";
        rsp.display(SWC_LOG_OSTREAM);
        SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
      }
      await->set_value();
    }
  );
  r_promise.get_future().wait();

  if(err) {
    message.append(Error::get_text(err));
    message.append("\n");
    return error(message);
  }
  return true;
}


}}} // namespace Utils::shell
