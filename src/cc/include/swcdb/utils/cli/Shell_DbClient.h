/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 */

#ifndef swc_utils_ShellDbClient_h
#define swc_utils_ShellDbClient_h

#include "swcdb/db/Types/MetaColumn.h"
#include "swcdb/db/client/sql/SQL.h"
#include "swcdb/fs/Interface.h"

#include "swcdb/common/Files/TSV.h"

#include "swcdb/core/FlowRate.h"

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
        "compact column", 
        {"compact column|s [NAME|ID,..];"},
        [ptr=this](std::string& cmd) {
          return ptr->compact_column(cmd);
        }, 
        new re2::RE2(
          "(?i)^(compact)\\s+(column(|s))(.*|$)")
      )
    );  
    options.push_back(
      new Option(
        "select", 
        {"select where [Columns[Cells[Interval Flags]]] Flags DisplayFlags;",
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
        {"dump col='ID|NAME' into 'folder/path/' "
        "where [cells=(Interval Flags) AND] OutputFlags DisplayFlags;",
        "-> dump col='ColName' into 'FolderName' OUTPUT_NO_* TS / VALUE;"
        },
        [ptr=this](std::string& cmd){return ptr->dump(cmd);}, 
        new re2::RE2(
          "(?i)^(dump)(\\s+|$)")
      )
    );
    options.push_back(
      new Option(
        "load", 
        {"load from 'folder/path/' into col='ID|NAME' DisplayFlags;"},
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
  bool mng_column(Protocol::Mngr::Req::ColumnMng::Func func, 
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
      (client::ConnQueue::ReqBase::Ptr req, int error) {
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
  
  // COMPACT COLUMN
  bool compact_column(std::string& cmd) {
    std::vector<DB::Schema::Ptr> schemas;  
    std::string message;
    client::SQL::parse_list_columns(err, cmd, schemas, message, "compact");
    if(err) 
      return error(message);

    if(schemas.empty()) {
      std::promise<int> res;
      Protocol::Mngr::Req::ColumnList::request(
        [&schemas, await=&res]
        (client::ConnQueue::ReqBase::Ptr req, int error, 
         const Protocol::Mngr::Params::ColumnListRsp& rsp) {
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

    std::promise<void> res;
    std::atomic<size_t> proccessing = schemas.size();
    for(auto schema : schemas) {
      Protocol::Mngr::Req::ColumnCompact::request(
        schema->cid,
        [schema, &proccessing, await=&res]
        (client::ConnQueue::ReqBase::Ptr req, 
         const Protocol::Mngr::Params::ColumnCompactRsp& rsp) {
          SWC_PRINT << "Compactig Column cid=" << schema->cid 
                    << " '" << schema->col_name << "' err=" << rsp.err 
                    << "(" << Error::get_text(rsp.err) << ")" 
                    << SWC_PRINT_CLOSE;
          if(!--proccessing)
            await->set_value();
        },
        300000
      );
    }
    res.get_future().wait();
    return true;
  }
  
  // LIST COLUMN/s
  bool list_columns(std::string& cmd) {
    std::vector<DB::Schema::Ptr> schemas;  
    std::string message;
    client::SQL::parse_list_columns(err, cmd, schemas, message, "list");
    if(err) 
      return error(message);

    if(schemas.empty()) { // get all schema
      std::promise<int> res;
      Protocol::Mngr::Req::ColumnList::request(
        [&schemas, await=&res]
        (client::ConnQueue::ReqBase::Ptr req, int error, 
         const Protocol::Mngr::Params::ColumnListRsp& rsp) {
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

    Mutex::scope lock(Logger::logger.mutex);
    for(auto& schema : schemas) {
      schema->display(std::cout);
      std::cout << std::endl;
    }
    return true;
  }

  // SELECT
  bool select(std::string& cmd) {
    int64_t ts = Time::now_ns();
    uint8_t display_flags = 0;
    size_t cells_count = 0;
    size_t cells_bytes = 0;
    auto req = std::make_shared<client::Query::Select>(
      [this, &display_flags, &cells_count, &cells_bytes]
      (client::Query::Select::Result::Ptr result) {
        display(result, display_flags, cells_count, cells_bytes);
      },
      true // cb on partial rsp
    );
    std::string message;
    client::SQL::parse_select(err, cmd, req->specs, display_flags, message);
    if(err) 
      return error(message);

    if(display_flags & DB::DisplayFlag::SPECS) {
      Mutex::scope lock(Logger::logger.mutex);
      std::cout << "\n\n";
      req->specs.display(
        std::cout, !(display_flags & DB::DisplayFlag::BINARY));
    }

    req->scan(err);
    if(!err) 
      req->wait();
    
    if(err) 
      return error(message);

    if(display_flags & DB::DisplayFlag::STATS)
      display_stats(SWC::Time::now_ns() - ts, cells_bytes, cells_count);

    return true;
  }

  void display(client::Query::Select::Result::Ptr result,
               uint8_t display_flags, 
               size_t& cells_count, size_t& cells_bytes) const {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Result cells; 
    bool meta;
    do {
      for(auto cid : result->get_cids()) {
        meta = !Types::MetaColumn::is_data(cid);;
        schema = Env::Clients::get()->schemas->get(err, cid);
        cells.free();
        result->get_cells(cid, cells);

        Mutex::scope lock(Logger::logger.mutex);
        for(auto cell : cells) {
          ++cells_count;
          cells_bytes += cell->encoded_length();
          cell->display(
            std::cout, 
            err ? Types::Column::PLAIN: schema->col_type,
            display_flags,
            meta
          );
          std::cout << "\n";  
        }
      }
    } while(!cells.empty());
  }

  // UPDATE
  bool update(std::string& cmd) {
    int64_t ts = Time::now_ns();
    uint8_t display_flags = 0;
 
    auto req = std::make_shared<client::Query::Update>();
    std::string message;
    client::SQL::parse_update(
      err, cmd, 
      *req->columns.get(), *req->columns_onfractions.get(), 
      display_flags, message
    );
    if(err) 
      return error(message);
    
    size_t cells_count = req->columns->size() 
                       + req->columns_onfractions->size();
    size_t cells_bytes = req->columns->size_bytes() 
                       + req->columns_onfractions->size_bytes();

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
  bool load(std::string& cmd) {
    int64_t ts = Time::now_ns();
    
    Env::FsInterface::init(FS::fs_type(
      Env::Config::settings()->get_str("swc.fs")));
    DB::Cells::TSV::FileReader reader(Env::FsInterface::interface());

    uint8_t display_flags = 0;
    client::SQL::parse_load(
      err, cmd, reader.base_path, reader.cid, display_flags, reader.message);
    if(err) 
      return error(reader.message);
              
    reader.read_and_load();

    if(display_flags & DB::DisplayFlag::STATS) 
      display_stats(
        SWC::Time::now_ns() - ts, reader.cells_bytes, 
        reader.cells_count, reader.resend_cells
      );
    if(err || (err = reader.err)) {
      if(reader.message.empty()) {
        reader.message.append(Error::get_text(err));
        reader.message.append("\n");
      }
      return error(reader.message);
    }

    Env::FsInterface::reset();
    return true;
  }

  // DUMP
  bool dump(std::string& cmd) {
    int64_t ts = Time::now_ns();

    Env::FsInterface::init(FS::fs_type(
      Env::Config::settings()->get_str("swc.fs")));
    DB::Cells::TSV::FileWriter writer(Env::FsInterface::interface());
    
    auto req = std::make_shared<client::Query::Select>(
      [this, &writer] 
      (client::Query::Select::Result::Ptr result) {
        writer.write(result);   
        // writer.err ? req->stop();
      },
      true // cb on partial rsp
    );
    
    uint8_t display_flags = 0;
    std::string message;
    client::SQL::parse_dump(
      err, cmd, 
      writer.base_path, req->specs, 
      writer.output_flags, display_flags, 
      message
    );
    if(err) 
      return error(message);
    
    writer.initialize();
    if(err = writer.err)
      return error(Error::get_text(err));

    if(display_flags & DB::DisplayFlag::SPECS) {
      Mutex::scope lock(Logger::logger.mutex);
      std::cout << "\n\n";
      req->specs.display(
        std::cout, !(display_flags & DB::DisplayFlag::BINARY));
    }

    req->scan(err);
    if(!err) 
      req->wait();

    writer.finalize();
    if(writer.err && !err)
      err = writer.err;

    if(display_flags & DB::DisplayFlag::STATS) {
      display_stats(
        SWC::Time::now_ns() - ts, writer.cells_bytes, writer.cells_count);

      std::vector<FS::SmartFd::Ptr> files;
      writer.get_length(files);
      if(err) 
        return error(Error::get_text(err));
      
      Mutex::scope lock(Logger::logger.mutex);
      std::cout << " Files Count:            " << files.size() << "\n";
      for(auto& file : files)
        std::cout << " File:                   " << file->filepath() 
                  << " (" << file->pos()  << " bytes)\n";
    }

    Env::FsInterface::reset();
    return err ? error(Error::get_text(err)) : true;
  }

  void display_stats(size_t took, size_t bytes, 
                     size_t cells_count, size_t resend_cells = 0) {
    FlowRate::Data rate(bytes, took);
    SWC_PRINT;
    rate.print_cells_statistics(std::cout, cells_count, resend_cells);
    std::cout << SWC_PRINT_CLOSE;
  }
  
};



}}} // namespace Utils::shell

#endif // swc_utils_ShellDbClient_h