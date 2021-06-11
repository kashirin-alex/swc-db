/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_db_RangeSplit_h
#define swcdb_ranger_db_RangeSplit_h


namespace SWC { namespace Ranger {


static void mngr_remove_range(const RangePtr& new_range) {
  std::promise<void> res;
  struct ReqData {
    const RangePtr&     new_range;
    std::promise<void>& res;
    SWC_CAN_INLINE
    ReqData(const RangePtr& new_range, std::promise<void>& res)
            noexcept : new_range(new_range), res(res) {
    }
    SWC_CAN_INLINE
    cid_t get_cid() const noexcept {
      return new_range->cfg->cid;
    }
    SWC_CAN_INLINE
    client::Clients::Ptr& get_clients() noexcept {
      return Env::Clients::get();
    }
    SWC_CAN_INLINE
    bool valid() noexcept {
      return true;
    }
    SWC_CAN_INLINE
    void callback(
        const Comm::client::ConnQueue::ReqBase::Ptr& req,
        const Comm::Protocol::Mngr::Params::RangeRemoveRsp& rsp) {

      SWC_LOGF(LOG_DEBUG,
        "Mngr::Req::RangeRemove err=%d(%s) %lu/%lu",
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
  };
  Comm::Protocol::Mngr::Req::RangeRemove<ReqData>::request(
    Comm::Protocol::Mngr::Params::RangeRemoveReq(
      new_range->cfg->cid, new_range->rid),
    10000,
    new_range, res
  );
  res.get_future().wait();
}



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

  //~RangeSplit() { }

  int run() {
    Time::Measure_ns t_measure;
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
      for(; it != range->blocks.cellstores.end(); ) {
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
    new_range->expand_and_align(false, Query::Update::CommonMeta::make(
      new_range,
      [this, col, await=&r_promise]
      (const Query::Update::CommonMeta::Ptr& hdlr) {
        SWC_LOGF(LOG_INFO,
          "COMPACT-SPLIT RANGE %lu/%lu unloading new-rid=%lu reg-err=%d(%s)",
            col->cfg->cid, range->rid, hdlr->range->rid,
            hdlr->error(), Error::get_text(hdlr->error()));
        hdlr->range->compacting(Range::COMPACT_NONE);
        col->internal_unload(hdlr->range->rid);

        struct ReqData {
          const cid_t cid;
          const rid_t new_rid;
          SWC_CAN_INLINE
          ReqData(cid_t cid, rid_t new_rid) noexcept
                  : cid(cid), new_rid(new_rid) {
          }
          SWC_CAN_INLINE
          cid_t get_cid() const noexcept {
            return cid;
          }
          SWC_CAN_INLINE
          client::Clients::Ptr& get_clients() noexcept {
            return Env::Clients::get();
          }
          SWC_CAN_INLINE
          bool valid() noexcept {
            return !Env::Rgr::is_not_accepting();
          }
          SWC_CAN_INLINE
          void callback(
              const Comm::client::ConnQueue::ReqBase::Ptr& req,
              const Comm::Protocol::Mngr::Params::RangeUnloadedRsp& rsp) {
            SWC_LOGF(LOG_DEBUG,
              "RangeSplit::Mngr::Req::RangeUnloaded err=%d(%s) %lu/%lu",
              rsp.err, Error::get_text(rsp.err), cid, new_rid);
            if(rsp.err && valid() &&
               rsp.err != Error::CLIENT_STOPPING &&
               rsp.err != Error::COLUMN_NOT_EXISTS &&
               rsp.err != Error::COLUMN_MARKED_REMOVED &&
               rsp.err != Error::COLUMN_NOT_READY) {
              req->request_again();
            }
          }
        };
        Comm::Protocol::Mngr::Req::RangeUnloaded<ReqData>::request(
          Comm::Protocol::Mngr::Params::RangeUnloadedReq(
            col->cfg->cid, hdlr->range->rid),
          10000,
          col->cfg->cid, hdlr->range->rid
        );

        range->expand_and_align(true, Query::Update::CommonMeta::make(
          range,
          [await] (const Query::Update::CommonMeta::Ptr&) {
            await->set_value();
          })
        );
    }));
    new_range = nullptr;
    r_promise.get_future().wait();

    SWC_LOG_OUT(LOG_INFO,
      SWC_LOG_PRINTF("COMPACT-SPLITTED RANGE %lu/%lu took=%luns new-end=",
                      range->cfg->cid, range->rid, t_measure.elapsed());
      range->blocks.cellstores.back()->interval.key_end.print(SWC_LOG_OSTREAM);
    );
    return Error::OK;
  }

  void mngr_create_range(int& err, rid_t& new_rid) {
    std::promise<void>  res;

    struct ReqData {
      const cid_t         cid;
      int&                err;
      rid_t&              new_rid;
      std::promise<void>& res;
      SWC_CAN_INLINE
      ReqData(cid_t cid, int& err, rid_t& new_rid, std::promise<void>& res)
              noexcept : cid(cid), err(err), new_rid(new_rid), res(res) {
      }
      SWC_CAN_INLINE
      cid_t get_cid() const noexcept {
        return cid;
      }
      SWC_CAN_INLINE
      client::Clients::Ptr& get_clients() noexcept {
        return Env::Clients::get();
      }
      SWC_CAN_INLINE
      bool valid() noexcept {
        return !Env::Rgr::is_not_accepting();
      }
      SWC_CAN_INLINE
      void callback(
          const Comm::client::ConnQueue::ReqBase::Ptr& req,
          const Comm::Protocol::Mngr::Params::RangeCreateRsp& rsp) {
        SWC_LOGF(LOG_DEBUG,
          "RangeSplit::Mngr::Req::RangeCreate err=%d(%s) %lu/%lu",
          rsp.err, Error::get_text(rsp.err), cid, rsp.rid);

        if(rsp.err && valid() &&
           rsp.err != Error::CLIENT_STOPPING &&
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
    };
    Comm::Protocol::Mngr::Req::RangeCreate<ReqData>::request(
      Comm::Protocol::Mngr::Params::RangeCreateReq(
        range->cfg->cid, Env::Rgr::rgr_data()->rgrid),
      10000,
      range->cfg->cid, err, new_rid, res
    );
    res.get_future().wait();
  }

  const RangePtr range;
  const size_t   split_at;
};


}} // namespace SWC::Ranger

#endif // swcdb_ranger_db_RangeSplit_h
