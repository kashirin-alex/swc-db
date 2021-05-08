/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_Time_h
#define swcdb_core_Time_h


#include "swcdb/core/Compat.h"
#include <chrono>


namespace SWC {


/**
 * @brief The SWC-DB Date and Time C++ namespace 'SWC::Time'
 *
 * \ingroup Core
 */
namespace Time {


static_assert(
  std::ratio_less_equal<
    std::chrono::system_clock::duration::period,
    std::chrono::nanoseconds::period
  >::value
);


SWC_CAN_INLINE
extern
int64_t now_ms() noexcept {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
}

SWC_CAN_INLINE
extern
int64_t now_ns() noexcept {
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count();
}

int64_t parse_ns(int& err, const std::string& buf);

std::string fmt_ns(int64_t ns);

std::ostream &hires_now_ns(std::ostream &out);



template<typename ClockT, typename DurationT>
struct Measure : std::chrono::time_point<ClockT> {

  SWC_CAN_INLINE
  Measure() noexcept
          : std::chrono::time_point<ClockT>(ClockT::now()) {
  }

  SWC_CAN_INLINE
  Measure(std::chrono::time_point<ClockT>::duration&& v) noexcept
          : std::chrono::time_point<ClockT>(v) {
  }

  SWC_CAN_INLINE
  void restart() noexcept {
    std::chrono::time_point<ClockT>::operator=(ClockT::now());
  }

  template<typename T = DurationT>
  SWC_CAN_INLINE
  uint64_t elapsed() const noexcept {
    return std::chrono::duration_cast<T>(ClockT::now() - *this).count();
  }

};

typedef Measure<std::chrono::steady_clock, std::chrono::nanoseconds>
  Measure_ns;
typedef Measure<std::chrono::steady_clock, std::chrono::microseconds>
  Measure_us;
typedef Measure<std::chrono::steady_clock, std::chrono::milliseconds>
  Measure_ms;
typedef Measure<std::chrono::steady_clock, std::chrono::seconds>
  Measure_sec;


}}



#ifdef SWC_IMPL_SOURCE
#include "swcdb/core/Time.cc"
#endif

#endif // swcdb_core_Time_h
