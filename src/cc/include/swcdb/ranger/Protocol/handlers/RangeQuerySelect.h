/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_ranger_Protocol_handlers_RangeQuerySelect_h
#define swcdb_ranger_Protocol_handlers_RangeQuerySelect_h

#include "swcdb/db/Protocol/Rgr/params/RangeQuerySelect.h"
#include "swcdb/ranger/callbacks/RangeQuerySelect.h"
#include "swcdb/ranger/callbacks/RangeQuerySelectUpdating.h"
#include "swcdb/ranger/callbacks/RangeQuerySelectUpdating_Append.h"
#include "swcdb/ranger/callbacks/RangeQuerySelectUpdating_Prepend.h"
#include "swcdb/ranger/callbacks/RangeQuerySelectUpdating_Insert.h"
#include "swcdb/ranger/callbacks/RangeQuerySelectUpdating_Overwrite.h"
#include "swcdb/ranger/callbacks/RangeQuerySelectUpdating_Serial.h"
#include "swcdb/ranger/callbacks/RangeQuerySelectDeleting.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Rgr { namespace Handler {


struct RangeQuerySelect {
  Comm::ConnHandlerPtr conn;
  Comm::Event::Ptr     ev;

  SWC_CAN_INLINE
  RangeQuerySelect(const Comm::ConnHandlerPtr& a_conn,
                   const Comm::Event::Ptr& a_ev) noexcept
                  : conn(a_conn), ev(a_ev) {
  }

  SWC_CAN_INLINE
  RangeQuerySelect(RangeQuerySelect&& other) noexcept
                  : conn(std::move(other.conn)),
                    ev(std::move(other.ev)) {
  }

  RangeQuerySelect(const RangeQuerySelect&) = delete;
  RangeQuerySelect& operator=(RangeQuerySelect&&) = delete;
  RangeQuerySelect& operator=(const RangeQuerySelect&) = delete;

  ~RangeQuerySelect() noexcept { }

  void operator()() {
    if(ev->expired())
      return;

    int err = Error::OK;
    Params::RangeQuerySelectReq params;
    Ranger::RangePtr range;

    try {
      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;
      params.decode(&ptr, &remain);

      range = Env::Rgr::columns()->get_range(err, params.cid, params.rid);

      if(!err) {
        if(!range || !range->is_loaded()) {
          err = Error::RGR_NOT_LOADED_RANGE;

        } else if(range->cfg->range_type == DB::Types::Range::DATA &&
                  Env::Rgr::res().is_low_mem_state() &&
                  Env::Rgr::scan_reserved_bytes()) {
          err = Error::SERVER_MEMORY_LOW;
        }
      }

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR,
        SWC_LOG_OSTREAM << e;
        ev->print(SWC_LOG_OSTREAM << "\n\t");
      );
      err = e.code();
    }

    if(err) {
      Params::RangeQuerySelectRsp rsp(err);
      conn->send_response(Buffers::make(ev, rsp));

    } else if(params.interval.has_opt__updating()) {
      uint8_t op = params.interval.updating->operation.get_op();
      if(range->cfg->col_type == DB::Types::Column::SERIAL) {
          if(params.interval.updating->operation.has_pos())
            op = DB::Specs::UpdateOP::REPLACE;
      } else if(DB::Types::is_counter(range->cfg->col_type) ||
                op == DB::Specs::UpdateOP::SERIAL)  {
        op = DB::Specs::UpdateOP::REPLACE;
      }

      switch(op) {
        case DB::Specs::UpdateOP::REPLACE: {
          range->scan(Ranger::Callback::RangeQuerySelectUpdating::Ptr(
            new Ranger::Callback::RangeQuerySelectUpdating(
              conn, ev, std::move(params.interval), range)
          ));
          break;
        }
        case DB::Specs::UpdateOP::APPEND: {
          range->scan(Ranger::Callback::RangeQuerySelectUpdating_Append::Ptr(
            new Ranger::Callback::RangeQuerySelectUpdating_Append(
              conn, ev, std::move(params.interval), range)
          ));
          break;
        }
        case DB::Specs::UpdateOP::PREPEND: {
          range->scan(Ranger::Callback::RangeQuerySelectUpdating_Prepend::Ptr(
            new Ranger::Callback::RangeQuerySelectUpdating_Prepend(
              conn, ev, std::move(params.interval), range)
          ));
          break;
        }
        case DB::Specs::UpdateOP::INSERT: {
          range->scan(Ranger::Callback::RangeQuerySelectUpdating_Insert::Ptr(
            new Ranger::Callback::RangeQuerySelectUpdating_Insert(
              conn, ev, std::move(params.interval), range)
          ));
          break;
        }
        case DB::Specs::UpdateOP::OVERWRITE: {
          range->scan(Ranger::Callback::RangeQuerySelectUpdating_Overwrite::Ptr(
            new Ranger::Callback::RangeQuerySelectUpdating_Overwrite(
              conn, ev, std::move(params.interval), range)
          ));
          break;
        }
        case DB::Specs::UpdateOP::SERIAL: {
          range->scan(Ranger::Callback::RangeQuerySelectUpdating_Serial::Ptr(
            new Ranger::Callback::RangeQuerySelectUpdating_Serial(
              conn, ev, std::move(params.interval), range)
          ));
          break;
        }
        default: {
          Params::RangeQuerySelectRsp rsp(Error::INVALID_ARGUMENT);
          conn->send_response(Buffers::make(ev, rsp));
          break;
        }
      }

    } else if(params.interval.has_opt__deleting()) {
      range->scan(Ranger::Callback::RangeQuerySelectDeleting::Ptr(
        new Ranger::Callback::RangeQuerySelectDeleting(
          conn, ev, std::move(params.interval), range)
      ));

    } else {
      range->scan(Ranger::Callback::RangeQuerySelect::Ptr(
        new Ranger::Callback::RangeQuerySelect(
          conn, ev, std::move(params.interval), range)
      ));
    }

  }

};


}}}}}

#endif // swcdb_ranger_Protocol_handlers_RangeQuerySelect_h
