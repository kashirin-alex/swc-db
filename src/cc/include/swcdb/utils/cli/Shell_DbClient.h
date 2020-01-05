/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_utils_ShellDbClient_h
#define swc_lib_utils_ShellDbClient_h

#include "swcdb/client/sql/SQL.h"
#include "swcdb/fs/Interface.h"

namespace SWC { namespace Utils { namespace shell {


class DbClient : public Interface {

  public:
  DbClient() 
    : Interface("\033[32mSWC-DB(\033[36mclient\033[32m)\033[33m> \033[00m",
                "/tmp/.swc-cli-dbclient-history") {    
    
    options.push_back(
      new Option(
        "add column", 
        {"add column|schema (schema definitions [name=value ]);"},
        [ptr=this](std::string& cmd){
          return ptr->mng_column(
            Protocol::Mngr::Req::ColumnMng::Func::CREATE, cmd);
        }, 
        new re2::RE2(
          "(?i)^(add|create)\\s+(column|schema)(.*|$)")
      )
    ); 
    options.push_back(
      new Option(
        "modify column", 
        {"modify column|schema (schema definitions [name=value ]);"},
        [ptr=this](std::string& cmd){
          return ptr->mng_column(
            Protocol::Mngr::Req::ColumnMng::Func::MODIFY, cmd);
        }, 
        new re2::RE2(
          "(?i)^(modify|change|update)\\s+(column|schema)(.*|$)")
      )
    ); 
    options.push_back(
      new Option(
        "delete column", 
        {"delete column|schema (schema definitions [name=value ]);"},
        [ptr=this](std::string& cmd){
          return ptr->mng_column(
            Protocol::Mngr::Req::ColumnMng::Func::DELETE, cmd);
        }, 
        new re2::RE2(
          "(?i)^(delete|remove)\\s+(column|schema)(.*|$)")
      )
    ); 
    options.push_back(
      new Option(
        "list columns", 
        {"list|get column|s [NAME|ID,..];"},
        [ptr=this](std::string& cmd){return ptr->list_columns(cmd);}, 
        new re2::RE2(
          "(?i)^(get|list)\\s+((column(|s))|(schema|s))(.*|$)")
      )
    );    
    options.push_back(
      new Option(
        "select", 
        {"select where [Columns[Cells[Interval Flags]]] Flags Display-Flags;",
        "-> select where COL(NAME|ID,) = ( cells=(Interval Flags) ) AND",
        "     COL(NAME-2|ID-2,) = ( cells=(Interval Flags) AND cells=(",
        "       [F-begin] <= range <= [F-end]                   AND", 
        "       [COMP 'F-start'] <=  key  <= [COMP 'F-finish']  AND", 
        "       'TS-begin' <= timestamp <= 'TS-finish'          AND", 
        "       offset_key = [F] offset_rev='TS'                AND", 
        "       value COMP 'DATA'                                  ",
        "       LIMIT=NUM   OFFSET=NUM  ONLY_KEYS   ONLY_DELETES     )",
        "     ) DISPLAY_* TIMESTAMP / DATETIME / SPECS / STATS / BINARY;"},
        [ptr=this](std::string& cmd){return ptr->select(cmd);}, 
        new re2::RE2(
          "(?i)^(select)(\\s+|$)")
      )
    ); 
    options.push_back(
      new Option(
        "update", 
        {"update cell(FLAG, CID|NAME, KEY, TIMESTAMP, VALUE), CELL(..)      ;",
        "-> UPDATE ",
        "     cell(DELETE,                  CID, ['K','E','Y']             );",
        "     cell(DELETE_VERSION,          CID, ['K','E','Y'], TS         );",
        "     cell(DELETE_FRACTION,         CID, ['K','E','Y']             );",
        "     cell(DELETE_FRACTION_VERSION, CID, ['K','E'],     TS         );",
        "     cell(INSERT,                  CID, ['K','E','Y'], ASC, TS, ''),",
        "     cell(INSERT,                  CID, ['K','E','Y'], DESC       ),",
        "     cell(INSERT,                 NAME, ['K','E','Y'], '', 'DATA' ),",
        "     cell(INSERT_FRACTION,        NAME, ['K','E'],     '', 'DATA' );",
        " Flags: INSERT|1 DELETE|2 DELETE_VERSION|3 ",
        "        INSERT_FRACTION|4 DELETE_FRACTION|5 DELETE_FRACTION_VERSION|6"
        },
        [ptr=this](std::string& cmd){return ptr->update(cmd);}, 
        new re2::RE2(
          "(?i)^(update)(\\s+|$)")
      )
    );
    options.push_back(
      new Option(
        "dump", 
        {"dump col='ID|NAME' into 'filepath.ext' "
         "where [cells=(Interval Flags) AND];"},
        [ptr=this](std::string& cmd){return ptr->dump(cmd);}, 
        new re2::RE2(
          "(?i)^(dump)(\\s+|$)")
      )
    );
    options.push_back(
      new Option(
        "load", 
        {"load from 'filepath.ext' into col(ID|NAME);"},
        [ptr=this](std::string& cmd){return ptr->load(cmd);}, 
        new re2::RE2(
          "(?i)^(load)(\\s+|$)")
      )
    );
  
  
  

    Env::Clients::init(
      std::make_shared<client::Clients>(
        nullptr,
        std::make_shared<client::AppContext>()
      )
    );

  }

  const bool error(int err, const std::string& message) {
    std::cout << "\033[31mERROR\033[00m: " << message << std::flush;
    return true;
  }

  const bool mng_column(Protocol::Mngr::Req::ColumnMng::Func func, 
                        std::string& cmd) {
    std::string message;
    DB::Schema::Ptr schema;
    int err = Error::OK;
    client::SQL::parse_column_schema(err, cmd, func, schema, message);
    if(err)
      return error(err, message);

    std::promise<int> res;
    Protocol::Mngr::Req::ColumnMng::request(
      func,
      schema,
      [await=&res]
      (Protocol::Common::Req::ConnQueue::ReqBase::Ptr req, int err) {
        /*if(err && Func::CREATE && err != Error::COLUMN_SCHEMA_NAME_EXISTS) {
          req->request_again();
          return;
        }*/
        await->set_value(err);
      },
      300000
    );
    
    if(err = res.get_future().get()) {
      message.append(Error::get_text(err));
      message.append("\n");
      return error(err, message);
    }
    if(schema->cid != DB::Schema::NO_CID)
      Env::Clients::get()->schemas->remove(schema->cid);
    else
      Env::Clients::get()->schemas->remove(schema->col_name);
    return true;
  }
  
  const bool list_columns(std::string& cmd) {
    int err = Error::OK;
    std::vector<DB::Schema::Ptr> schemas;  
    std::string message;
    client::SQL::parse_list_columns(err, cmd, schemas, message);
    if(err) 
      return error(err, message);

    if(schemas.empty()) { // get all schema
      std::promise<int> res;
      Protocol::Mngr::Req::ColumnList::request(
        [&schemas, await=&res]
        (Protocol::Common::Req::ConnQueue::ReqBase::Ptr req, int err, 
         Protocol::Mngr::Params::ColumnListRsp rsp) {
          if(!err)
            schemas = rsp.schemas;
          await->set_value(err);
        },
        300000
      );
      if(err = res.get_future().get()) {
        message.append(Error::get_text(err));
        message.append("\n");
        return error(err, message);
      }
    }

    for(auto& schema : schemas) {
      schema->display(std::cout);
      std::cout << std::endl;
    }
    return true;
  }

  void display(Protocol::Common::Req::Query::Select::Result::Ptr result,
               uint8_t display_flags, 
               size_t& cells_count, size_t& cells_bytes) const {
    DB::Schema::Ptr schema = 0;
    int err = Error::OK;
    DB::Cells::Vector vec; 
    do {
      for(auto cid : result->get_cids()) {
        schema = Env::Clients::get()->schemas->get(err, cid);
        vec.free();
        result->get_cells(cid, vec);
        for(auto& cell : vec.cells) {
          cells_count++;
          cells_bytes += cell->encoded_length();
          cell->display(
            std::cout, 
            err ? Types::Column::PLAIN: schema->col_type,
            display_flags
          );
          std::cout << "\n";  
          delete cell;
          cell = nullptr;
        }
      }
    } while(!vec.cells.empty());
  }

  const bool select(std::string& cmd) {
    int64_t ts = Time::now_ns();
    uint8_t display_flags = 0;
    size_t cells_count = 0;
    size_t cells_bytes = 0;
    auto req = std::make_shared<Protocol::Common::Req::Query::Select>(
      [this, &display_flags, &cells_count, &cells_bytes]
      (Protocol::Common::Req::Query::Select::Result::Ptr result) {
        display(result, display_flags, cells_count, cells_bytes);
      },
      true // cb on partial rsp
    );
    int err = Error::OK;
    std::string message;
    client::SQL::parse_select(err, cmd, req->specs, display_flags, message);
    if(err) 
      return error(err, message);

    if(display_flags & DB::DisplayFlag::SPECS) {
      std::cout << "\n\n";
      req->specs.display(
        std::cout, !(display_flags & DB::DisplayFlag::BINARY));
    }

    req->scan();
    req->wait();

    if(display_flags & DB::DisplayFlag::STATS)
      display_stats(SWC::Time::now_ns() - ts, cells_bytes, cells_count);

    return true;
  }

  const bool update(std::string& cmd) {
    int64_t ts = Time::now_ns();
    uint8_t display_flags = 0;

    auto req = std::make_shared<Protocol::Common::Req::Query::Update>();
    auto req_fraction = std::make_shared<Protocol::Common::Req::Query::Update>();
    int err = Error::OK;
    std::string message;
    client::SQL::parse_update(
      err, cmd, 
      *req->columns.get(), *req_fraction->columns.get(), 
      display_flags, message
    );
    if(err) 
      return error(err, message);
    
    size_t cells_count = req->columns->size() 
                       + req_fraction->columns->size();
    size_t cells_bytes = req->columns->size_bytes() 
                       + req_fraction->columns->size_bytes();

    req->timeout_commit += 10*cells_count;
    req->commit();
    req->wait();

    // req->result->errored
    if(display_flags & DB::DisplayFlag::STATS) 
      display_stats(SWC::Time::now_ns() - ts, cells_bytes, cells_count);
    return true;
  }

  void display_stats(double took, double bytes, size_t cells_count) {      
    std::cout << "\n\nStatistics:\n";

    char time_base = 'n';
    if(took > 100000 && took < 10000000) { 
      took /= 1000;
      time_base = 'u';
    } else if(took < 10000000000) { 
      took /= 1000000;
      time_base = 'm';
    } else  if(took > 10000000000) {
      took /= 1000000000;
      time_base = 0;
    }

    char byte_base = 0;
    if(bytes > 1000000 && bytes < 1000000000) {
      bytes /= 1000;
      byte_base = 'K';
    } else if (bytes > 1000000000) {
      bytes /= 1000000;
      byte_base = 'M';
    }
        
    std::cout 
      << " Total Time Took:        " << took << " " << time_base  << "s\n"
      << " Total Cells Count:      " << cells_count                << "\n"
      << " Total Cells Size:       " << bytes << " " << byte_base << "B\n"
      << " Average Transfer Rate:  " << bytes/took 
                         << " " << byte_base << "B/" << time_base << "s\n" 
      << " Average Cells Rate:     " << (cells_count?cells_count/took:0)
                                         << " cell/" << time_base << "s\n"
    ;

  }

  const bool load(std::string& cmd) {
    //
    return true;
  }

  void write_to_file(Protocol::Common::Req::Query::Select::Result::Ptr result,
                     FS::SmartFd::Ptr& smartfd, int& err, 
                     size_t& cells_count, size_t& cells_bytes) const {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Vector vec; 
    DynamicBuffer buffer;
    
    do {
      for(auto cid : result->get_cids()) {
        schema = Env::Clients::get()->schemas->get(err, cid);
        if(err)
          break;
        if(!cells_count) {
          std::string header("TIMESTAMP\tFCOUNT\tFLEN\tKEY\t");
          if(Types::is_counter(schema->col_type))
            header.append("COUNT\tEQ\tSINCE");
          else
            header.append("VLEN\tVALUE");
          header.append("\n");
          buffer.add(header.data(), header.length());
        }

        vec.free();
        result->get_cells(cid, vec);
        for(auto& cell : vec.cells) {

          cells_count++;
          cells_bytes += cell->encoded_length();
          cell->write_tsv(
            buffer,
            schema->col_type
          ); // display_flags
          
          delete cell;
          cell = nullptr;

          if(buffer.fill() >= 8388608) {
            write_cells(err, smartfd, buffer);
            if(err)
              break;
          }
        }
      }
    } while(!vec.cells.empty() || err);
    
    if(buffer.fill() && !err) 
      write_cells(err, smartfd, buffer);
  }

  void write_cells(int& err, FS::SmartFd::Ptr& smartfd, 
                   DynamicBuffer& buffer) const {
    StaticBuffer buff_write(buffer);

    Env::FsInterface::fs()->append(
      err,
      smartfd, 
      buff_write, 
      FS::Flags::FLUSH
    );
  }

  const bool dump(std::string& cmd) {
    int64_t ts = Time::now_ns();
    FS::SmartFd::Ptr smartfd = nullptr;
    int err = Error::OK;
    size_t cells_count = 0;
    size_t cells_bytes = 0;
    auto req = std::make_shared<Protocol::Common::Req::Query::Select>(
      [this, &err, &smartfd, &cells_count, &cells_bytes]
      (Protocol::Common::Req::Query::Select::Result::Ptr result) {
        write_to_file(result, smartfd, err, cells_count, cells_bytes);
      },
      true // cb on partial rsp
    );
    
    uint8_t display_flags = 0;
    std::string message;
    std::string filepath;
    client::SQL::parse_dump(
      err, cmd, filepath, req->specs, display_flags, message);
    if(err) 
      return error(err, message);
      
    Env::FsInterface::init(FS::fs_type(
      Env::Config::settings()->get<std::string>("swc.fs")));
      
    auto at = filepath.find_last_of("/");
    if(at != std::string::npos) {
      std::string base_path = filepath.substr(0, at);
      Env::FsInterface::interface()->mkdirs(err, base_path);
      if(err) 
        return error(err, Error::get_text(err));
    }

    smartfd = FS::SmartFd::make_ptr(
      filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE);
    while(Env::FsInterface::interface()->create(err, smartfd, 0, 0, 0));
    if(err) 
      return error(err, Error::get_text(err));

    if(display_flags & DB::DisplayFlag::SPECS) {
      std::cout << "\n\n";
      req->specs.display(
        std::cout, !(display_flags & DB::DisplayFlag::BINARY));
    }

    req->scan();
    req->wait();
    if(err) 
      return error(err, Error::get_text(err));

    Env::FsInterface::fs()->close(err, smartfd);
    if(err) 
      return error(err, Error::get_text(err));

    if(display_flags & DB::DisplayFlag::STATS) {
      size_t len = Env::FsInterface::fs()->length(err, smartfd->filepath());
      if(err) 
        return error(err, Error::get_text(err));

      display_stats(SWC::Time::now_ns() - ts, cells_bytes, cells_count);
      std::cout << " File Size:              " << len  << " bytes\n";
    }

    Env::FsInterface::reset();

    return true;
  }

};



}}} // namespace Utils::shell

#endif // swc_lib_utils_ShellDbClient_h