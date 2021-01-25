/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_app_thriftbroker_AppHandler_h
#define swcdb_app_thriftbroker_AppHandler_h


#include "swcdb/core/Semaphore.h"
#include "swcdb/db/client/sql/SQL.h"
#include "swcdb/thrift/gen-cpp/Broker.h"
#include "swcdb/thrift/utils/Converter.h"

#include <thrift/transport/TSocket.h>


namespace SWC {
namespace thrift = apache::thrift;


namespace ThriftBroker {


using namespace Thrift;


class AppHandler final : virtual public BrokerIf {
  public:

  const std::shared_ptr<thrift::transport::TSocket> socket;

  AppHandler(const std::shared_ptr<thrift::transport::TSocket>& socket)
            : socket(socket) {
  }

  virtual ~AppHandler() { }

  /* SQL any */
  void exec_sql(Result& _return, const std::string& sql) override {
    int err = Error::OK;
    std::string message;
    auto cmd = client::SQL::recognize_cmd(err, sql, message);
    if(err)
      Converter::exception(err, message);

    switch(cmd) {

      case client::SQL::CREATE_COLUMN :
      case client::SQL::MODIFY_COLUMN :
      case client::SQL::REMOVE_COLUMN :
        return sql_mng_column(sql);

      case client::SQL::GET_COLUMNS : {
        sql_list_columns(_return.schemas, sql);
        return;
      }

      case client::SQL::SELECT : {
        sql_select(_return.cells, sql);
        return;
      }

      case client::SQL::COMPACT_COLUMNS : {
        sql_compact_columns(_return.compact, sql);
        return;
      }

      case client::SQL::UPDATE :
        return sql_update(sql, 0);

      default: { }
    }
  }


  /* SQL SCHEMAS/COLUMNS */
  void sql_list_columns(Schemas& _return, const std::string& sql) override {
    int err = Error::OK;
    std::vector<DB::Schema::Ptr> dbschemas;
    std::string message;
    client::SQL::parse_list_columns(err, sql, dbschemas, message, "list");
    if(err)
      Converter::exception(err, message);

    process_results(err, dbschemas, _return);
  }

  void sql_mng_column(const std::string& sql) override {
    int err = Error::OK;
    std::string message;
    DB::Schema::Ptr schema;
    auto func = Comm::Protocol::Mngr::Req::ColumnMng::Func::CREATE;
    client::SQL::parse_column_schema(
      err, sql,
      &func,
      schema, message);
    if(err)
      Converter::exception(err, message);

    mng_column(func, schema);
  }

  void sql_compact_columns(CompactResults& _return,
                           const std::string& sql) override {
    int err = Error::OK;
    std::vector<DB::Schema::Ptr> dbschemas;
    std::string message;
    client::SQL::parse_list_columns(err, sql, dbschemas, message, "compact");
    if(err)
      Converter::exception(err, message);

    process_results(err, dbschemas, _return);
  }

  /* SQL QUERY */
  client::Query::Select::Ptr sync_select(const std::string& sql) {
    auto req = std::make_shared<client::Query::Select>();
    int err = Error::OK;
    std::string message;
    uint8_t display_flags = 0;
    client::SQL::parse_select(err, sql, req->specs, display_flags, message);
    if(!err) {
      req->scan(err);
      if(!err)
        req->wait();
    }
    if(err)
      Converter::exception(err, message);
    return req;
  }

  void sql_query(CellsGroup& _return, const std::string& sql,
                 const CellsResult::type rslt) override {
    switch(rslt) {
      case CellsResult::ON_COLUMN : {
        sql_select_rslt_on_column(_return.ccells, sql);
        break;
      }
      case CellsResult::ON_KEY : {
        sql_select_rslt_on_key(_return.kcells, sql);
        break;
      }
      case CellsResult::ON_FRACTION : {
        sql_select_rslt_on_fraction(_return.fcells, sql);
        break;
      }
      default : {
        sql_select(_return.cells, sql);
        break;
      }
    }
  }

  void sql_select(Cells& _return, const std::string& sql) override {
    auto req = sync_select(sql);

    int err = Error::OK;
    process_results(err, req->result, _return);
    if(err)
      Converter::exception(err);
  }

  void sql_select_rslt_on_column(CCells& _return,
                                 const std::string& sql) override {
    auto req = sync_select(sql);

    int err = Error::OK;
    process_results(err, req->result, _return);
    if(err)
      Converter::exception(err);
  }

  void sql_select_rslt_on_key(KCells& _return,
                              const std::string& sql) override {
    auto req = sync_select(sql);

    int err = Error::OK;
    process_results(err, req->result, _return);
    if(err)
      Converter::exception(err);
  }

  void sql_select_rslt_on_fraction(FCells& _return,
                                   const std::string& sql) override {
    auto req = sync_select(sql);

    int err = Error::OK;
    process_results(err, req->result, _return);
    if(err)
      Converter::exception(err);
  }

  /* SQL UPDATE */
  void sql_update(const std::string& sql, const int64_t updater_id) override {

    client::Query::Update::Ptr req = nullptr;
    if(updater_id)
      updater(updater_id, req);
    else
      req = std::make_shared<client::Query::Update>();

    std::string message;
    uint8_t display_flags = 0;
    int err = Error::OK;
    client::SQL::parse_update(
      err, sql,
      *req->columns.get(), *req->columns_onfractions.get(),
      display_flags, message
    );
    if(err)
      Converter::exception(err, message);

    if(updater_id) {
      req->commit_or_wait();
    } else {
      req->commit();
      req->wait();
    }
    if((err = req->result->error()))
      Converter::exception(err);
  }


  /* SPECS SCHEMAS/COLUMNS */

  void list_columns(Schemas& _return,
                    const SpecSchemas& spec) override {
    int err = Error::OK;
    std::vector<DB::Schema::Ptr> dbschemas;
    get_schemas(err, spec, dbschemas);
    process_results(err, dbschemas, _return);
  }

  void mng_column(const SchemaFunc::type func,
                  const Schema& schema) override {
    DB::Schema::Ptr dbschema = DB::Schema::make();
    Converter::set(schema, dbschema);
    mng_column(
      (Comm::Protocol::Mngr::Req::ColumnMng::Func)(uint8_t)func,
      dbschema
    );
  }

  void compact_columns(CompactResults& _return,
                       const SpecSchemas& spec) override {
    int err = Error::OK;
    std::vector<DB::Schema::Ptr> dbschemas;
    get_schemas(err, spec, dbschemas);
    process_results(err, dbschemas, _return);
  }

  /* SPECS SCAN QUERY */
  client::Query::Select::Ptr sync_select(const SpecScan& spec) {
    auto req = std::make_shared<client::Query::Select>();
    int err = Error::OK;

    if(spec.__isset.flags)
      Converter::set(spec.flags, req->specs.flags);

    DB::Schema::Ptr schema;
    DB::Specs::Interval::Ptr dbintval;

    for(auto& col : spec.columns) {
      schema = Env::Clients::get()->schemas->get(err, col.cid);
      if(!schema)
        Converter::exception(err, "cid=" + std::to_string(col.cid));

      req->specs.columns.push_back(DB::Specs::Column::make_ptr(col.cid));
      auto& dbcol = req->specs.columns.back();

      for(auto& intval : col.intervals) {
        dbintval = DB::Specs::Interval::make_ptr(schema->col_type);
        Converter::set(intval, *dbintval.get());
        dbcol->intervals.push_back(dbintval);
      }
    }

    for(auto& col : spec.columns_serial) {
      schema = Env::Clients::get()->schemas->get(err, col.cid);
      if(!schema)
        Converter::exception(err, "cid=" + std::to_string(col.cid));

      req->specs.columns.push_back(DB::Specs::Column::make_ptr(col.cid));
      auto& dbcol = req->specs.columns.back();

      for(auto& intval : col.intervals) {
        dbintval = DB::Specs::Interval::make_ptr(schema->col_type);
        Converter::set(intval, *dbintval.get());
        dbcol->intervals.push_back(dbintval);
      }
    }

    if(!err) {
      req->scan(err);
      if(!err)
        req->wait();
    }
    if(err)
      Converter::exception(err);
    return req;
  }

  void scan_rslt_on(CellsGroup& _return, const SpecScan& specs,
                    const CellsResult::type rslt) override {
    switch(rslt) {
      case CellsResult::ON_COLUMN : {
        scan_rslt_on_column(_return.ccells, specs);
        break;
      }
      case CellsResult::ON_KEY : {
        scan_rslt_on_key(_return.kcells, specs);
        break;
      }
      case CellsResult::ON_FRACTION : {
        scan_rslt_on_fraction(_return.fcells, specs);
        break;
      }
      default : {
        scan(_return.cells, specs);
        break;
      }
    }
  }

  void scan(Cells& _return, const SpecScan& specs) override {
    auto req = sync_select(specs);

    int err = Error::OK;
    process_results(err, req->result, _return);
    if(err)
      Converter::exception(err);
  }

  void scan_rslt_on_column(CCells& _return, const SpecScan& specs) override {
    auto req = sync_select(specs);

    int err = Error::OK;
    process_results(err, req->result, _return);
    if(err)
      Converter::exception(err);
  }

  void scan_rslt_on_key(KCells& _return, const SpecScan& specs) override {
    auto req = sync_select(specs);

    int err = Error::OK;
    process_results(err, req->result, _return);
    if(err)
      Converter::exception(err);
  }

  void scan_rslt_on_fraction(FCells& _return, const SpecScan& specs) override {
    auto req = sync_select(specs);

    int err = Error::OK;
    process_results(err, req->result, _return);
    if(err)
      Converter::exception(err);
  }


  /* UPDATER */
  int64_t updater_create(const int32_t buffer_size) override {
    std::scoped_lock lock(m_mutex);

    int64_t id = 1;
    for(auto it = m_updaters.begin();
        it != m_updaters.end();
        it = m_updaters.find(++id)
    );
    m_updaters[id] = std::make_shared<client::Query::Update>();
    if(buffer_size)
      m_updaters[id]->buff_sz = buffer_size;
    return id;
  }

  void updater_close(const int64_t id) override {
    client::Query::Update:: Ptr req;
    {
      std::scoped_lock lock(m_mutex);

      auto it = m_updaters.find(id);
      if(it == m_updaters.end())
        Converter::exception(ERANGE, "Updater ID not found");
      req = it->second;
      m_updaters.erase(it);
    }
    updater_close(req);
  }

  /* UPDATE */
  void update(const UCCells& cells, const int64_t updater_id) override  {
    client::Query::Update::Ptr req = nullptr;
    if(updater_id)
      updater(updater_id, req);
    else
      req = std::make_shared<client::Query::Update>();

    int err = Error::OK;
    DB::Cells::Cell dbcell;
    cid_t cid;
    for(auto& col_cells : cells) {
      auto col = req->columns->get_col(cid = col_cells.first);
      if(!col) {
        auto schema = Env::Clients::get()->schemas->get(err, cid);
        if(err)
          Converter::exception(err);
        req->columns->create(schema);
        col = req->columns->get_col(cid);
      }
      for(auto& cell : col_cells.second) {
        dbcell.flag = (uint8_t)cell.f;
        dbcell.key.read(cell.k);
        dbcell.control = 0;
        if(cell.__isset.ts)
          dbcell.set_timestamp(cell.ts);
        if(cell.__isset.ts_desc)
          dbcell.set_time_order_desc(cell.ts_desc);

        cell.__isset.encoder
          ? dbcell.set_value((DB::Types::Encoder)(uint8_t)cell.encoder, cell.v)
          : dbcell.set_value(cell.v);

        col->add(dbcell);
        req->commit_or_wait(col);
      }
    }

    if(updater_id) {
      req->commit_or_wait();
    } else {
      req->commit();
      req->wait();
    }
    if((err = req->result->error()))
      Converter::exception(err);
  }

  /* UPDATE-SERIAL */
  void update_serial(const UCCellsSerial& cells,
                     const int64_t updater_id) override {
    client::Query::Update::Ptr req = nullptr;
    if(updater_id)
      updater(updater_id, req);
    else
      req = std::make_shared<client::Query::Update>();

    int err = Error::OK;
    DB::Cells::Cell dbcell;
    cid_t cid;
    for(auto& col_cells : cells) {
      auto col = req->columns->get_col(cid = col_cells.first);
      if(!col) {
        auto schema = Env::Clients::get()->schemas->get(err, cid);
        if(err)
          Converter::exception(err);
        req->columns->create(schema);
        col = req->columns->get_col(cid);
      }
      for(auto& cell : col_cells.second) {
        dbcell.flag = (uint8_t)cell.f;
        dbcell.key.read(cell.k);
        dbcell.control = 0;
        if(cell.__isset.ts)
          dbcell.set_timestamp(cell.ts);
        if(cell.__isset.ts_desc)
          dbcell.set_time_order_desc(cell.ts_desc);

        DB::Cell::Serial::Value::FieldsWriter wfields;
        size_t len = 0;
        for(auto& fields : cell.v) {
          if(fields.__isset.v_int64)
            len += 16;
          if(fields.__isset.v_double)
            len += 24;
          if(!fields.v_bytes.empty())
            len += fields.v_bytes.size() + 8;
          if(!fields.v_key.empty())
            len += fields.v_key.size() + 8;
          if(!fields.v_li.empty())
            len += fields.v_li.size() * 16;
          for(auto& v : fields.v_lb)
            len += v.size() + 8;
        }
        wfields.ensure(len);
        for(auto& fields : cell.v) {
          if(fields.__isset.v_int64)
            wfields.add(fields.field_id, fields.v_int64);
          if(fields.__isset.v_double)
            wfields.add(fields.field_id, (long double)fields.v_double);
          if(!fields.v_bytes.empty())
            wfields.add(fields.field_id, fields.v_bytes);
          if(!fields.v_key.empty()) {
            DB::Cell::Serial::Value::Field_KEY field;
            field.fid = fields.field_id;
            Converter::set(fields.v_key, field.key);
            //
            wfields.add(&field);
          }
          if(!fields.v_li.empty())
            wfields.add(fields.field_id, fields.v_li);
          if(!fields.v_lb.empty())
            wfields.add(fields.field_id, fields.v_lb);
        }

        cell.__isset.encoder
          ? dbcell.set_value((DB::Types::Encoder)(uint8_t)cell.encoder,
                              wfields.base, wfields.fill() )
          : dbcell.set_value(wfields.base, wfields.fill(), false);

        col->add(dbcell);
        req->commit_or_wait(col);
      }
    }

    if(updater_id) {
      req->commit_or_wait();
    } else {
      req->commit();
      req->wait();
    }
    if((err = req->result->error()))
      Converter::exception(err);
  }

  void disconnected() {
    updater_close();
  }


  private:

  void updater_close() {
    client::Query::Update:: Ptr req;
    for(;;) {
      {
        std::scoped_lock lock(m_mutex);
        auto it = m_updaters.begin();
        if(it == m_updaters.end())
          break;
        m_updaters.erase(it);
        req = it->second;
      }
      try { updater_close(req); } catch(...) {}
    }
  }

  void updater(const int64_t id, client::Query::Update::Ptr& req) {
    std::scoped_lock lock(m_mutex);

    auto it = m_updaters.find(id);
    if(it == m_updaters.end())
      Converter::exception(ERANGE, "Updater ID not found");
    req = it->second;
  }

  void updater_close(const client::Query::Update::Ptr& req) {
    req->commit_if_need();
    req->wait();
    int err = req->result->error();
    if(err)
      Converter::exception(err);
  }

  void get_schemas(int& err, const SpecSchemas& spec,
                   std::vector<DB::Schema::Ptr>& dbschemas) {
    if(!spec.patterns.empty()) {
      std::vector<DB::Schemas::Pattern> dbpatterns;
      dbpatterns.resize(spec.patterns.size());
      size_t i = 0;
      for(auto& pattern : spec.patterns) {
        dbpatterns[i].comp  = (Condition::Comp)(uint8_t)pattern.comp;
        dbpatterns[i].value = pattern.value;
        ++i;
      }
      Env::Clients::get()->schemas->get(err, dbpatterns, dbschemas);
      if(err)
        Converter::exception(
          err, "problem getting columns schemas on patterns");
    }

    DB::Schema::Ptr schema = 0;
    for(auto& cid : spec.cids) {
      schema = Env::Clients::get()->schemas->get(err, cid);
      if(!schema && !err)
        err = Error::COLUMN_SCHEMA_MISSING;
      if(err)
        Converter::exception(
          err, "problem getting column cid='"+std::to_string(cid)+"' schema");
      dbschemas.push_back(schema);
    }

    for(auto& name : spec.names) {
      schema = Env::Clients::get()->schemas->get(err, name);
      if(!schema && !err)
        err = Error::COLUMN_SCHEMA_MISSING;
      if(err)
        Converter::exception(
          err, "problem getting column name='"+name+"' schema");
      dbschemas.push_back(schema);
    }
  }

  void mng_column(Comm::Protocol::Mngr::Req::ColumnMng::Func func,
                  DB::Schema::Ptr& schema) {
    std::promise<int> res;
    Comm::Protocol::Mngr::Req::ColumnMng::request(
      func, schema,
      [await=&res] (const Comm::client::ConnQueue::ReqBase::Ptr&, int error) {
        await->set_value(error);
      },
      300000
    );
    int err = res.get_future().get();
    if(err)
      Converter::exception(err);

    if(schema->cid != DB::Schema::NO_CID)
      Env::Clients::get()->schemas->remove(schema->cid);
    else
      Env::Clients::get()->schemas->remove(schema->col_name);
  }

  static void process_results(
          int& err, std::vector<DB::Schema::Ptr>& dbschemas,
          Schemas& _return) {
    if(dbschemas.empty()) { // get all schema
      std::promise<int> res;
      Comm::Protocol::Mngr::Req::ColumnList::request(
        [&dbschemas, await=&res]
        (const Comm::client::ConnQueue::ReqBase::Ptr&, int error,
         const Comm::Protocol::Mngr::Params::ColumnListRsp& rsp) {
          if(!error)
            dbschemas = rsp.schemas;
          await->set_value(error);
        },
        300000
      );
      if((err = res.get_future().get()))
        Converter::exception(err);
    }

    _return.resize(dbschemas.size());
    uint32_t c = 0;
    for(auto& dbschema : dbschemas) {
      Converter::set(dbschema, _return[c]);
      ++c;
    }
  }

  static void process_results(
          int& err, std::vector<DB::Schema::Ptr>& dbschemas,
          CompactResults& _return) {
    if(dbschemas.empty()) { // get all schema
      std::promise<int> res;
      Comm::Protocol::Mngr::Req::ColumnList::request(
        [&dbschemas, await=&res]
        (const Comm::client::ConnQueue::ReqBase::Ptr&, int error,
         const Comm::Protocol::Mngr::Params::ColumnListRsp& rsp) {
          if(!error)
            dbschemas = rsp.schemas;
          await->set_value(error);
        },
        300000
      );
      if((err = res.get_future().get()))
        Converter::exception(err);
    }

    size_t sz = dbschemas.size();
    Core::Semaphore sem(sz, sz);
    _return.resize(sz);
    for(size_t idx = 0; idx < sz; ++idx) {
      auto& r = _return[idx];
      Comm::Protocol::Mngr::Req::ColumnCompact::request(
        (r.cid = dbschemas[idx]->cid),
        [&sem, err=&r.err]
        (const Comm::client::ConnQueue::ReqBase::Ptr&,
         const Comm::Protocol::Mngr::Params::ColumnCompactRsp& rsp) {
          *err = rsp.err;
          sem.release();
        },
        300000
      );
    }
    sem.wait_all();
  }

  static void process_results(
          int& err, const client::Query::Select::Result::Ptr& result,
          Cells& _return) {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Result cells;

    for(cid_t cid : result->get_cids()) {
      cells.free();
      result->get_cells(cid, cells);

      schema = Env::Clients::get()->schemas->get(err, cid);
      if(err)
        return;
      switch(schema->col_type) {
        case DB::Types::Column::SERIAL: {
          auto& rcells = _return.serial_cells;
          size_t c = rcells.size();
          rcells.resize(c + cells.size());
          for(auto dbcell : cells) {
            auto& cell = rcells[c++];
            cell.c = schema->col_name;
            dbcell->key.convert_to(cell.k);
            cell.ts = dbcell->timestamp;
            if(dbcell->vlen)
              Converter::set(*dbcell, cell.v);
          }
          break;
        }
        default: {
          auto& rcells = _return.cells;
          size_t c = rcells.size();
          rcells.resize(c + cells.size());
          for(auto dbcell : cells) {
            auto& cell = rcells[c++];
            cell.c = schema->col_name;
            dbcell->key.convert_to(cell.k);
            cell.ts = dbcell->timestamp;
            if(dbcell->vlen) {
              StaticBuffer v;
              dbcell->get_value(v);
              cell.v.append((const char*)v.base, v.size);
            }
          }
        }
      }
    }
  }

  static void process_results(
          int& err, const client::Query::Select::Result::Ptr& result,
          CCells& _return) {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Result cells;

    for(cid_t cid : result->get_cids()) {
      cells.free();
      result->get_cells(cid, cells);

      schema = Env::Clients::get()->schemas->get(err, cid);
      if(err)
        return;
      auto& _col = _return[schema->col_name];

      switch(schema->col_type) {
        case DB::Types::Column::SERIAL: {
          auto& rcells = _col.serial_cells;
          size_t c = rcells.size();
          rcells.resize(c + cells.size());
          for(auto dbcell : cells) {
            auto& cell = rcells[c++];
            dbcell->key.convert_to(cell.k);
            cell.ts = dbcell->timestamp;
            if(dbcell->vlen)
              Converter::set(*dbcell, cell.v);
          }
          break;
        }
        default: {
          auto& rcells = _col.cells;
          size_t c = rcells.size();
          rcells.resize(c + cells.size());
          for(auto dbcell : cells) {
            auto& cell = rcells[c++];
            dbcell->key.convert_to(cell.k);
            cell.ts = dbcell->timestamp;
            if(dbcell->vlen) {
              StaticBuffer v;
              dbcell->get_value(v);
              cell.v.append((const char*)v.base, v.size);
            }
          }
        }
      }
    }
  }

  static void process_results(
          int& err, const client::Query::Select::Result::Ptr& result,
          KCells& _return) {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Result cells;

    for(cid_t cid : result->get_cids()) {
      cells.free();
      result->get_cells(cid, cells);

      schema = Env::Clients::get()->schemas->get(err, cid);
      if(err)
        return;
      switch(schema->col_type) {
        case DB::Types::Column::SERIAL: {
          for(auto dbcell : cells) {
            auto it = std::find_if(_return.begin(), _return.end(),
                              [dbcell](const kCells& key_cells)
                              {return dbcell->key.equal(key_cells.k);});
            if(it == _return.end()) {
              _return.emplace_back();
              it = _return.end()-1;
              dbcell->key.convert_to(it->k);
            }

            auto& cell = it->serial_cells.emplace_back();
            cell.c = schema->col_name;
            cell.ts = dbcell->timestamp;
            if(dbcell->vlen)
              Converter::set(*dbcell, cell.v);
          }
          break;
        }
        default: {
          for(auto dbcell : cells) {
            auto it = std::find_if(_return.begin(), _return.end(),
                              [dbcell](const kCells& key_cells)
                              {return dbcell->key.equal(key_cells.k);});
            if(it == _return.end()) {
              _return.emplace_back();
              it = _return.end()-1;
              dbcell->key.convert_to(it->k);
            }

            auto& cell = it->cells.emplace_back();
            cell.c = schema->col_name;
            cell.ts = dbcell->timestamp;
            if(dbcell->vlen) {
              StaticBuffer v;
              dbcell->get_value(v);
              cell.v.append((const char*)v.base, v.size);
            }
          }
        }
      }
    }
  }

  static void process_results(
          int& err, const client::Query::Select::Result::Ptr& result,
          FCells& _return) {
    DB::Schema::Ptr schema = 0;
    DB::Cells::Result cells;

    std::vector<std::string> key;

    for(cid_t cid : result->get_cids()) {
      cells.free();
      result->get_cells(cid, cells);

      schema = Env::Clients::get()->schemas->get(err, cid);
      if(err)
        return;
      switch(schema->col_type) {
        case DB::Types::Column::SERIAL: {
          FCells* fraction_cells;
          for(auto dbcell : cells) {
            fraction_cells = &_return;
            key.clear();
            dbcell->key.convert_to(key);
            for(auto& f : key)
              fraction_cells = &fraction_cells->f[f];
            auto& cell = fraction_cells->serial_cells.emplace_back();
            cell.c = schema->col_name;
            cell.ts = dbcell->timestamp;
            if(dbcell->vlen)
              Converter::set(*dbcell, cell.v);
          }
          break;
        }
        default: {
          FCells* fraction_cells;
          for(auto dbcell : cells) {
            fraction_cells = &_return;
            key.clear();
            dbcell->key.convert_to(key);
            for(auto& f : key)
              fraction_cells = &fraction_cells->f[f];
            auto& cell = fraction_cells->cells.emplace_back();
            cell.c = schema->col_name;
            cell.ts = dbcell->timestamp;
            if(dbcell->vlen) {
              StaticBuffer v;
              dbcell->get_value(v);
              cell.v.append((const char*)v.base, v.size);
            }
          }
        }
      }
    }
  }

  std::mutex m_mutex;
  std::unordered_map<
    int64_t, client::Query::Update::Ptr> m_updaters;
};




}}

#endif // swcdb_app_thriftbroker_AppHandler_h