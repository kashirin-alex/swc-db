
/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_manager_RangersResources_h
#define swcdb_manager_RangersResources_h

#include "swcdb/manager/Protocol/Rgr/req/ReportRes.h"

namespace SWC { namespace Manager {


struct RangerResources {

  RangerResources(rgrid_t rgrid = 0,
                  uint32_t mem = 0, uint32_t cpu = 0, size_t ranges = 0)
                  noexcept
                  : rgrid(rgrid), mem(mem), cpu(cpu), ranges(ranges),
                    load_scale(0), rebalance(0) {
  }

  ~RangerResources() { }

  rgrid_t     rgrid;
  uint32_t    mem;
  uint32_t    cpu;
  size_t      ranges;

  uint16_t    load_scale;
  uint8_t     rebalance;

  void print(std::ostream& out) const {
    out << "Res(load_scale=" << load_scale
        << " rebalance=" << int16_t(rebalance)
        << " mem=" << mem << "MB cpu=" << cpu << "Mhz ranges=" << ranges
        << " rgrid=" << rgrid << ')';
  }
};


class RangersResources final : private std::vector<RangerResources> {

  public:

  const Config::Property::V_GINT32::Ptr cfg_check;
  const Config::Property::V_GUINT8::Ptr cfg_rebalance_max;

  RangersResources():
        cfg_check(
          Env::Config::settings()->get<Config::Property::V_GINT32>(
            "swc.mngr.rangers.resource.interval.check")),
        cfg_rebalance_max(
          Env::Config::settings()->get<Config::Property::V_GUINT8>(
            "swc.mngr.rangers.range.rebalance.max")),
        m_due(0), m_last_check(0) {
  }

  ~RangersResources() { }

  void print(std::ostream& out) {
    out << "RangersResources(rangers=";
    Core::MutexSptd::scope lock(m_mutex);
    out << size() << " [";
    for(auto& r : *this)
      r.print(out << "\n ");
    out << "\n])";
  }

  void check(const RangerList& rangers) {
    bool support;
    if(!m_mutex.try_full_lock(support))
      return;
    if(m_due ||
       int64_t(m_last_check + cfg_check->get()) > Time::now_ns())
      return m_mutex.unlock(support);

    m_last_check = Time::now_ns();
    for(auto& rgr : rangers) {
      if(rgr->state == RangerState::ACK ||
         rgr->state == RangerState::MARKED_OFFLINE) {
        rgr->put(std::make_shared<Comm::Protocol::Rgr::Req::ReportRes>(rgr));
        ++m_due;
      }
    }
    m_mutex.unlock(support);
  }

  bool add_and_more(rgrid_t rgrid, int err,
                    const Comm::Protocol::Rgr::Params::Report::RspRes& rsp) {
    Core::MutexSptd::scope lock(m_mutex);
    if(!err && m_due) {
      auto& res = emplace_back(rgrid, rsp.mem, rsp.cpu, rsp.ranges);
      if(!res.mem || !res.cpu) {
        SWC_LOG_OUT(LOG_WARN,
          res.print(SWC_LOG_OSTREAM << "received zero(resource) "); );
      }
    }
    return m_due ? --m_due : true;
  }

  void evaluate() {
    // load_scale = (UINT16_MAX free , 1 overloaded)
    size_t max_mem = 1;
    size_t max_cpu = 1;
    size_t max_ranges = 1;
    uint16_t max_scale = 1;

    size_t total_ranges = 0;
    uint16_t max_load_scale = 0;
    uint8_t balanceable = cfg_rebalance_max->get();

    {
      Core::MutexSptd::scope lock(m_mutex);
      for(auto& res : *this) {
        if(res.mem > max_mem)
          max_mem = res.mem;
        if(res.cpu > max_cpu)
          max_cpu = res.cpu;
        if(res.ranges > max_ranges)
          max_ranges = res.ranges;
        total_ranges += res.ranges;
      }

      for(auto& res : *this) {
        size_t ranges_scale = (res.ranges * 100) / max_ranges;
        size_t load_scale = (
            ( (res.mem * 100 * INT16_MAX) / max_mem +
              (res.cpu * 100 * INT16_MAX) / max_cpu )
          ) / (ranges_scale ? ranges_scale : 1);
        res.load_scale = load_scale > UINT16_MAX
          ? UINT16_MAX : (load_scale ? load_scale : 1);
        if(res.load_scale > max_scale)
          max_scale = res.load_scale;
      }
      for(auto& res : *this) {
        res.load_scale = (size_t(res.load_scale) * UINT16_MAX) / max_scale;
        if(max_load_scale < res.load_scale)
          max_load_scale = res.load_scale;
      }

      if(balanceable) {
        std::sort(begin(), end(),
                  [](const RangerResources& rr1, const RangerResources& rr2) {
                    return rr1.load_scale < rr2.load_scale; });

        if(total_ranges > size() * 4 && (max_load_scale >>= 1)) {
          for(auto& res : *this) {
            if(res.ranges && res.load_scale < max_load_scale) {
              uint16_t chk = max_load_scale / res.load_scale;
              uint8_t balance = chk > balanceable ? balanceable : chk;
              if(balance) {
                balanceable -= balance;
                res.rebalance = balance;
              }
            }
            if(!balanceable)
              break;
          }
        }
      }

    }
  }

  void changes(const RangerList& rangers, RangerList& changed) {
    SWC_LOG_OUT(LOG_DEBUG, print(SWC_LOG_OSTREAM << "changes "); );

    Core::MutexSptd::scope lock(m_mutex);
    for(auto& res : *this) {
      for(auto& rgr : rangers) {
        if(rgr->rgrid != res.rgrid)
          continue;

        uint8_t balance = res.rebalance;
        if(balance && rgr->state == RangerState::ACK) {
          rgr->rebalance(balance);
        } else if((balance = rgr->rebalance())) {
          rgr->rebalance(0);
        }

        if(balance || rgr->load_scale != res.load_scale) {
          rgr->load_scale.store(res.load_scale);
          changed.push_back(rgr);
        }
        rgr->interm_ranges.store(0);
        break;
      }
    }
    std::vector<RangerResources>::clear();
  }

  private:

  Core::MutexSptd    m_mutex;
  size_t             m_due;
  size_t             m_last_check;

};

}}

#endif // swcdb_manager_RangersResources_h
