
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
                  : rgrid(rgrid), mem(mem), cpu(cpu), ranges(ranges), 
                    load_scale(0) {
  }

  ~RangerResources() { }

  rgrid_t     rgrid;
  uint32_t    mem;
  uint32_t    cpu;
  size_t      ranges;
  uint16_t    load_scale;

  void print(std::ostream& out) const {
    out << "Res(load_scale=" << load_scale
        << " mem=" << mem << "MB cpu=" << cpu << "Mhz ranges=" << ranges
        << " rgrid=" << rgrid << ')';
  }
};


class RangersResources final : private std::vector<RangerResources> {

  public:

  const Config::Property::V_GINT32::Ptr cfg_rgr_res_check;

  RangersResources():
        cfg_rgr_res_check(
          Env::Config::settings()->get<Config::Property::V_GINT32>(
            "swc.mngr.rangers.resource.interval.check")),
        m_due(0), m_last_check(0) { 
  }
      
  ~RangersResources() { }

  void print(std::ostream& out) {
    out << "RangersResources(rangers=";
    Core::MutexAtomic::scope lock(m_mutex);
    out << size() << " [";
    for(auto& r : *this)
      r.print(out << "\n ");
    out << "\n])";
  }
  
  void check(const RangerList& rangers) {
    SWC_LOG_OUT(LOG_DEBUG, print(SWC_LOG_OSTREAM << "check "); );

    if(!m_mutex.try_lock())
      return;
    if(m_due || 
       int64_t(m_last_check + cfg_rgr_res_check->get()) > Time::now_ns())
      return m_mutex.unlock();

    m_last_check = Time::now_ns();
    for(auto& rgr : rangers) {
      if(rgr->state != Ranger::State::ACK)
        continue;
      rgr->put(std::make_shared<Comm::Protocol::Rgr::Req::ReportRes>(rgr));
      ++m_due;
    }

    m_mutex.unlock();
  }

  bool add_and_more(rgrid_t rgrid, int err,
                    const Comm::Protocol::Rgr::Params::Report::RspRes& rsp) {
    Core::MutexAtomic::scope lock(m_mutex);
    if(!err) {
      auto& res = emplace_back(rgrid, rsp.mem, rsp.cpu, rsp.ranges);
      if(!res.mem || !res.cpu) {
        SWC_LOG_OUT(LOG_WARN, 
          res.print(SWC_LOG_OSTREAM << "received zero(resource) "); );
      }
    }
    return m_due ? --m_due : false;
  }

  void evaluate() {
    // load_scale = (UINT16_MAX free , 0 overloaded)
    size_t max_mem = 1;
    size_t max_cpu = 1;
    size_t max_ranges = 1;
    uint16_t max_scale = 1;
    {
      Core::MutexAtomic::scope lock(m_mutex);
      for(auto& res : *this) {
        if(res.mem > max_mem)
          max_mem = res.mem;
        if(res.cpu > max_cpu)
          max_cpu = res.cpu;
        if(res.ranges > max_ranges)
          max_ranges = res.ranges;
      }
      
      for(auto& res : *this) {
        size_t ranges_scale = (res.ranges * 100) / max_ranges;
        size_t load_scale = (
            ( (res.mem * 100 * INT16_MAX) / max_mem +
              (res.cpu * 100 * INT16_MAX) / max_cpu )
          ) / (ranges_scale ? ranges_scale : 1);
        res.load_scale = load_scale > UINT16_MAX 
          ? UINT16_MAX : load_scale;
        if(res.load_scale > max_scale)
          max_scale = res.load_scale;
      }
      for(auto& res : *this)
        res.load_scale = size_t(res.load_scale * UINT16_MAX) / max_scale;
    }
  }

  void changes(const RangerList& rangers, RangerList& changed) {
    SWC_LOG_OUT(LOG_DEBUG, print(SWC_LOG_OSTREAM << "changes "); );

    Core::MutexAtomic::scope lock(m_mutex);
    for(auto& rgr : rangers) {
      for(auto& res : *this) {
        if(rgr->rgrid == res.rgrid) {
          if(rgr->load_scale != res.load_scale) {
            rgr->load_scale = res.load_scale;
            changed.push_back(rgr);
          }
          rgr->interm_ranges = 0;
          break;
        }
      }
    }
    std::vector<RangerResources>::clear();
  }
  
  private:

  Core::MutexAtomic  m_mutex;
  size_t             m_due;
  size_t             m_last_check;

};

}}

#endif // swcdb_manager_RangersResources_h
