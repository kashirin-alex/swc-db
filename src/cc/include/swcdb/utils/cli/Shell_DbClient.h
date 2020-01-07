/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */

#ifndef swc_lib_utils_ShellDbClient_h
#define swc_lib_utils_ShellDbClient_h

#include "swcdb/client/sql/SQL.h"
#include "swcdb/fs/Interface.h"

#include "swcdb/db/Cells/TSV.h"

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
        {"load from 'filepath.ext' into col='ID|NAME';"},
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

  // CREATE/MODIFY/DELETE COLUMN
  const bool mng_column(Protocol::Mngr::Req::ColumnMng::Func func, 
                        std::string& cmd) {
    std::string message;
    DB::Schema::Ptr schema;
    client::SQL::parse_column_schema(err, cmd, func, schema, message);
    if(err)
      return error(message);

    std::promise<int> res;
    Protocol::Mngr::Req::ColumnMng::request(
      func,
      schema,
      [await=&res]
      (Protocol::Common::Req::ConnQueue::ReqBase::Ptr req, int error) {
        /*if(err && Func::CREATE && err != Error::COLUMN_SCHEMA_NAME_EXISTS) {
          req->request_again();
          return;
        }*/
        await->set_value(error);
      },
      300000
    );
    
    if(err = res.get_future().get()) {
      message.append(Error::get_text(err));
      message.append("\n");
      return error(message);
    }
    if(schema->cid != DB::Schema::NO_CID)
      Env::Clients::get()->schemas->remove(schema->cid);
    else
      Env::Clients::get()->schemas->remove(schema->col_name);
    return true;
  }
  
  // LIST COLUMN/s
  const bool list_columns(std::string& cmd) {
    std::vector<DB::Schema::Ptr> schemas;  
    std::string message;
    client::SQL::parse_list_columns(err, cmd, schemas, message);
    if(err) 
      return error(message);

    if(schemas.empty()) { // get all schema
      std::promise<int> res;
      Protocol::Mngr::Req::ColumnList::request(
        [&schemas, await=&res]
        (Protocol::Common::Req::ConnQueue::ReqBase::Ptr req, int error, 
         Protocol::Mngr::Params::ColumnListRsp rsp) {
          if(!error)
            schemas = rsp.schemas;
          await->set_value(error);
        },
        300000
      );
      if(err = res.get_future().get()) {
        message.append(Error::get_text(err));
        message.append("\n");
        return error(message);
      }
    }

    for(auto& schema : schemas) {
      schema->display(std::cout);
      std::cout << std::endl;
    }
    return true;
  }

  // SELECT
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
    std::string message;
    client::SQL::parse_select(err, cmd, req->specs, display_flags, message);
    if(err) 
      return error(message);

    if(display_flags & DB::DisplayFlag::SPECS) {
      std::cout << "\n\n";
      req->specs.display(
        std::cout, !(display_flags & DB::DisplayFlag::BINARY));
    }

    req->scan();
    req->wait();
    
    if(err) 
      return error(message);

    if(display_flags & DB::DisplayFlag::STATS)
      display_stats(SWC::Time::now_ns() - ts, cells_bytes, cells_count);

    return true;
  }

  void display(Protocol::Common::Req::Query::Select::Result::Ptr result,
               uint8_t display_flags, 
               size_t& cells_count, size_t& cells_bytes) const {
    DB::Schema::Ptr schema = 0;
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

  // UPDATE
  const bool update(std::string& cmd) {
    int64_t ts = Time::now_ns();
    uint8_t display_flags = 0;

    auto req = std::make_shared<Protocol::Common::Req::Query::Update>();
    auto req_fraction = std::make_shared<Protocol::Common::Req::Query::Update>();
    std::string message;
    client::SQL::parse_update(
      err, cmd, 
      *req->columns.get(), *req_fraction->columns.get(), 
      display_flags, message
    );
    if(err) 
      return error(message);
    
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
    if(err) 
      return error(message);
    return true;
  }

  // LOAD
  const bool load(std::string& cmd) {
    int64_t ts = Time::now_ns();
    uint8_t display_flags = 0;
    
    std::string filepath;    
    int64_t cid = DB::Schema::NO_CID;
    std::string message;
    client::SQL::parse_load(err, cmd, filepath, cid, display_flags, message);
    if(err) 
      return error(message);
                   
    Env::FsInterface::init(FS::fs_type(
      Env::Config::settings()->get<std::string>("swc.fs")));

    auto smartfd = FS::SmartFd::make_ptr(filepath, 0);
    size_t cells_count = 0;
    size_t cells_bytes = 0;
    read_and_load(smartfd, cid, cells_count, cells_bytes, message);

    if(display_flags & DB::DisplayFlag::STATS) 
      display_stats(SWC::Time::now_ns() - ts, cells_bytes, cells_count);
    if(err) 
      return error(message);

    Env::FsInterface::reset();
    return true;
  }

  void read_and_load(FS::SmartFd::Ptr smartfd, int64_t cid,
                     size_t& cells_count, size_t& cells_bytes,
                     std::string& message) {
    auto update_req = std::make_shared<Protocol::Common::Req::Query::Update>();
    
    auto schema = Env::Clients::get()->schemas->get(err, cid);
    if(err)
      return;
    size_t length = Env::FsInterface::fs()->length(err, smartfd->filepath());
    if(err)
      return;

    update_req->columns->create(schema);
    size_t offset = 0;
    size_t r_sz;
    
    auto col = update_req->columns->get_col(schema->cid);
    StaticBuffer buffer;
    DynamicBuffer buffer_remain;
    DynamicBuffer buffer_write;
    std::vector<std::string> header;
    bool has_ts;
    bool ok;
    size_t s_sz;
    size_t cell_pos = 0;
    size_t cell_mark = 0;

    do {
      err = Error::OK;
      if(!smartfd->valid() 
        && !Env::FsInterface::interface()->open(err, smartfd) 
        && err)
        break;
      if(err)
        continue;
      
      r_sz = length - offset > 8388608 ? 8388608 : length - offset;
      for(;;) {
        buffer.free();
        err = Error::OK;

        if(Env::FsInterface::fs()->pread(
            err, smartfd, offset, &buffer, r_sz) != r_sz) {
          int tmperr = Error::OK;
          Env::FsInterface::fs()->close(tmperr, smartfd);
          if(err != Error::FS_EOF){
            if(!Env::FsInterface::interface()->open(err, smartfd))
              break;
            continue;
          }
        }
        break;
      }
      if(err)
        break;

      offset += r_sz;
      
      if(buffer_remain.fill()) {
        buffer_write.add(buffer_remain.base, buffer_remain.fill());
        buffer_write.add(buffer.base, buffer.size);
        buffer_remain.free();
      } else {
        buffer_write.set(buffer.base, buffer.size);
      }

      const uint8_t* ptr = buffer_write.base;
      size_t remain = buffer_write.fill();
      
      if(header.empty()) {
        DB::Cells::TSV::header_read(&ptr, &remain, has_ts, header);
        if(header.empty()) {
          err = Error::SQL_BAD_LOAD_FILE_FORMAT;
          message.append("TSV file missing columns defintion header");
          break;
        }
      }

      while(remain) {
        DB::Cells::Cell cell;
        try {
          cell_mark = remain;
          ok = DB::Cells::TSV::read(
            &ptr, &remain, has_ts, schema->col_type, cell);
          cell_pos += cell_mark-remain;
        } catch(const std::exception& ex) {
          message.append(ex.what());
          message.append(", corrupted '");
          message.append(smartfd->filepath());
          message.append("' starting at-offset=");
          message.append(std::to_string(cell_pos));
          offset = length;
          break;
        }
        if(ok) {
          col->add(cell);
          cells_count++;
          //if((cells_count % 100) == 0)
          //  std::cout << cells_count << " : " << cell_pos << "\n";
          cells_bytes += cell.encoded_length();
          
          s_sz = col->size_bytes();
          if(update_req->result->completion && s_sz > update_req->buff_sz*3)
            update_req->wait();
          if(!update_req->result->completion && s_sz >= update_req->buff_sz)
            update_req->commit(col);
          
        } else  {
          buffer_remain.add(ptr, remain);
          break;
        }
      }
      
      buffer_write.free();
      buffer.free();

    } while(offset < length);

    if(smartfd->valid())
      Env::FsInterface::fs()->close(err, smartfd);
    
    if(col->size_bytes() && !update_req->result->completion)
      update_req->commit(col);
    
    update_req->wait();
  }

  // DUMP
  const bool dump(std::string& cmd) {
    int64_t ts = Time::now_ns();
    FS::SmartFd::Ptr smartfd = nullptr;
    size_t cells_count = 0;
    size_t cells_bytes = 0;
    auto req = std::make_shared<Protocol::Common::Req::Query::Select>(
      [this, &smartfd, &cells_count, &cells_bytes]
      (Protocol::Common::Req::Query::Select::Result::Ptr result) {
        write_to_file(result, smartfd, cells_count, cells_bytes);
      },
      true // cb on partial rsp
    );
    
    uint8_t display_flags = 0;
    std::string message;
    std::string filepath;
    client::SQL::parse_dump(
      err, cmd, filepath, req->specs, display_flags, message);
    if(err) 
      return error(message);
      
    Env::FsInterface::init(FS::fs_type(
      Env::Config::settings()->get<std::string>("swc.fs")));
      
    auto at = filepath.find_last_of("/");
    if(at != std::string::npos) {
      std::string base_path = filepath.substr(0, at);
      Env::FsInterface::interface()->mkdirs(err, base_path);
      if(err) 
        return error(Error::get_text(err));
    }

    smartfd = FS::SmartFd::make_ptr(
      filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE);
    while(Env::FsInterface::interface()->create(err, smartfd, 0, 0, 0));
    if(err) 
      return error(Error::get_text(err));

    if(display_flags & DB::DisplayFlag::SPECS) {
      std::cout << "\n\n";
      req->specs.display(
        std::cout, !(display_flags & DB::DisplayFlag::BINARY));
    }

    req->scan();
    req->wait();

    if(err) 
      return error(Error::get_text(err));

    Env::FsInterface::fs()->close(err, smartfd);
    if(err) 
      return error(Error::get_text(err));

    if(display_flags & DB::DisplayFlag::STATS) {
      size_t len = Env::FsInterface::fs()->length(err, smartfd->filepath());
      if(err) 
        return error(Error::get_text(err));

      display_stats(SWC::Time::now_ns() - ts, cells_bytes, cells_count);
      std::cout << " File Size:              " << len  << " bytes\n";
    }

    Env::FsInterface::reset();
    return true;
  }

  void write_to_file(Protocol::Common::Req::Query::Select::Result::Ptr result,
                     FS::SmartFd::Ptr& smartfd, 
                     size_t& cells_count, size_t& cells_bytes) const {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Vector vec; 
    DynamicBuffer buffer;
    
    do {
      for(auto cid : result->get_cids()) {
        schema = Env::Clients::get()->schemas->get(err, cid);
        if(err)
          break;
        if(!cells_count)
          DB::Cells::TSV::header_write(schema->col_type, buffer); // OUT_FLAGS

        vec.free();
        result->get_cells(cid, vec);
        for(auto& cell : vec.cells) {

          cells_count++;
          cells_bytes += cell->encoded_length();
          
          DB::Cells::TSV::write(
            *cell, schema->col_type, buffer); // OUT_FLAGS
          
          delete cell;
          cell = nullptr;

          if(buffer.fill() >= 8388608) {
            write_cells(smartfd, buffer);
            if(err)
              break;
          }
        }
      }
    } while(!vec.cells.empty() || err);
    
    if(buffer.fill() && !err) 
      write_cells(smartfd, buffer);
  }

  void write_cells(FS::SmartFd::Ptr& smartfd, DynamicBuffer& buffer) const {    
    StaticBuffer buff_write(buffer);

    Env::FsInterface::fs()->append(
      err,
      smartfd, 
      buff_write, 
      FS::Flags::FLUSH
    );
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
  
};



}}} // namespace Utils::shell

#endif // swc_lib_utils_ShellDbClient_h