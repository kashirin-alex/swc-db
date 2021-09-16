/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_common_sys_Resources_CPU_h
#define swcdb_common_sys_Resources_CPU_h


#include <sys/sysinfo.h>
#include <fstream>


namespace SWC { namespace System {

class CPU {
  static const uint32_t FAIL_MS = 300;
  public:
  typedef CPU*                                  Ptr;
  Core::Atomic<uint32_t>                        concurrency;
  Core::Atomic<uint32_t>                        mhz;
  Core::Atomic<uint32_t>                        usage;
  Core::Atomic<uint32_t>                        threads;

  CPU(Notifier* a_notifier) noexcept
      : concurrency(std::thread::hardware_concurrency()), mhz(0), usage(0),
        notifier(a_notifier), ms_intval(10000),
        stat_chk(0), stat_utime(0), stat_stime(0) {
  }

  uint64_t check(uint64_t ts) noexcept {
    try {
      if(notifier && ms_intval > notifier->get_cpu_ms_interval())
        ms_intval = notifier->get_cpu_ms_interval();
      return _check(ts);
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
      return FAIL_MS;
    }
  }

  void print(std::ostream& out) const {
    out << "Res(cores=" << concurrency
             << " MHz=" << mhz
             << " usage=" << usage << "%m"
             << " threads=" << threads << ')';
  }

  private:

  uint64_t _check(uint64_t ts) {
    if(!concurrency) {
      concurrency.store(std::thread::hardware_concurrency());
      if(!concurrency)
        return FAIL_MS;
    }
    if(stat_chk + ms_intval > ts)
      return (stat_chk + ms_intval) - ts;

    try {
      std::ifstream buffer("/proc/self/stat");
      if(!buffer.is_open())
        return FAIL_MS;
      std::string str_tmp;
      uint64_t tmp = 0, utime = 0, stime = 0, _nthreads = 0;
      buffer >> tmp >> str_tmp >> str_tmp
             >> tmp >> tmp >> tmp
             >> tmp >> tmp >> tmp
             >> tmp >> tmp >> tmp
             >> tmp >> utime >> stime
             >> tmp >> tmp >> tmp
             >> tmp >> _nthreads;
      buffer.close();
      threads.store(_nthreads);
      if(notifier)
        notifier->cpu_threads(_nthreads);

      if(!stat_chk) {
        stat_chk = ts;
        stat_utime = utime;
        stat_stime = stime;
      } else {
        uint64_t chk = ts;
        std::swap(stat_chk, chk);
        chk = ((stat_chk - chk) * sysconf(_SC_CLK_TCK) * concurrency) / 1000;
        std::swap(stat_utime, utime);
        utime = ((stat_utime - utime) * 100000) / chk;
        std::swap(stat_stime, stime);
        stime = ((stat_stime - stime) * 100000) / chk;
        usage.store((usage + utime + stime) / (1 + bool(usage)));
        if(notifier) {
          notifier->cpu_user(utime);
          notifier->cpu_sys(stime);
        }
      }
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }

    try {
      std::ifstream buffer(
        "/sys/devices/system/cpu/cpufreq/policy0/cpuinfo_max_freq");
      if(buffer.is_open()) {
        size_t khz = 0;
        buffer >> khz;
        buffer.close();
        if(khz)
          mhz.store(concurrency * (khz/1000));
        return ms_intval;
      }
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
    }

    try {
      std::ifstream buffer("/proc/cpuinfo");
      if(buffer.is_open()) {
        size_t _mhz = 0;
        std::string tmp;
        size_t tmp_speed = 0;
        do {
          buffer >> tmp;
          if(Condition::str_eq(tmp.c_str(), "cpu", 3)) {
            buffer >> tmp;
            if(Condition::str_eq(tmp.c_str(), "MHz", 3)) {
              buffer >> tmp >> tmp_speed;
              _mhz += tmp_speed;
            }
          }
        } while (!buffer.eof());
        buffer.close();
        if(_mhz)
          mhz.store(_mhz);
      }
    } catch(...) {
      SWC_LOG_CURRENT_EXCEPTION("");
      return FAIL_MS;
    }
    return ms_intval;
  }

  Notifier* notifier;
  uint64_t  ms_intval;
  uint64_t  stat_chk;
  uint64_t  stat_utime;
  uint64_t  stat_stime;

};


}}


#endif // swcdb_common_sys_Resources_CPU_h
