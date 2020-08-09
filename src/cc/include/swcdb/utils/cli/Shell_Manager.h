/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_utils_ShellManager_h
#define swc_utils_ShellManager_h

#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/AppContext.h"
#include "swcdb/db/Protocol/Mngr/req/Report.h"
#include "swcdb/db/client/sql/Reader.h"


namespace SWC { namespace Utils { namespace shell {


class Mngr : public Interface {
  public:
  Mngr() 
    : Interface("\033[32mSWC-DB(\033[36mmngr\033[32m)\033[33m> \033[00m",
                "/tmp/.swc-cli-manager-history") {

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
        {"report rangers status of column or rangers manager",
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

  bool column_status(std::string& cmd) {
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
          Mutex::scope lock(Logger::logger.mutex);
          rsp.display(std::cout);
          std::cout << '\n';
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

  bool rangers_status(std::string& cmd) {
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
          Mutex::scope lock(Logger::logger.mutex);
          rsp.display(std::cout);
          std::cout << '\n';
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
};



}}} // namespace Utils::shell

#endif // swc_utils_ShellManager_h