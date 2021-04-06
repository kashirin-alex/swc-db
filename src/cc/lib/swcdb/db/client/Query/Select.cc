/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/db/Types/MetaColumn.h"
#include "swcdb/db/client/Clients.h"
#include "swcdb/db/client/Query/Select.h"


namespace SWC { namespace client { namespace Query { namespace Select {



void scan(const Handlers::Base::Ptr& hdlr,
          DB::Types::KeySeq key_seq,
          cid_t cid,
          const DB::Specs::Interval& intval) {
  hdlr->completion.increment();
  std::make_shared<Scanner>(
    hdlr, key_seq, intval, cid
  )->mngr_locate_master();
}

void scan(const Handlers::Base::Ptr& hdlr,
          DB::Types::KeySeq key_seq,
          cid_t cid,
          DB::Specs::Interval&& intval) {
  hdlr->completion.increment();
  std::make_shared<Scanner>(
    hdlr, key_seq, std::move(intval), cid
  )->mngr_locate_master();
}

void scan(const Handlers::Base::Ptr& hdlr,
          const DB::Schema::Ptr& schema,
          const DB::Specs::Interval& intval) {
  scan(hdlr, schema->col_seq, schema->cid, intval);
}

void scan(const Handlers::Base::Ptr& hdlr,
          const DB::Schema::Ptr& schema,
          DB::Specs::Interval&& intval) {
  scan(hdlr, schema->col_seq, schema->cid, std::move(intval));
}

void scan(int& err,
          const Handlers::BaseUnorderedMap::Ptr& hdlr,
          const DB::Specs::Scan& specs) {
  std::vector<DB::Schema::Ptr> schemas(specs.columns.size());
  auto it_seq = schemas.begin();
  for(auto& col : specs.columns) {
    auto schema = Env::Clients::get()->schemas->get(err, col->cid);
    if(err)
      return;
    *it_seq++ = schema;
    hdlr->completion.increment();
  }

  it_seq = schemas.begin();
  for(auto& col : specs.columns) {
    for(auto& intval : col->intervals) {
      if(!intval->flags.max_buffer)
        intval->flags.max_buffer = hdlr->buff_sz;
      if(!intval->values.empty())
        intval->values.col_type = (*it_seq)->col_type;
      std::make_shared<Scanner>(
        hdlr, (*it_seq)->col_seq, *intval.get(), col->cid
      )->mngr_locate_master();
    }
    ++it_seq;
  }
}

void scan(int& err,
          const Handlers::BaseUnorderedMap::Ptr& hdlr,
          DB::Specs::Scan&& specs) {
  std::vector<DB::Schema::Ptr> schemas(specs.columns.size());
  auto it_seq = schemas.begin();
  for(auto& col : specs.columns) {
    auto schema = Env::Clients::get()->schemas->get(err, col->cid);
    if(err)
      return;
    *it_seq++ = schema;
    hdlr->completion.increment();
  }

  it_seq = schemas.begin();
  for(auto& col : specs.columns) {
    for(auto& intval : col->intervals) {
      if(!intval->flags.max_buffer)
        intval->flags.max_buffer = hdlr->buff_sz;
      if(!intval->values.empty())
        intval->values.col_type = (*it_seq)->col_type;
      std::make_shared<Scanner>(
        hdlr, (*it_seq)->col_seq, std::move(*intval.get()), col->cid
      )->mngr_locate_master();
    }
    ++it_seq;
  }
}






#define SWC_SCANNER_REQ_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, params.print(SWC_LOG_OSTREAM << msg << ' '); );

#define SWC_SCANNER_RSP_DEBUG(msg) \
  SWC_LOG_OUT(LOG_DEBUG, \
    print(SWC_LOG_OSTREAM << msg << ' '); \
    rsp.print(SWC_LOG_OSTREAM << ' '); \
  );


Scanner::Scanner(const Handlers::Base::Ptr& hdlr,
                 const DB::Types::KeySeq col_seq,
                 const DB::Specs::Interval& interval,
                 const cid_t cid)
            : completion(0),
              selector(hdlr),
              col_seq(col_seq),
              interval(interval),
              master_cid(DB::Types::MetaColumn::get_master_cid(col_seq)),
              meta_cid(DB::Types::MetaColumn::get_meta_cid(col_seq)),
              data_cid(cid),
              master_rid(0),
              meta_rid(0),
              data_rid(0),
              master_mngr_next(false),
              master_rgr_next(false),
              meta_next(false) {
}

Scanner::Scanner(const Handlers::Base::Ptr& hdlr,
                 const DB::Types::KeySeq col_seq,
                 DB::Specs::Interval&& interval,
                 const cid_t cid) noexcept
            : completion(0),
              selector(hdlr),
              col_seq(col_seq),
              interval(std::move(interval)),
              master_cid(DB::Types::MetaColumn::get_master_cid(col_seq)),
              meta_cid(DB::Types::MetaColumn::get_meta_cid(col_seq)),
              data_cid(cid),
              master_rid(0),
              meta_rid(0),
              data_rid(0),
              master_mngr_next(false),
              master_rgr_next(false),
              meta_next(false) {
}

void Scanner::debug_res_cache(const char* msg, cid_t cid, rid_t rid,
                              const Comm::EndPoints& endpoints) {
  SWC_LOG_OUT(LOG_DEBUG,
    print(SWC_LOG_OSTREAM << msg << ' ');
    SWC_LOG_OSTREAM
      << "Ranger(cid=" << cid << " rid=" << rid << " endpoints=[";
    for(auto& endpoint : endpoints)
      SWC_LOG_OSTREAM << endpoint << ',';
    SWC_LOG_OSTREAM << "])";
  );
}

void Scanner::print(std::ostream& out) {
  out << "Scanner(" << DB::Types::to_string(col_seq)
      << " master(" << master_cid << '/' << master_rid
        << " mngr-next=" << master_mngr_next
        << " rgr-next=" << master_rgr_next << ')'
      << " meta(" << meta_cid << '/' << meta_rid
        << " next=" << meta_next << ')'
      << " data(" << data_cid << '/' << data_rid  << ") ";
  interval.print(out);
  out << " completion=" << completion.count() << ')';
}

bool Scanner::add_cells(const StaticBuffer& buffer, bool reached_limit) {
  return selector->add_cells(data_cid, buffer, reached_limit, interval);
}

void Scanner::response_if_last() {
  if(completion.is_last()) {
    master_rgr_req_base = nullptr;
    meta_req_base = nullptr;
    data_req_base = nullptr;
    if(selector->completion.is_last())
      selector->response(Error::OK);
  }
}

void Scanner::next_call() {
  if(meta_next) {
    if(!interval.offset_key.empty() && !meta_end.empty()) {
      if(DB::KeySeq::compare(col_seq, interval.offset_key, meta_end)
         != Condition::LT) {
        interval.offset_key.free();
        interval.offset_rev = 0;
      }
    }
    rgr_locate_meta();

  } else if(master_rgr_next) {
    rgr_locate_master();

  } else if(master_mngr_next) {
    mngr_locate_master();
  }
}


//
void Scanner::mngr_locate_master() {
  completion.increment();

  Comm::Protocol::Mngr::Params::RgrGetReq params(
    master_cid, 0, master_mngr_next);

  if(master_mngr_next) {
    params.range_begin.copy(master_mngr_offset);
  }
  /*** MASTER columns need to have aligned MetaData interval
  if(master_mngr_offset.empty()) {
    interval.apply_possible_range(
      params.range_begin, params.range_end);
  } else {
    params.range_begin.copy(master_mngr_offset);
    interval.apply_possible_range_end(params.range_end);
  }
  ***/

  if(DB::Types::MetaColumn::is_data(data_cid)) {
    auto data_cid_str = std::to_string(data_cid);
    params.range_begin.insert(0, data_cid_str);
    params.range_end.insert(0, data_cid_str);
  }
  if(!DB::Types::MetaColumn::is_master(data_cid)) {
    auto meta_cid_str = DB::Types::MetaColumn::get_meta_cid_str(col_seq);
    params.range_begin.insert(0, meta_cid_str);
    params.range_end.insert(0, meta_cid_str);
  }

  SWC_SCANNER_REQ_DEBUG("mngr_locate_master");
  Comm::Protocol::Mngr::Req::RgrGet::request(
    params,
    [profile=selector->profile.mngr_locate(),
     scanner=shared_from_this()]
    (const ReqBase::Ptr& req,
     const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid);
      if(scanner->mngr_located_master(req, rsp))
        scanner->response_if_last();
    }
  );
}

bool Scanner::mngr_located_master(
        const ReqBase::Ptr& req,
        const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  switch(rsp.err) {
    case Error::OK: {
      if(!rsp.rid) {
        SWC_SCANNER_RSP_DEBUG("mngr_located_master RETRYING(no rid)");
        return !selector->valid(req);
      }
      if(master_cid != rsp.cid) {
        SWC_SCANNER_RSP_DEBUG("mngr_located_master RETRYING(cid no match)");
        return !selector->valid(req);
      }

      SWC_SCANNER_RSP_DEBUG("mngr_located_master");
      Env::Clients::get()->rangers.set(rsp.cid, rsp.rid, rsp.endpoints);
      master_mngr_next = true;
      master_mngr_offset.copy(rsp.range_begin);
      if(DB::Types::MetaColumn::is_master(data_cid)) {
        data_rid = rsp.rid;
        data_req_base = req;
        data_endpoints = rsp.endpoints;
        rgr_select();
      } else {
        master_rid = rsp.rid;
        master_rgr_req_base = req;
        master_rgr_endpoints = rsp.endpoints;
        rgr_locate_master();
      }
      return true;
    }
    case Error::RANGE_NOT_FOUND: {
      if(master_mngr_next) {
        master_mngr_next = false;
        SWC_SCANNER_RSP_DEBUG("mngr_located_master finished");
        return true;
      }
      [[fallthrough]];
    }
    default: {
      SWC_SCANNER_RSP_DEBUG("mngr_located_master RETRYING");
      return !selector->valid(req);
    }
  }
}


//
void Scanner::rgr_locate_master() {
  completion.increment();

  Comm::Protocol::Rgr::Params::RangeLocateReq params(master_cid, master_rid);
  auto data_cid_str = std::to_string(data_cid);
  if(master_rgr_next) {
    params.flags |= Comm::Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE;
    params.range_offset.copy(master_rgr_offset);
    params.range_offset.insert(0, data_cid_str);
  }

  /*** MASTER columns need to have aligned MetaData interval
  bool range_end_rest = false;
  interval.apply_possible_range(
    params.range_begin, params.range_end, &range_end_rest);
  if(range_end_rest)
    params.flags
      |= Comm::Protocol::Rgr::Params::RangeLocateReq::RANGE_END_REST;
  if(interval.has_opt__key_equal())
    params.flags
      |= Comm::Protocol::Rgr::Params::RangeLocateReq::KEY_EQUAL;
  ***/

  params.range_begin.insert(0, data_cid_str);
  params.range_end.insert(0, data_cid_str);
  if(DB::Types::MetaColumn::is_data(data_cid)) {
    auto meta_cid_str = DB::Types::MetaColumn::get_meta_cid_str(col_seq);
    params.range_begin.insert(0, meta_cid_str);
    params.range_end.insert(0, meta_cid_str);
    if(master_rgr_next)
      params.range_offset.insert(0, meta_cid_str);
  }

  SWC_SCANNER_REQ_DEBUG("rgr_locate_master");
  Comm::Protocol::Rgr::Req::RangeLocate::request(
    params, master_rgr_endpoints,
    [profile=selector->profile.rgr_locate(DB::Types::Range::MASTER),
     scanner=shared_from_this()]
    (const ReqBase::Ptr& req,
     const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
      profile.add(!rsp.rid || rsp.err);
      scanner->rgr_located_master(req, rsp);
    }
  );
}

void Scanner::rgr_located_master(
          const ReqBase::Ptr& req,
          const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
  switch(rsp.err) {
    case Error::OK: {
      if(!rsp.rid) { // sake check (must be an err rsp)
        SWC_SCANNER_RSP_DEBUG("rgr_located_master RETRYING(no rid)");
        Env::Clients::get()->rangers.remove(master_cid, master_rid);
        if(selector->valid(master_rgr_req_base))
          return;
        break;
      }
      if(meta_cid != rsp.cid) { // sake check (must be an err rsp)
        SWC_SCANNER_RSP_DEBUG("rgr_located_master RETRYING(cid no match)");
        Env::Clients::get()->rangers.remove(master_cid, master_rid);
        if(selector->valid(master_rgr_req_base))
          return;
        break;
      }
      SWC_SCANNER_RSP_DEBUG("rgr_located_master");
      master_rgr_next = true;
      master_rgr_offset.copy(rsp.range_begin);
      if(DB::Types::MetaColumn::is_meta(data_cid)) {
        data_rid = rsp.rid;
        data_req_base = req;
        mngr_resolve_rgr_select();
      } else {
        meta_rid = rsp.rid;
        meta_req_base = req;
        mngr_resolve_rgr_meta();
      }
      break;
    }
    case Error::RANGE_NOT_FOUND: {
      if(master_rgr_next) {
        master_rgr_next = false;
        SWC_SCANNER_RSP_DEBUG("rgr_located_master master_rgr_next");
        next_call();
        break;
      }
      [[fallthrough]];
    }
    case Error::RGR_NOT_LOADED_RANGE:
    case Error::SERVER_SHUTTING_DOWN:
    case Error::COMM_NOT_CONNECTED: {
      SWC_SCANNER_RSP_DEBUG("rgr_located_master RETRYING");
      Env::Clients::get()->rangers.remove(master_cid, master_rid);
      if(selector->valid(master_rgr_req_base))
        return;
      break;
    }
    default: {
      if(selector->valid(req))
        return;
      break;
    }
  }
  response_if_last();
}


//
void Scanner::mngr_resolve_rgr_meta() {
  completion.increment();

  auto profile = selector->profile.mngr_res();

  if(Env::Clients::get()->rangers.get(meta_cid, meta_rid, meta_endpoints)) {
    profile.add_cached(Error::OK);
    debug_res_cache("mngr_resolved_rgr_meta Cache hit",
                    meta_cid, meta_rid, meta_endpoints);
    rgr_locate_meta();
    return response_if_last();
  }

  Comm::Protocol::Mngr::Params::RgrGetReq params(meta_cid, meta_rid);
  SWC_SCANNER_REQ_DEBUG("mngr_resolve_rgr_meta");
  Comm::Protocol::Mngr::Req::RgrGet::request(
    params,
    [profile, scanner=shared_from_this()]
    (const ReqBase::Ptr& req,
     const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid || rsp.endpoints.empty());
      if(scanner->mngr_resolved_rgr_meta(req, rsp))
        scanner->response_if_last();
    }
  );
}

bool Scanner::mngr_resolved_rgr_meta(
        const ReqBase::Ptr& req,
        const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  switch(rsp.err) {
    case Error::OK: {
      SWC_SCANNER_RSP_DEBUG("mngr_resolved_rgr_meta");
      meta_endpoints = rsp.endpoints;
      Env::Clients::get()->rangers.set(meta_cid, meta_rid, rsp.endpoints);
      rgr_locate_meta();
      return true;
    }
    case Error::RANGE_NOT_FOUND: {
      SWC_SCANNER_RSP_DEBUG("mngr_resolved_rgr_meta RETRYING");
      return !selector->valid(meta_req_base);
    }
    default: {
      SWC_SCANNER_RSP_DEBUG("mngr_resolved_rgr_meta RETRYING");
      return !selector->valid(req);
    }
  }
}


//
void Scanner::rgr_locate_meta() {
  completion.increment();

  Comm::Protocol::Rgr::Params::RangeLocateReq params(meta_cid, meta_rid);
  auto data_cid_str = std::to_string(data_cid);
  if(meta_next) {
    params.flags |= Comm::Protocol::Rgr::Params::RangeLocateReq::NEXT_RANGE;
    params.range_offset.copy(meta_offset);
    params.range_offset.insert(0, data_cid_str);
  }

  bool range_end_rest = false;
  interval.apply_possible_range(
    params.range_begin, params.range_end, &range_end_rest);
  if(range_end_rest ||
     (!interval.range_end.empty() && interval.has_opt__range_end_rest()))
    params.flags
      |= Comm::Protocol::Rgr::Params::RangeLocateReq::RANGE_END_REST;
  if(interval.has_opt__key_equal())
    params.flags
      |= Comm::Protocol::Rgr::Params::RangeLocateReq::KEY_EQUAL;

  params.range_begin.insert(0, data_cid_str);
  params.range_end.insert(0, data_cid_str);

  SWC_SCANNER_REQ_DEBUG("rgr_locate_meta");
  Comm::Protocol::Rgr::Req::RangeLocate::request(
    params, meta_endpoints,
    [profile=selector->profile.rgr_locate(DB::Types::Range::META),
     scanner=shared_from_this()]
    (const ReqBase::Ptr& req,
     const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
      profile.add(!rsp.rid || rsp.err);
      scanner->rgr_located_meta(req, rsp);
    }
  );
}

void Scanner::rgr_located_meta(
          const ReqBase::Ptr& req,
          const Comm::Protocol::Rgr::Params::RangeLocateRsp& rsp) {
  switch(rsp.err) {
    case Error::OK: {
      if(!rsp.rid) { // sake check (must be an err rsp)
        SWC_SCANNER_RSP_DEBUG("rgr_located_meta RETRYING(no rid)");
        Env::Clients::get()->rangers.remove(meta_cid, meta_rid);
        if(selector->valid(meta_req_base))
          return;
        break;
      }
      if(data_cid != rsp.cid) { // sake check (must be an err rsp)
        SWC_SCANNER_RSP_DEBUG("rgr_located_meta RETRYING(cid no match)");
        Env::Clients::get()->rangers.remove(meta_cid, meta_rid);
        if(selector->valid(meta_req_base))
          return;
        break;
      }
      SWC_SCANNER_RSP_DEBUG("rgr_located_meta");
      meta_next = true;
      meta_offset.copy(rsp.range_begin);
      meta_end.copy(rsp.range_end);
      data_rid = rsp.rid;
      data_req_base = req;
      mngr_resolve_rgr_select();
      break;
    }
    case Error::RANGE_NOT_FOUND: {
      meta_next = false;
      SWC_SCANNER_RSP_DEBUG("rgr_located_meta meta_next");
      next_call();
      break;
    }
    case Error::RGR_NOT_LOADED_RANGE:
    case Error::COMM_NOT_CONNECTED:
    case Error::SERVER_SHUTTING_DOWN: {
      SWC_SCANNER_RSP_DEBUG("rgr_located_meta RETRYING");
      Env::Clients::get()->rangers.remove(meta_cid, meta_rid);
      if(selector->valid(meta_req_base))
        return;
      break;
    }
    default: {
      SWC_SCANNER_RSP_DEBUG("rgr_located_meta RETRYING-locate");
      if(selector->valid(req))
        return;
      break;
    }
  }
  response_if_last();
}


//
void Scanner::mngr_resolve_rgr_select() {
  completion.increment();

  auto profile = selector->profile.mngr_res();

  if(Env::Clients::get()->rangers.get(data_cid, data_rid, data_endpoints)) {
    profile.add_cached(Error::OK);
    debug_res_cache("mngr_resolved_rgr_select Cache hit",
                    data_cid, data_rid, data_endpoints);
    rgr_select();
    return response_if_last();
  }

  Comm::Protocol::Mngr::Params::RgrGetReq params(data_cid, data_rid);
  SWC_SCANNER_REQ_DEBUG("mngr_resolve_rgr_select");
  Comm::Protocol::Mngr::Req::RgrGet::request(
    params,
    [profile, scanner=shared_from_this()]
    (const ReqBase::Ptr& req,
     const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
      profile.add(rsp.err || !rsp.rid || rsp.endpoints.empty());
      if(scanner->mngr_resolved_rgr_select(req, rsp))
        scanner->response_if_last();
    }
  );
}

bool Scanner::mngr_resolved_rgr_select(
        const ReqBase::Ptr& req,
        const Comm::Protocol::Mngr::Params::RgrGetRsp& rsp) {
  switch(rsp.err) {
    case Error::OK: {
      SWC_SCANNER_RSP_DEBUG("mngr_resolved_rgr_select");
      data_endpoints = rsp.endpoints;
      Env::Clients::get()->rangers.set(data_cid, data_rid, rsp.endpoints);
      rgr_select();
      return true;
    }
    case Error::COLUMN_NOT_EXISTS: {
      SWC_SCANNER_RSP_DEBUG("mngr_resolved_rgr_select QUIT");
      int at = Error::OK; // rsp.err = Error::CONSIST_ERRORS;
      selector->state_error.compare_exchange_weak(at, rsp.err);
      selector->error(data_cid, rsp.err);
      return true;
    }
    case Error::RANGE_NOT_FOUND: {
      SWC_SCANNER_RSP_DEBUG("mngr_resolved_rgr_select RETRYING");
      return !selector->valid(data_req_base);
    }
    default: {
      SWC_SCANNER_RSP_DEBUG("mngr_resolved_rgr_select RETRYING");
      return !selector->valid(req);
    }
  }
}


//
void Scanner::rgr_select() {
  completion.increment();

  Comm::Protocol::Rgr::Params::RangeQuerySelectReq params(
    data_cid, data_rid, interval);

  SWC_SCANNER_REQ_DEBUG("rgr_select");
  Comm::Protocol::Rgr::Req::RangeQuerySelect::request(
    params, data_endpoints,
    [profile=selector->profile.rgr_data(), scanner=shared_from_this()]
    (const ReqBase::Ptr& req,
     const Comm::Protocol::Rgr::Params::RangeQuerySelectRsp& rsp) {
      profile.add(rsp.err);
      scanner->rgr_selected(req, rsp);
    },
    selector->timeout
  );
}

void Scanner::rgr_selected(
                const ReqBase::Ptr& req,
                const Comm::Protocol::Rgr::Params::RangeQuerySelectRsp& rsp) {
  switch(rsp.err) {
    case Error::OK: {
      SWC_SCANNER_RSP_DEBUG("rgr_selected");
      if(interval.flags.offset)
        interval.flags.offset = rsp.offset;

      if(!rsp.data.size || add_cells(rsp.data, rsp.reached_limit))
        rsp.reached_limit ? rgr_select() : next_call();
      break;
    }
    case Error::RGR_NOT_LOADED_RANGE:
    case Error::SERVER_SHUTTING_DOWN:
    case Error::COMM_NOT_CONNECTED: {
      SWC_SCANNER_RSP_DEBUG("rgr_selected RETRYING");
      Env::Clients::get()->rangers.remove(data_cid, data_rid);
      if(selector->valid(data_req_base))
        return;
      break;
    }
    default: {
      if(selector->valid(req))
        return;
    }
  }
  response_if_last();
}


#undef SWC_SCANNER_REQ_DEBUG
#undef SWC_SCANNER_RSP_DEBUG

}}}}
