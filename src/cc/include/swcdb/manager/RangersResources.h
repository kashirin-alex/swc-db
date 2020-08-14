
/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_manager_RangersResources_h
#define swc_manager_RangersResources_h

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

  std::string to_string() const {
    std::string s("Res(load_scale=");
    s.append(std::to_string(load_scale));
    s.append(" mem=");
    s.append(std::to_string(mem));
    s.append("MB cpu=");
    s.append(std::to_string(cpu));
    s.append("Mhz ranges=");
    s.append(std::to_string(ranges));
    s.append(" rgrid=");
    s.append(std::to_string(rgrid));
    s.append(")");
    return s;
  }
};


class RangersResources final : private std::vector<RangerResources> {

  public:

  const Property::V_GINT32::Ptr cfg_rgr_res_check;

  RangersResources():
        cfg_rgr_res_check(Env::Config::settings()->get<Property::V_GINT32>(
          "swc.mngr.rangers.resource.interval.check")),
        m_due(0), m_last_check(0) { 
  }
      
  ~RangersResources() { }

  std::string to_string() {
    std::string s("RangersResources(rangers=");
    LockAtomic::Unique::scope lock(m_mutex);
    s.append(std::to_string(size()));
    s.append(" [");
    for(auto& r : *this) {
      s.append("\n ");
      s.append(r.to_string());
    }
    s.append("])");
    return s;
  }
  
  void check(const RangerList& rangers) {
    SWC_LOGF(LOG_DEBUG, "check %s ", to_string().c_str());

    if(!m_mutex.try_lock())
      return;
    if(m_due || 
       int64_t(m_last_check + cfg_rgr_res_check->get()) > Time::now_ns())
      return m_mutex.unlock();

    m_last_check = Time::now_ns();
    for(auto& rgr : rangers) {
      if(rgr->state != Ranger::State::ACK)
        continue;
      rgr->put(std::make_shared<Protocol::Rgr::Req::ReportRes>(rgr));
      ++m_due;
    }

    m_mutex.unlock();
  }

  bool add_and_more(rgrid_t rgrid, int err,
                    const Protocol::Rgr::Params::Report::RspRes& rsp) {
    LockAtomic::Unique::scope lock(m_mutex);
    if(!err)
      emplace_back(rgrid, rsp.mem, rsp.cpu, rsp.ranges);
    return m_due ? --m_due : false;
  }

  void evaluate() {
    // load_scale = (UINT16_MAX free , 0 overloaded)
    size_t max_mem = 0;
    size_t max_cpu = 0;
    size_t max_ranges = 1;
    uint16_t max_scale = 1;
    {
      LockAtomic::Unique::scope lock(m_mutex);
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
    SWC_LOGF(LOG_DEBUG, "changes %s ", to_string().c_str());

    LockAtomic::Unique::scope lock(m_mutex);
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

  LockAtomic::Unique m_mutex;
  size_t             m_due;
  size_t             m_last_check;

};

}}

#endif // swc_manager_RangersResources_h
