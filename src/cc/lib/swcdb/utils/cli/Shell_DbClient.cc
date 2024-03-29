/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/utils/cli/Shell_DbClient.h"
#include "swcdb/db/client/Query/Select/Handlers/Common.h"
#include "swcdb/db/client/Query/Update/Handlers/Common.h"

#include "swcdb/common/Stats/FlowRate.h"

#include "swcdb/db/Types/SystemColumn.h"

#include "swcdb/fs/Interface.h"
#include "swcdb/common/Files/TSV.h"

#include "swcdb/db/Protocol/Mngr/req/ColumnMng_Sync.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnCompact_Sync.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnGet_Sync.h"
#include "swcdb/db/Protocol/Mngr/req/ColumnList_Sync.h"

#include "swcdb/db/Protocol/Bkr/req/ColumnMng_Sync.h"
#include "swcdb/db/Protocol/Bkr/req/ColumnCompact_Sync.h"
#include "swcdb/db/Protocol/Bkr/req/ColumnGet_Sync.h"
#include "swcdb/db/Protocol/Bkr/req/ColumnList_Sync.h"


namespace SWC { namespace Utils { namespace shell {


DbClient::DbClient()
  : Interface("\033[32mSWC-DB(\033[36mclient\033[32m)\033[33m> \033[00m",
              "/tmp/.swc-cli-dbclient-history",
              CLI::DBCLIENT),
    with_broker(Env::Config::settings()->get_bool("with-broker")),
    clients(
      (with_broker
        ? client::Clients::make(
            *Env::Config::settings(),
            Comm::IoContext::make("DbClient", 8),
            nullptr  // std::make_shared<client::BrokerContext>()
          )
        : client::Clients::make(
            *Env::Config::settings(),
            Comm::IoContext::make("DbClient", 8),
            nullptr, // std::make_shared<client::ManagerContext>()
            nullptr  // std::make_shared<client::RangerContext>()
          )
      )->init()
    ) {

  add_option(
    "add column",
    {"add column|schema (schema definitions [name=value ]);"},
    [ptr=this](std::string& cmd){
      return ptr->mng_column(
        Comm::Protocol::Mngr::Params::ColumnMng::Function::CREATE, cmd);
    },
    "(?i)^(add|create)\\s+(column|schema)(.*|$)"
  );
  add_option(
    "modify column",
    {"modify column|schema (schema definitions [name=value ]);"},
    [ptr=this](std::string& cmd){
      return ptr->mng_column(
        Comm::Protocol::Mngr::Params::ColumnMng::Function::MODIFY, cmd);
    },
    "(?i)^(modify|change|update)\\s+(column|schema)(.*|$)"
  );
  add_option(
    "delete column",
    {"delete column|schema (schema definitions [name=value ]);"},
    [ptr=this](std::string& cmd){
      return ptr->mng_column(
        Comm::Protocol::Mngr::Params::ColumnMng::Function::REMOVE, cmd);
    },
    "(?i)^(delete|remove)\\s+(column|schema)(.*|$)"
  );
  add_option(
    "list columns",
    {"list|get column|s [OUTPUT_FLAGS]",
    "     [(NAME|ID),.., Comp'expr',.., tags Comp[Comp'expr',..]];",
    "* OUTPUT_FLAGS: OUTPUT_ONLY_CID"},
    [ptr=this](std::string& cmd){return ptr->list_columns(cmd);},
    "(?i)^(get|list)\\s+((column(|s))|(schema|s))(.*|$)"
  );
  add_option(
    "compact column",
    {"compact column|s",
     "     [(NAME|ID),.., Comp'expr',.., tags Comp[Comp'expr',..]];"},
    [ptr=this](std::string& cmd) {
      return ptr->compact_column(cmd);
    },
    "(?i)^(compact)\\s+(column(|s))(.*|$)"
  );
  add_option(
    "select",
    {"select where [Columns[Cells[Interval Flags]]] Flags DisplayFlags;",
    "-> select where COL(NAME|ID,.,Comp'expr',.,tags Comp[Comp'expr',..])",
    "                 = (cells=(Interval Flags)) AND",
    "     COL(NAME-2|ID-2,) = ( cells=(Interval Flags) AND cells=(",
    "       [F-begin] <= range <= [F-end]                   AND",
    "       [[COMP 'F-start'] <=  key  <= [COMP 'F-finish'] AND]",
    "       'TS-begin' <= timestamp <= 'TS-finish'          AND",
    "       offset_key = [F] offset_rev='TS'                AND",
    "       value COMP 'DATA'                                  ",
    "       LIMIT=NUM OFFSET=NUM MAX_VERSIONS=NUM ONLY_KEYS ONLY_DELETES",
    "       update[OP](TIMESTAMP, VALUE, ENC) |OR| DELETE_MATCHING )",
    "     ) DISPLAY_* TIMESTAMP, DATETIME, SPECS, STATS, BINARY, COLUMN;",
    "* DATA-value: PLAN, COUNTER, SERIAL([ID:TYPE:COMP \"VALUE\", ..])",
    "* [OP] of update: ",
    "    REPLACE (=)",
    "    APPEND (+=)",
    "    PREPEND (=+)",
    "    INSERT (+:#)",
    "    OVERWRITE (=:#)",
    "    SERIAL (~=): [ID:TYPE:[FIELD-OP]val, ..]"},
    [ptr=this](std::string& cmd){return ptr->select(cmd);},
    "(?i)^(select)(\\s+|$)"
  );
  add_option(
    "update",
    {"update cell(FLAG, CID|NAME, KEY, TIMESTAMP, VALUE, ENC), CELL(..) ;",
    "-> UPDATE cell(DELETE_LE,  CID, ['K','E','Y']              ),",
    "          cell(DELETE_EQ,  CID, ['K','E','Y'], TS          ),",
    "          cell(INSERT,     CID, ['K','E','Y'], ASC, TS, '' ),",
    "          cell(INSERT,     CID, ['K','E','Y'], DESC        ),",
    "          cell(INSERT,     NAME, ['K','E','Y'], '', 'DATA' );",
    "* FLAG: INSERT|1 DELETE_LE|2 DELETE_EQ|3 ",
    "* Encoder(ENC): at INSERT with DATA, options: ZLIB|2 SNAPPY|3 ZSTD|4",
    "* DATA: PLAIN( val ) COUNTER( -/+/=val ) SERIAL( [ID:TYPE:val, ..] )",
    },
    [ptr=this](std::string& cmd){return ptr->update(cmd);},
    "(?i)^(update)(\\s+|$)"
  );
  add_option(
    "dump",
    {"dump col='ID|NAME' into [FS] path='folder/path/' [FORMAT]",
    "   where [cells=(Interval Flags) AND .. ] OutputFlags DisplayFlags;",
    "-> dump col='ColName' into fs=hadoop_jvm path='FolderName' ",
    "     split=1GB ext=zst level=6 OUTPUT_NO_* TS/VALUE|ENCODE;",
    "* FS optional: [fs=Type] Write to the specified Type",
    "* FORMAT optional: split=1GB ext=zst level=INT(ext-dependent)",
    },
    [ptr=this](std::string& cmd){return ptr->dump(cmd);},
    "(?i)^(dump)(\\s+|$)"
  );
  add_option(
    "load",
    {"load from [FS] path='folder/path/' into col='ID|NAME' DisplayFlags;",
    "* FS optional: [fs=Type] Read from the specified Type"},
    [ptr=this](std::string& cmd){return ptr->load(cmd);},
    "(?i)^(load)(\\s+|$)"
  );

  SWC_ASSERT(!with_broker || clients->brokers.has_endpoints());
}

DbClient::~DbClient() {
  clients->stop();
}

// CREATE/MODIFY/REMOVE COLUMN
bool DbClient::mng_column(
                  Comm::Protocol::Mngr::Params::ColumnMng::Function func,
                  std::string& cmd) {
  std::string message;
  DB::Schema::Ptr schema;
  client::SQL::parse_column_schema(err, cmd, func, schema, message);
  if(err)
    return error(message);

  with_broker
    ? Comm::Protocol::Bkr::Req::ColumnMng_Sync::request(
        func, schema, 1800000, clients, err)
    : Comm::Protocol::Mngr::Req::ColumnMng_Sync::request(
        func, schema, 1800000, clients, err);
  if(err) {
    message.append(Error::get_text(err));
    message.append("\n");
    return error(message);
  }
  schema->cid == DB::Schema::NO_CID
    ? clients->schemas.remove(schema->col_name)
    : clients->schemas.remove(schema->cid);
  return true;
}

// COMPACT COLUMN
bool DbClient::compact_column(std::string& cmd) {
  DB::SchemasVec schemas;
  Comm::Protocol::Mngr::Params::ColumnListReq params;
  std::string message;
  client::SQL::parse_list_columns(
    err, clients, cmd, schemas, params, message, "compact");
  if(err)
    return error(message);

  if(!params.patterns.names.empty() ||
      params.patterns.tags.comp != Condition::NONE ||
      schemas.empty()) {
    // get all schemas or on patterns
    DB::SchemasVec _schemas;
    with_broker
      ? Comm::Protocol::Bkr::Req::ColumnList_Sync::request(
          params, 300000, clients, err, _schemas)
      : Comm::Protocol::Mngr::Req::ColumnList_Sync::request(
          params, 300000, clients, err, _schemas);
    if(err) {
      message.append(Error::get_text(err));
      message.append("\n");
      return error(message);
    }
    if(schemas.empty())
      schemas = std::move(_schemas);
    else
      schemas.insert(schemas.cend(), _schemas.cbegin(), _schemas.cend());
  }
  for(auto& schema : schemas) {
    with_broker
      ? Comm::Protocol::Bkr::Req::ColumnCompact_Sync::request(
          schema->cid, 300000, clients, err)
      : Comm::Protocol::Mngr::Req::ColumnCompact_Sync::request(
          schema->cid, 300000, clients, err);
    SWC_PRINT << "Compacting Column cid=" << schema->cid
              << " '" << schema->col_name << "' ";
    Error::print(SWC_LOG_OSTREAM, err);
    SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
  }
  return true;
}

// LIST COLUMN/s
bool DbClient::list_columns(std::string& cmd) {
  DB::SchemasVec schemas;
  Comm::Protocol::Mngr::Params::ColumnListReq params;
  std::string message;
  uint8_t output_flags = 0;
  client::SQL::parse_list_columns(
    err, clients, cmd, schemas, params, output_flags, message, "list");
  if(err)
    return error(message);

  if(!params.patterns.names.empty() ||
      params.patterns.tags.comp != Condition::NONE ||
      schemas.empty()) {
    // get all schemas or on patterns
    DB::SchemasVec _schemas;
    with_broker
      ? Comm::Protocol::Bkr::Req::ColumnList_Sync::request(
          params, 300000, clients, err, _schemas)
      : Comm::Protocol::Mngr::Req::ColumnList_Sync::request(
          params, 300000, clients, err, _schemas);
    if(err) {
      message.append(Error::get_text(err));
      message.append("\n");
      return error(message);
    }
    if(schemas.empty())
      schemas = std::move(_schemas);
    else
      schemas.insert(schemas.cend(), _schemas.cbegin(), _schemas.cend());
  }

  std::sort(
    schemas.begin(), schemas.end(),
    [](const DB::Schema::Ptr& s1, const DB::Schema::Ptr& s2) {
      return s1->cid < s2->cid;
    });

  Core::MutexSptd::scope lock(Core::logger.mutex);
  if(output_flags & client::SQL::ColumnOutputFlag::ONLY_CID) {
    for(auto& schema : schemas) {
      SWC_LOG_OSTREAM << schema->cid << std::endl;
    }
  } else {
    for(auto& schema : schemas) {
      schema->display(SWC_LOG_OSTREAM);
      SWC_LOG_OSTREAM << std::endl;
    }
  }
  return true;
}

// SELECT
bool DbClient::select(std::string& cmd) {
  Time::Measure_ns t_measure;
  uint8_t display_flags = 0;
  size_t cells_count = 0;
  size_t cells_bytes = 0;

  std::string message;
  DB::Specs::Scan specs;
  client::SQL::parse_select(err, clients, cmd, specs, display_flags, message);
  if(err)
    return error(message);

  if(display_flags & DB::DisplayFlag::SPECS) {
    SWC_PRINT << "\n\n";
    specs.display(
      SWC_LOG_OSTREAM, !(display_flags & DB::DisplayFlag::BINARY));
    SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
  }

  auto hdlr = client::Query::Select::Handlers::Common::make(
    clients,
    [this, &display_flags, &cells_count, &cells_bytes]
    (const client::Query::Select::Handlers::Common::Ptr& _hdlr) {
      if(!_hdlr->error())
        display(_hdlr, display_flags, cells_count, cells_bytes);
    },
    true, // cb on partial rsp
    nullptr,
    with_broker
      ? client::Clients::BROKER
      : client::Clients::DEFAULT
  );

  hdlr->scan(err, std::move(specs));
  if(!err)
    hdlr->wait();

  if(err || (err = hdlr->error())) {
    for(cid_t cid : hdlr->get_cids())
      clients->schemas.remove(cid);
    return error(message);
  }

  if(display_flags & DB::DisplayFlag::STATS)
    display_stats(
      hdlr->profile,
      t_measure.elapsed(),
      cells_bytes,
      cells_count
    );

  return true;
}

void DbClient::display(
      const client::Query::Select::Handlers::BaseUnorderedMap::Ptr& hdlr,
      uint8_t display_flags, size_t& cells_count, size_t& cells_bytes) const {
  DB::Schema::Ptr schema;
  DB::Cells::Result cells;
  bool meta;
  size_t count_state;
  do {
    count_state = cells_count;
    for(cid_t cid : hdlr->get_cids()) {
      meta = !DB::Types::SystemColumn::is_data(cid);
      schema = clients->get_schema(err, cid);
      cells.free();
      hdlr->get_cells(cid, cells);

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
  } while(!hdlr->error() && count_state != cells_count);
}

// UPDATE
bool DbClient::update(std::string& cmd) {
  Time::Measure_ns t_measure;
  uint8_t display_flags = 0;

  auto hdlr = client::Query::Update::Handlers::Common::make(
    clients,
    nullptr,
    nullptr,
    with_broker
      ? client::Clients::BROKER
      : client::Clients::DEFAULT
  );
  std::string message;
  client::SQL::parse_update(err, cmd, hdlr, display_flags, message);
  if(err)
    return error(message);

  size_t cells_count = hdlr->size();
  size_t cells_bytes = hdlr->size_bytes();

  hdlr->commit();
  hdlr->wait();

  if(display_flags & DB::DisplayFlag::STATS)
    display_stats(
      hdlr->profile,
      t_measure.elapsed(),
      cells_bytes,
      cells_count
    );
  if((err=hdlr->error()))
    return error(message);
  return true;
}

// LOAD
bool DbClient::load(std::string& cmd) {
  Time::Measure_ns t_measure;
  DB::Cells::TSV::FileReader reader(
    clients,
    with_broker
      ? client::Clients::BROKER
      : client::Clients::DEFAULT
  );

  std::string fs;
  uint8_t display_flags = 0;
  client::SQL::parse_load(
    err,
    cmd,
    reader.hdlr,
    fs, reader.base_path, reader.cid,
    display_flags, reader.message);
  if(err)
    return error(reader.message);

  try {
    reader.interface.reset(new FS::Interface(
      Env::Config::settings(),
      FS::fs_type(
        fs.empty() ? Env::Config::settings()->get_str("swc.fs") : fs)));
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    err = e.code();
    return error(e.what());
  }
  reader.initialize();
  err = reader.err;
  if(err)
    return error(reader.message);

  reader.read_and_load();

  if(display_flags & DB::DisplayFlag::STATS) {
    client::Query::Profiling tmp;
    display_stats(
      reader.hdlr ? reader.hdlr->profile : tmp,
      t_measure.elapsed(),
      reader.cells_bytes,
      reader.cells_count,
      reader.resend_cells
    );
  }

  reader.interface->stop();

  if(err || (err = reader.err) || (err = reader.hdlr->error())) {
    if(reader.message.empty()) {
      reader.message.append(Error::get_text(err));
      reader.message.append("\n");
    }
    return error(reader.message);
  }
  return true;
}

// DUMP
bool DbClient::dump(std::string& cmd) {
  Time::Measure_ns t_measure;
  DB::Cells::TSV::FileWriter writer(clients);

  std::string fs;
  std::string ext;
  int level = 0;
  DB::Specs::Scan specs;
  uint8_t display_flags = 0;
  std::string message;
  client::SQL::parse_dump(
    err, writer.clients, cmd,
    fs, writer.base_path, writer.split_size, ext, level,
    specs,
    writer.output_flags, display_flags,
    message
  );
  if(err)
    return error(message);
  if((err = writer.set_extension(ext, level)))
    return error("Problem with file Extension");

  try {
    writer.interface.reset(new FS::Interface(
      Env::Config::settings(),
      FS::fs_type(
        fs.empty() ? Env::Config::settings()->get_str("swc.fs") : fs)));
  } catch(...) {
    const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
    err = e.code();
    return error(e.what());
  }
  writer.initialize();
  if((err = writer.err))
    return error(Error::get_text(err));

  if(display_flags & DB::DisplayFlag::SPECS) {
    SWC_PRINT << "\n\n";
    specs.display(
      SWC_LOG_OSTREAM, !(display_flags & DB::DisplayFlag::BINARY));
    SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
  }

  auto hdlr = client::Query::Select::Handlers::Common::make(
    writer.clients,
    [&writer]
    (const client::Query::Select::Handlers::Common::Ptr& _hdlr) {
      writer.write(_hdlr);
      if(writer.err)
        _hdlr->valid_state.store(false);
    },
    true, // cb on partial rsp
    nullptr,
    with_broker
      ? client::Clients::BROKER
      : client::Clients::DEFAULT
  );

  hdlr->scan(err, std::move(specs));
  if(!err)
    hdlr->wait();

  writer.finalize();
  if(writer.err && !err)
    err = writer.err;

  if(display_flags & DB::DisplayFlag::STATS) {
    display_stats(
      hdlr->profile,
      t_measure.elapsed(),
      writer.cells_bytes,
      writer.cells_count
    );

    Core::Vector<FS::SmartFd::Ptr> files;
    writer.get_length(files);
    if(!writer.err) {
      SWC_PRINT << " Files Count:            " << files.size() << "\n";
      for(auto& file : files)
        SWC_LOG_OSTREAM << " File:                   " << file->filepath()
                        << " (" << file->pos()  << " bytes)\n";
      SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
    } else if(!err) {
      err = writer.err;
    }
  }

  writer.interface->stop();
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
