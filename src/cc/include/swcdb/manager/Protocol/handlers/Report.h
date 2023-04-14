/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_manager_Protocol_handlers_Report_h
#define swcdb_manager_Protocol_handlers_Report_h

#include "swcdb/db/Protocol/Mngr/params/Report.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace Mngr { namespace Handler {


struct Report {
  Comm::ConnHandlerPtr conn;
  Comm::Event::Ptr     ev;

  SWC_CAN_INLINE
  Report(const Comm::ConnHandlerPtr& a_conn,
         const Comm::Event::Ptr& a_ev) noexcept
        : conn(a_conn), ev(a_ev) {
  }

  SWC_CAN_INLINE
  Report(Report&& other) noexcept
        : conn(std::move(other.conn)), ev(std::move(other.ev)) {
  }

  Report(const Report&) = delete;
  Report& operator=(Report&&) = delete;
  Report& operator=(const Report&) = delete;

  ~Report() noexcept { }

  void operator()() {
    if(ev->expired())
      return;


    int err = Error::OK;
    Buffers::Ptr cbp;

    try {

      const uint8_t *ptr = ev->data.base;
      size_t remain = ev->data.size;

      switch(
        Params::Report::Function(Serialization::decode_i8(&ptr, &remain))) {

        case Params::Report::Function::CLUSTER_STATUS: {

          if(Env::Mngr::mngd_columns()->is_schemas_mngr(err) && err)
            goto function_send_response;

          if(Env::Mngr::role()->is_active_role(DB::Types::MngrRole::RANGERS)
             && Env::Mngr::rangers()->empty()) {
            err = Error::MNGR_NOT_INITIALIZED;
            goto function_send_response;
          }

          Env::Mngr::mngd_columns()->columns_ready(err);

          function_send_response:
            cbp = Buffers::make(ev, 4);
            cbp->append_i32(err);
            goto send_response;
        }

        case Params::Report::Function::MANAGERS_STATUS: {
          auto& role = *Env::Mngr::role();

          Manager::MngrsStatus mngrs;
          role.get_states(mngrs);

          Params::Report::RspManagersStatus rsp_params;
          rsp_params.managers.resize(mngrs.size());
          size_t i = 0;
          for(auto& mngr : mngrs) {
            auto& m = rsp_params.managers[i];
            m.priority = mngr->priority;
            m.state = mngr->state;
            m.role = mngr->role;
            m.cid_begin = mngr->cid_begin;
            m.cid_end = mngr->cid_end;
            m.failures = mngr->failures;
            m.endpoints = mngr->endpoints;
            ++i;
          }
          rsp_params.inchain = role.get_inchain_endpoint();

          cbp = Buffers::make(ev, rsp_params, 4);
          cbp->append_i32(err);
          goto send_response;
        }

        case Params::Report::Function::RANGERS_STATUS: {
          auto& mngr_rangers = *Env::Mngr::rangers();

          Manager::RangerList rangers;
          mngr_rangers.rgr_list(0, rangers);

          Params::Report::RspRangersStatus rsp_params;
          rsp_params.rangers.resize(rangers.size());
          size_t i = 0;
          for(auto& rgr : rangers) {
            auto& r = rsp_params.rangers[i];
            r.state = rgr->state;
            r.rgr_id = rgr->rgrid;
            r.failures = rgr->failures;
            r.interm_ranges = rgr->interm_ranges;
            r.load_scale = rgr->load_scale;
            r.rebalance = rgr->rebalance();
            mngr_rangers.rgr_get(rgr, r.endpoints);
            ++i;
          }
          cbp = Buffers::make(ev, rsp_params, 4);
          cbp->append_i32(err);
          goto send_response;
        }

        case Params::Report::Function::COLUMN_STATUS: {
          Params::Report::ReqColumnStatus params;
          params.decode(&ptr, &remain);

          Params::Report::RspColumnStatus rsp_params;
          auto col = Env::Mngr::mngd_columns()->get_column(err, params.cid);
          if(err == Error::COLUMN_NOT_READY)
            err = Error::OK;
          else if(err)
            goto send_error;

          Core::Vector<Manager::Range::Ptr> ranges;
          col->get_ranges(ranges);
          rsp_params.ranges.resize(ranges.size());
          size_t i = 0;
          for(auto& r : ranges) {
            auto& set_r = rsp_params.ranges[i];
            set_r.state = r->state();
            set_r.rid = r->rid;
            set_r.rgr_id = r->get_rgr_id();
            ++i;
          }
          rsp_params.state = col->state();
          cbp = Buffers::make(ev, rsp_params, 4);
          cbp->append_i32(err);
          goto send_response;
        }

        default:
          err = Error::NOT_IMPLEMENTED;
          break;
      }

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      SWC_LOG_OUT(LOG_ERROR, SWC_LOG_OSTREAM << e; );
      err = e.code();
    }


    send_error:
      cbp = Buffers::make(ev, 4);
      cbp->append_i32(err);


    send_response:
      conn->send_response(cbp);
  }

};


}}}}}

#endif // swcdb_manager_Protocol_handlers_:Report_h
