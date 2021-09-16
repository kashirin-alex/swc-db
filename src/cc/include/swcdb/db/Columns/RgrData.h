/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_Columns_RgrData_h
#define swcdb_db_Columns_RgrData_h


#include "swcdb/core/Serialization.h"
#include "swcdb/core/Buffer.h"
#include "swcdb/core/StateSynchronization.h"

#include "swcdb/db/client/Query/Select/Handlers/BaseSingleColumn.h"
#include "swcdb/db/client/Query/Update/Handlers/BaseSingleColumn.h"
#include "swcdb/db/client/Query/Update/Handlers/Common.h"


namespace SWC { namespace DB {



class RgrData final {
  public:

  typedef std::shared_ptr<RgrData>  Ptr;
  Core::Atomic<rgrid_t>             rgrid;
  Comm::EndPoints                   endpoints;

  SWC_CAN_INLINE
  RgrData() noexcept : rgrid(0) { }

  //~RgrData() { }

  uint32_t encoded_length() const noexcept {
    return  Serialization::encoded_length_vi64(rgrid.load()) +
            Serialization::encoded_length(endpoints);
  }

  void encode(uint8_t** bufp) const noexcept {
    Serialization::encode_vi64(bufp, rgrid.load());
    Serialization::encode(bufp, endpoints);
  }

  void decode(const uint8_t *ptr, size_t remain) {
    rgrid.store(Serialization::decode_vi64(&ptr, &remain));
    Serialization::decode(&ptr, &remain, endpoints);
  }

  void print(std::ostream& out) {
    out << "RgrData(";
    Comm::print(out, endpoints);
    out << " rgrid=" << rgrid.load() << ')';
  }




  static void get_rgr(RgrData& data, cid_t cid, rid_t rid) noexcept {
    int err = Error::OK;
    try {
      auto hdlr = SyncSelector::Ptr(new SyncSelector());
      hdlr->scan(cid, rid);
      hdlr->await.wait();
      if((err = hdlr->error()))
        return;
      hdlr->get_rgr(err, data);
    } catch(...) {
      err = ENOKEY;
    }
    if(err) {
      data.rgrid.store(0);
      data.endpoints.clear();
    }
  }


  void set_insert_cell(cid_t cid, rid_t rid,
                       client::Query::Update::Handlers::Base::Column* colp)
                       const noexcept {
    DB::Cells::Cell cell;
    cell.set_time_order_desc(true);
    cell.key.add(std::to_string(cid));
    cell.key.add(std::to_string(rid));
    cell.flag = DB::Cells::INSERT;

    StaticBuffer buff(encoded_length());
    uint8_t* ptr = buff.base;
    encode(&ptr);
    cell.set_value(buff.base, buff.size, false);
    colp->add(cell);
  }

  static
  void set_delete_cell(cid_t cid, rid_t rid,
                       client::Query::Update::Handlers::Base::Column* colp)
                       noexcept {
    DB::Cells::Cell cell;
    cell.set_time_order_desc(true);
    cell.key.add(std::to_string(cid));
    cell.key.add(std::to_string(rid));
    cell.flag = DB::Cells::DELETE;

    colp->add(cell);
  }

  void set_rgr(int &err, cid_t cid, rid_t rid) noexcept {
    try {
      auto hdlr = SyncUpdater::Ptr(new SyncUpdater());
      hdlr->set_rgr(*this, cid, rid);
      hdlr->await.wait();
      err = hdlr->error();
    } catch(...) {
      err = SWC_CURRENT_EXCEPTION("").code();
    }
  }


  static void remove(int &err, cid_t cid, rid_t rid) noexcept {
    try {
      auto hdlr = SyncUpdater::Ptr(new SyncUpdater());
      hdlr->remove_rgr(cid, rid);
      hdlr->await.wait();
      err = hdlr->error();
    } catch(...) {
      err = SWC_CURRENT_EXCEPTION("").code();
    }
  }

  static void remove(int &err, cid_t cid) noexcept {
    try {
      DB::Cells::Result cells;
      {
      DB::Specs::Interval spec(DB::Types::Column::PLAIN);
      spec.flags.set_only_keys();
      auto& key_intval = spec.key_intervals.add();
      key_intval.start.reserve(2);
      key_intval.start.add(std::to_string(cid), Condition::EQ);
      key_intval.start.add("", Condition::GE);

      auto hdlr = SyncSelector::Ptr(new SyncSelector());
      hdlr->scan(
        DB::Types::KeySeq::VOLUME,
        DB::Types::SystemColumn::SYS_RGR_DATA,
        std::move(spec)
      );
      hdlr->await.wait();
      err = hdlr->error();
      if(err) {
        SWC_LOGF(LOG_WARN, "Remove RgrData for cid=%lu err=%d", cid, err);
        return;
      }
      hdlr->get_cells(cells);
      if(cells.empty())
        return;
      }

      auto hdlr = client::Query::Update::Handlers::Common::make(
        Env::Clients::get());
      auto& col = hdlr->create(
        DB::Types::SystemColumn::SYS_RGR_DATA,
        DB::Types::KeySeq::VOLUME, 1, 0, DB::Types::Column::PLAIN
      );
      for(auto cell : cells) {
        cell->flag = DB::Cells::DELETE;
        col->add(*cell);
        hdlr->commit_or_wait(col.get());
      }
      hdlr->commit_if_need();
      hdlr->wait();
      err = hdlr->error();
      SWC_LOGF(
        (err ? LOG_WARN : LOG_DEBUG),
        "Removed RgrData for cid=%lu ranges=%lu err=%d", cid, cells.size(), err
      );
    } catch(...) {
      err = SWC_CURRENT_EXCEPTION("").code();
    }
  }



  class BaseUpdater : public client::Query::Update::Handlers::BaseSingleColumn {
    public:
    typedef std::shared_ptr<BaseUpdater> Ptr;

    BaseUpdater()
        : client::Query::Update::Handlers::BaseSingleColumn(
            Env::Clients::get(),
            DB::Types::SystemColumn::SYS_RGR_DATA,
            DB::Types::KeySeq::VOLUME,
            1, 0, DB::Types::Column::PLAIN) {
    }

    void set_rgr(const RgrData& data, cid_t cid, rid_t rid) noexcept {
      data.set_insert_cell(cid, rid, &column);
      commit(&column);
    }

    void remove_rgr(cid_t cid, rid_t rid) noexcept {
      set_delete_cell(cid, rid, &column);
      commit(&column);
    }

    virtual bool valid() noexcept override {
      return true;
    }

    void response(int err=Error::OK) override {
      if(!completion.is_last())
        return;

      if(!err && valid() && requires_commit()) {
        commit(&column);
        return;
      }

      if(err)
        error(err);
      else if(!empty())
        error(Error::CLIENT_DATA_REMAINED);

      profile.finished();
      callback();
    }

    virtual void callback() = 0;

    protected:

    virtual ~BaseUpdater() { }

  };

  class SyncUpdater final : public BaseUpdater {
    public:
    typedef std::shared_ptr<SyncUpdater> Ptr;
    Core::StateSynchronization           await;

    void callback() override {
      await.acknowledge();
    };

    virtual ~SyncUpdater() { }
  };



  class BaseSelector :
          public client::Query::Select::Handlers::BaseSingleColumn {
    public:
    typedef std::shared_ptr<BaseSelector>   Ptr;

    using client::Query::Select::Handlers::BaseSingleColumn::scan;
    using client::Query::Select::Handlers::BaseSingleColumn::get_cells;

    SWC_CAN_INLINE
    BaseSelector()
          : client::Query::Select::Handlers::BaseSingleColumn(
              Env::Clients::get(), DB::Types::SystemColumn::SYS_RGR_DATA) {
    }

    bool valid() noexcept override {
      return !state_error;
    }

    void response(int err) override {
      if(err) {
        int at = Error::OK;
        state_error.compare_exchange_weak(at, err);
      }
      profile.finished();
      callback();
    }

    virtual void callback() = 0;

    void scan(cid_t a_cid, rid_t a_rid) {
      DB::Specs::Interval spec(DB::Types::Column::PLAIN);
      spec.flags.limit = 1;
      auto& key_intval = spec.key_intervals.add();
      key_intval.start.reserve(2);
      key_intval.start.add(std::to_string(a_cid), Condition::EQ);
      key_intval.start.add(std::to_string(a_rid), Condition::EQ);
      scan(
        DB::Types::KeySeq::VOLUME,
        DB::Types::SystemColumn::SYS_RGR_DATA,
        std::move(spec)
      );
    }

    void get_rgr(int& err, DB::RgrData& data) noexcept {
      try {
        DB::Cells::Result cells;
        get_cells(cells);
        if(cells.empty()) {
          err = ENOKEY;
          return;
        }
        const DB::Cells::Cell& cell = **cells.begin();
        data.decode(cell.value, cell.vlen);
      } catch(...) {
        err = SWC_CURRENT_EXCEPTION("").code();
      }
    }

    protected:

    virtual ~BaseSelector() { }

  };

  class SyncSelector final : public BaseSelector {
    public:
    typedef std::shared_ptr<SyncSelector> Ptr;
    Core::StateSynchronization            await;

    void callback() override {
      await.acknowledge();
    };

    virtual ~SyncSelector() { }
  };


};



}} // SWC::DB namespace



#endif // swcdb_db_Columns_RgrData_h
