/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/utils/cli/Shell_DbClient.h"

#include "swcdb/common/Stats/FlowRate.h"

#include "swcdb/db/Types/MetaColumn.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/common/Files/TSV.h"



namespace SWC { namespace Utils { namespace shell {


DbClient::DbClient()
  : Interface("\033[32mSWC-DB(\033[36mclient\033[32m)\033[33m> \033[00m",
              "/tmp/.swc-cli-dbclient-history") {

  options.push_back(
    new Option(
      "add column",
      {"add column|schema (schema definitions [name=value ]);"},
      [ptr=this](std::string& cmd){
        return ptr->mng_column(
          Comm::Protocol::Mngr::Req::ColumnMng::Func::CREATE, cmd);
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
          Comm::Protocol::Mngr::Req::ColumnMng::Func::MODIFY, cmd);
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
          Comm::Protocol::Mngr::Req::ColumnMng::Func::DELETE, cmd);
      },
      new re2::RE2(
        "(?i)^(delete|remove)\\s+(column|schema)(.*|$)")
    )
  );
  options.push_back(
    new Option(
      "list columns",
      {"list|get column|s [(NAME|ID)|Comp'expr'..];"},
      [ptr=this](std::string& cmd){return ptr->list_columns(cmd);},
      new re2::RE2(
        "(?i)^(get|list)\\s+((column(|s))|(schema|s))(.*|$)")
    )
  );
  options.push_back(
    new Option(
      "compact column",
      {"compact column|s [(NAME|ID)|Comp'expr',..];"},
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
      "-> select where COL(NAME|ID|Comp'expr',)=(cells=(Interval Flags)) AND",
      "     COL(NAME-2|ID-2,) = ( cells=(Interval Flags) AND cells=(",
      "       [F-begin] <= range <= [F-end]                   AND",
      "       [[COMP 'F-start'] <=  key  <= [COMP 'F-finish'] AND]",
      "       'TS-begin' <= timestamp <= 'TS-finish'          AND",
      "       offset_key = [F] offset_rev='TS'                AND",
      "       value COMP 'DATA'                                  ",
      "       LIMIT=NUM   OFFSET=NUM  ONLY_KEYS   ONLY_DELETES     )",
      "     ) DISPLAY_* TIMESTAMP, DATETIME, SPECS, STATS, BINARY, COLUMN;",
      "* DATA-value: PLAN, COUNTER, SERIAL([ID:TYPE:COMP \"VALUE\", ..]) "},
      [ptr=this](std::string& cmd){return ptr->select(cmd);},
      new re2::RE2(
        "(?i)^(select)(\\s+|$)")
    )
  );
  options.push_back(
    new Option(
      "update",
      {"update cell(FLAG, CID|NAME, KEY, TIMESTAMP, VALUE, ENC), CELL(..) ;",
      "-> UPDATE ",
      "     cell(DELETE,                  CID, ['K','E','Y']             );",
      "     cell(DELETE_VERSION,          CID, ['K','E','Y'], TS         );",
      "     cell(DELETE_FRACTION,         CID, ['K','E','Y']             );",
      "     cell(DELETE_FRACTION_VERSION, CID, ['K','E'],     TS         );",
      "     cell(INSERT,                  CID, ['K','E','Y'], ASC, TS, ''),",
      "     cell(INSERT,                  CID, ['K','E','Y'], DESC       ),",
      "     cell(INSERT,                 NAME, ['K','E','Y'], '', 'DATA' ),",
      "     cell(INSERT_FRACTION,        NAME, ['K','E'],     '', 'DATA' );",
      "* FLAG: INSERT|1 DELETE|2 DELETE_VERSION|3 ",
      "       INSERT_FRACTION|4 DELETE_FRACTION|5 DELETE_FRACTION_VERSION|6",
      "* Encoder(ENC): at INSERT with DATA, options: ZLIB|2 SNAPPY|3 ZSTD|4",
      "* DATA: PLAIN( val ) COUNTER( -/+/=val ) SERIAL( [ID:TYPE:val, ..] )",
      },
      [ptr=this](std::string& cmd){return ptr->update(cmd);},
      new re2::RE2(
        "(?i)^(update)(\\s+|$)")
    )
  );
  options.push_back(
    new Option(
      "dump",
      {"dump col='ID|NAME' into [FS] path='folder/path/' [FORMAT]",
      "   where [cells=(Interval Flags) AND .. ] OutputFlags DisplayFlags;",
      "-> dump col='ColName' into fs=hadoop_jvm path='FolderName' ",
      "     split=1GB ext=zst level=6 OUTPUT_NO_* TS/VALUE|ENCODE;",
      "* FS optional: [fs=Type] Write to the specified Type",
      "* FORMAT optional: split=1GB ext=zst level=INT(ext-dependent)",
      },
      [ptr=this](std::string& cmd){return ptr->dump(cmd);},
      new re2::RE2(
        "(?i)^(dump)(\\s+|$)")
    )
  );
  options.push_back(
    new Option(
      "load",
      {"load from [FS] path='folder/path/' into col='ID|NAME' DisplayFlags;",
      "* FS optional: [fs=Type] Read from the specified Type"},
      [ptr=this](std::string& cmd){return ptr->load(cmd);},
      new re2::RE2(
        "(?i)^(load)(\\s+|$)")
    )
  );


  //Env::IoCtx::init(settings->get_i32("swc.client.handlers"));
  Env::Clients::init(
    std::make_shared<client::Clients>(
      nullptr, // Env::IoCtx::io(),
      nullptr, // std::make_shared<client::ManagerContext>()
      nullptr  // std::make_shared<client::RangerContext>()
    )
  );

}

// CREATE/MODIFY/DELETE COLUMN
bool DbClient::mng_column(Comm::Protocol::Mngr::Req::ColumnMng::Func func,
                          std::string& cmd) {
  std::string message;
  DB::Schema::Ptr schema;
  client::SQL::parse_column_schema(err, cmd, func, schema, message);
  if(err)
    return error(message);

  std::promise<int> res;
  Comm::Protocol::Mngr::Req::ColumnMng::request(
    func, schema,
    [await=&res]
    (const Comm::client::ConnQueue::ReqBase::Ptr&, int error) {
      /*if(err && Func::CREATE && err != Error::COLUMN_SCHEMA_NAME_EXISTS) {
        req->request_again();
        return;
      }*/
      await->set_value(error);
    },
    1800000
  );

  if((err = res.get_future().get())) {
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
bool DbClient::compact_column(std::string& cmd) {
  std::vector<DB::Schema::Ptr> schemas;
  std::string message;
  client::SQL::parse_list_columns(err, cmd, schemas, message, "compact");
  if(err)
    return error(message);

  if(schemas.empty()) {
    std::promise<int> res;
    Comm::Protocol::Mngr::Req::ColumnList::request(
      [&schemas, await=&res]
      (const Comm::client::ConnQueue::ReqBase::Ptr&, int error,
       const Comm::Protocol::Mngr::Params::ColumnListRsp& rsp) {
        if(!error)
          schemas = rsp.schemas;
        await->set_value(error);
      },
      300000
    );
    if((err = res.get_future().get())) {
      message.append(Error::get_text(err));
      message.append("\n");
      return error(message);
    }
  }

  std::promise<void> res;
  Core::Atomic<size_t> proccessing(schemas.size());
  for(auto& schema : schemas) {
    Comm::Protocol::Mngr::Req::ColumnCompact::request(
      schema->cid,
      [schema, &proccessing, await=&res]
      (const Comm::client::ConnQueue::ReqBase::Ptr&,
       const Comm::Protocol::Mngr::Params::ColumnCompactRsp& rsp) {
        SWC_PRINT << "Compactig Column cid=" << schema->cid
                  << " '" << schema->col_name << "' err=" << rsp.err
                  << "(" << Error::get_text(rsp.err) << ")"
                  << SWC_PRINT_CLOSE;
        if(proccessing.fetch_sub(1) == 1)
          await->set_value();
      },
      300000
    );
  }
  res.get_future().wait();
  return true;
}

// LIST COLUMN/s
bool DbClient::list_columns(std::string& cmd) {
  std::vector<DB::Schema::Ptr> schemas;
  Comm::Protocol::Mngr::Params::ColumnListReq params;
  std::string message;
  client::SQL::parse_list_columns(err, cmd, schemas, params, message, "list");
  if(err)
    return error(message);

  if(!params.patterns.empty() || schemas.empty()) {
    // get all schemas or on patterns
    std::promise<int> res;
    Comm::Protocol::Mngr::Req::ColumnList::request(
      params,
      [&schemas, await=&res]
      (const Comm::client::ConnQueue::ReqBase::Ptr&, int error,
       const Comm::Protocol::Mngr::Params::ColumnListRsp& rsp) {
        if(!error)
          schemas.insert(
            schemas.end(), rsp.schemas.begin(), rsp.schemas.end());
        await->set_value(error);
      },
      300000
    );
    if((err = res.get_future().get())) {
      message.append(Error::get_text(err));
      message.append("\n");
      return error(message);
    }
  }

  std::sort(
    schemas.begin(), schemas.end(),
    [](const DB::Schema::Ptr& s1, const DB::Schema::Ptr& s2) {
      return s1->cid < s2->cid;
    });

  Core::MutexSptd::scope lock(Core::logger.mutex);
  for(auto& schema : schemas) {
    schema->display(SWC_LOG_OSTREAM);
    SWC_LOG_OSTREAM << std::endl;
  }
  return true;
}

// SELECT
bool DbClient::select(std::string& cmd) {
  int64_t ts = Time::now_ns();
  uint8_t display_flags = 0;
  size_t cells_count = 0;
  size_t cells_bytes = 0;
  auto req = std::make_shared<client::Query::Select>(
    [this, &display_flags, &cells_count, &cells_bytes]
    (const client::Query::Select::Result::Ptr& result) {
      display(result, display_flags, cells_count, cells_bytes);
    },
    true // cb on partial rsp
  );
  std::string message;
  client::SQL::parse_select(err, cmd, req->specs, display_flags, message);
  if(err)
    return error(message);

  if(display_flags & DB::DisplayFlag::SPECS) {
    SWC_PRINT << "\n\n";
    req->specs.display(
      SWC_LOG_OSTREAM, !(display_flags & DB::DisplayFlag::BINARY));
    SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
  }

  req->scan(err);
  if(!err)
    req->wait();

  if(err)
    return error(message);

  if(display_flags & DB::DisplayFlag::STATS)
    display_stats(
      req->result->profile,
      SWC::Time::now_ns() - ts,
      cells_bytes,
      cells_count
    );

  return true;
}

void DbClient::display(const client::Query::Select::Result::Ptr& result,
                       uint8_t display_flags,
                       size_t& cells_count, size_t& cells_bytes) const {
  DB::Schema::Ptr schema;
  DB::Cells::Result cells;
  bool meta;
  size_t count_state;
  do {
    count_state = cells_count;
    for(cid_t cid : result->get_cids()) {
      meta = !DB::Types::MetaColumn::is_data(cid);;
      schema = Env::Clients::get()->schemas->get(err, cid);
      cells.free();
      result->get_cells(cid, cells);

      Core::MutexSptd::scope lock(Core::logger.mutex);
      for(auto cell : cells) {
        ++cells_count;
        cells_bytes += cell->encoded_length();
        if(display_flags & DB::DisplayFlag::COLUMN)  {
          SWC_LOG_OSTREAM << schema->col_name << '\t';
        }
        cell->display(
          SWC_LOG_OSTREAM,
          err ? DB::Types::Column::PLAIN: schema->col_type,
          display_flags,
          meta
        );
        SWC_LOG_OSTREAM << std::endl;
      }
    }
  } while(count_state != cells_count);
}

// UPDATE
bool DbClient::update(std::string& cmd) {
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
    display_stats(
      req->result->profile,
      SWC::Time::now_ns() - ts,
      cells_bytes,
      cells_count
    );
  if(err)
    return error(message);
  return true;
}

// LOAD
bool DbClient::load(std::string& cmd) {
  int64_t ts = Time::now_ns();
  DB::Cells::TSV::FileReader reader;

  std::string fs;
  uint8_t display_flags = 0;
  client::SQL::parse_load(
    err, cmd,
    fs, reader.base_path, reader.cid,
    display_flags, reader.message);
  if(err)
    return error(reader.message);

  FS::Interface::Ptr fs_interface;
  try {
    fs_interface.reset(new FS::Interface(FS::fs_type(
      fs.empty() ? Env::Config::settings()->get_str("swc.fs") : fs)));
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    err = e.code();
    return error(e.what());
  }
  reader.initialize(fs_interface);
  err = reader.err;
  if(err)
    return error(reader.message);

  auto res = reader.read_and_load();

  if(display_flags & DB::DisplayFlag::STATS) {
    client::Query::Profiling tmp;
    display_stats(
      res ? res->profile : tmp,
      SWC::Time::now_ns() - ts,
      reader.cells_bytes,
      reader.cells_count,
      reader.resend_cells
    );
  }
  if(err || (err = reader.err)) {
    if(reader.message.empty()) {
      reader.message.append(Error::get_text(err));
      reader.message.append("\n");
    }
    return error(reader.message);
  }

  fs_interface = nullptr;
  return true;
}

// DUMP
bool DbClient::dump(std::string& cmd) {
  int64_t ts = Time::now_ns();
  DB::Cells::TSV::FileWriter writer;
  auto req = std::make_shared<client::Query::Select>(
    [&writer] (const client::Query::Select::Result::Ptr& result) {
      writer.write(result);
      // writer.err ? req->stop();
    },
    true // cb on partial rsp
  );

  std::string fs;
  std::string ext;
  int level = 0;
  uint8_t display_flags = 0;
  std::string message;
  client::SQL::parse_dump(
    err, cmd,
    fs, writer.base_path, writer.split_size, ext, level,
    req->specs,
    writer.output_flags, display_flags,
    message
  );
  if(err)
    return error(message);
  if((err = writer.set_extension(ext, level)))
    return error("Problem with file Extension");

  FS::Interface::Ptr fs_interface;
  try {
    fs_interface.reset(new FS::Interface(FS::fs_type(
      fs.empty() ? Env::Config::settings()->get_str("swc.fs") : fs)));
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    err = e.code();
    return error(e.what());
  }
  writer.initialize(fs_interface);
  if((err = writer.err))
    return error(Error::get_text(err));

  if(display_flags & DB::DisplayFlag::SPECS) {
    SWC_PRINT << "\n\n";
    req->specs.display(
      SWC_LOG_OSTREAM, !(display_flags & DB::DisplayFlag::BINARY));
    SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
  }

  req->scan(err);
  if(!err)
    req->wait();

  writer.finalize();
  if(writer.err && !err)
    err = writer.err;

  if(display_flags & DB::DisplayFlag::STATS) {
    display_stats(
      req->result->profile,
      SWC::Time::now_ns() - ts,
      writer.cells_bytes,
      writer.cells_count
    );

    std::vector<FS::SmartFd::Ptr> files;
    writer.get_length(files);
    if(err)
      return error(Error::get_text(err));

    SWC_PRINT << " Files Count:            " << files.size() << "\n";
    for(auto& file : files)
      SWC_LOG_OSTREAM << " File:                   " << file->filepath()
                      << " (" << file->pos()  << " bytes)\n";
    SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
  }

  fs_interface = nullptr;
  return err ? error(Error::get_text(err)) : true;
}

void DbClient::display_stats(const client::Query::Profiling& profile,
                             size_t took, size_t bytes,
                             size_t cells_count, size_t resend_cells) const {
  Common::Stats::FlowRate::Data rate(bytes, took);
  SWC_PRINT << std::endl << std::endl;
  rate.print_cells_statistics(SWC_LOG_OSTREAM, cells_count, resend_cells);
  profile.display(SWC_LOG_OSTREAM);
  SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
}




}}} // namespace Utils::shell
