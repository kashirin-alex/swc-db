/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_RangeSplit_h
#define swcdb_ranger_db_RangeSplit_h


namespace SWC { namespace Ranger {

  
class RangeSplit final {
  public:
  

  RangeSplit(const RangePtr& range, const size_t split_at) 
            : range(range), split_at(split_at) {
    SWC_LOGF(LOG_INFO, "COMPACT-SPLIT RANGE START %lu/%lu at=%lu", 
             range->cfg->cid, range->rid, split_at);
  }

  RangeSplit(const RangeSplit&) = delete;

  RangeSplit(const RangeSplit&&) = delete;
  
  RangeSplit& operator=(const RangeSplit&) = delete;

  ~RangeSplit() { }

  int run() {
    int64_t ts = Time::now_ns();
    int err = Error::OK;

    rid_t new_rid = 0;
    mngr_create_range(err, new_rid);
    if((err && err != Error::COLUMN_NOT_READY) || !new_rid) 
      return err;
      
    ColumnPtr col = Env::Rgr::columns()->get_column(range->cfg->cid);
    if(!col || col->removing())
      return Error::CANCELLED;

    SWC_LOGF(LOG_INFO, "COMPACT-SPLIT RANGE %lu/%lu new-rid=%lu", 
             range->cfg->cid, range->rid, new_rid);

    auto new_range = col->internal_create(err, new_rid, true);
    if(!err)
      new_range->internal_create_folders(err);

    if(err) {
      SWC_LOGF(LOG_INFO, "COMPACT-SPLIT RANGE cancelled err=%d %lu/%lu new-rid=%lu", 
                err, range->cfg->cid, range->rid, new_rid);
      int tmperr = Error::OK;
      new_range->compacting(Range::COMPACT_NONE);
      col->internal_remove(tmperr, new_rid);
      mngr_remove_range(new_range);
      return err;
    }

    CellStore::Readers::Vec mv_css;
    
    auto it = range->blocks.cellstores.begin() + split_at;
    mv_css.assign(it, range->blocks.cellstores.end());

    new_range->internal_create(err, mv_css);
    if(!err) {
      for(; it<range->blocks.cellstores.end(); ) {
        delete *it;
        range->blocks.cellstores.erase(it);
      }
    } else {
      int tmperr = Error::OK;
      new_range->compacting(Range::COMPACT_NONE);
      col->internal_remove(tmperr, new_rid);
      mngr_remove_range(new_range);
      return err;
    }
    
    if(range->blocks.commitlog.cells_count()) {
      CommitLog::Fragments::Vec fragments_old;
      range->blocks.commitlog.get(fragments_old); // fragments for removal

      CommitLog::Splitter splitter(
        range->blocks.cellstores.back()->interval.key_end,
        fragments_old,
        &range->blocks.commitlog,
        &new_range->blocks.commitlog
      );

      splitter.run();
      range->blocks.commitlog.remove(err, fragments_old);

      range->blocks.commitlog.commit_new_fragment(true);
      new_range->blocks.commitlog.commit_new_fragment(true);
    }

    std::promise<void>  r_promise;
    new_range->expand_and_align(false,
      [this, col, new_range, await=&r_promise]
      (const client::Query::Update::Result::Ptr&) {
        SWC_LOGF(LOG_INFO,
          "COMPACT-SPLIT RANGE %lu/%lu unloading new-rid=%lu",
          col->cfg->cid, range->rid, new_range->rid);
        new_range->compacting(Range::COMPACT_NONE);
        col->internal_unload(new_range->rid);
        Comm::Protocol::Mngr::Req::RangeUnloaded::request(
          col->cfg->cid, new_range->rid,
          [cid=col->cfg->cid, new_rid=new_range->rid]
          (const Comm::client::ConnQueue::ReqBase::Ptr& req,
           const Comm::Protocol::Mngr::Params::RangeUnloadedRsp& rsp) {
            SWC_LOGF(LOG_DEBUG,
              "RangeSplit::Mngr::Req::RangeUnloaded err=%d(%s) %lu/%lu",
              rsp.err, Error::get_text(rsp.err), cid, new_rid);
            if(rsp.err && !Env::Rgr::is_not_accepting() &&
               rsp.err != Error::COLUMN_NOT_EXISTS &&
               rsp.err != Error::COLUMN_MARKED_REMOVED &&
               rsp.err != Error::COLUMN_NOT_READY) {
              req->request_again();
            }
        });

        range->expand_and_align(true,
        [await] (const client::Query::Update::Result::Ptr&) {
          await->set_value();
        });
    });
    new_range = nullptr;
    r_promise.get_future().wait();

    SWC_LOG_OUT(LOG_INFO, 
      SWC_LOG_PRINTF("COMPACT-SPLITTED RANGE %lu/%lu took=%ldns new-end=",
                      range->cfg->cid, range->rid, Time::now_ns() - ts);
      range->blocks.cellstores.back()->interval.key_end.print(SWC_LOG_OSTREAM);
    );
    return Error::OK;
  }

  void mngr_create_range(int& err, rid_t& new_rid) {
    std::promise<void>  res;
    Comm::Protocol::Mngr::Req::RangeCreate::request(
      range->cfg->cid,
      Env::Rgr::rgr_data()->rgrid,
      [&] (const Comm::client::ConnQueue::ReqBase::Ptr& req, 
           const Comm::Protocol::Mngr::Params::RangeCreateRsp& rsp) {
      
        SWC_LOGF(LOG_DEBUG, 
          "RangeSplit::Mngr::Req::RangeCreate err=%d(%s) %lu/%lu", 
          rsp.err, Error::get_text(rsp.err), range->cfg->cid, rsp.rid);

        if(rsp.err && 
           rsp.err != Error::COLUMN_NOT_EXISTS &&
           rsp.err != Error::COLUMN_MARKED_REMOVED &&
           rsp.err != Error::COLUMN_NOT_READY) {
          req->request_again();
          return;
        }
        err = rsp.err;
        new_rid = rsp.rid;
        res.set_value();
      }
    );
    res.get_future().wait();
  }
  
  void mngr_remove_range(const RangePtr& new_range) {
    std::promise<void> res;
    Comm::Protocol::Mngr::Req::RangeRemove::request(
      new_range->cfg->cid,
      new_range->rid,
      [&] (const Comm::client::ConnQueue::ReqBase::Ptr& req, 
           const Comm::Protocol::Mngr::Params::RangeRemoveRsp& rsp) {
      
        SWC_LOGF(LOG_DEBUG, 
          "RangeSplit::Mngr::Req::RangeRemove err=%d(%s) %lu/%lu", 
          rsp.err, Error::get_text(rsp.err), 
          new_range->cfg->cid, new_range->rid);
      
        if(rsp.err && 
           rsp.err != Error::COLUMN_NOT_EXISTS &&
           rsp.err != Error::COLUMN_MARKED_REMOVED &&
           rsp.err != Error::COLUMN_NOT_READY) {
           req->request_again();
        } else {
          res.set_value();
        }
      }
    );
    res.get_future().get();
  }

  const RangePtr range;
  const size_t   split_at;
};


}} // namespace SWC::Ranger

#endif // swcdb_ranger_db_RangeSplit_h
