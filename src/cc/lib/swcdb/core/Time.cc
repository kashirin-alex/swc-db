/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */
 

#include "swcdb/core/Exception.h"
#include "swcdb/core/Time.h"

#include <ratio>

#include <iomanip>
#include <ctime>

extern "C" {
#include <sys/time.h>
}


namespace SWC { namespace Time {


void checkings() { // no need runtime checks, call at app start 
  SWC_ASSERT((
    std::ratio_less_equal<
      std::chrono::system_clock::duration::period, 
      std::chrono::nanoseconds::period
    >::value
  ));
}

int64_t parse_ns(int& err, const std::string& buf) {
  const char* ptr = buf.c_str();
  if(buf.find("/") == std::string::npos) {  
    while(*ptr != 0 && *ptr == '0') ++ptr;
    char *last;
    errno = 0;
    int64_t ns = strtoll(ptr, &last, 0);
    if(errno || (*ptr != 0 && ptr == last))
      err = errno ? errno : EINVAL;
    return ns;
  }

  int64_t ns = 0;

  struct tm info;
  if(strptime(ptr, "%Y/%m/%d %H:%M:%S", &info) == NULL) {
    err = EINVAL;
    return ns;
  } 
  ns += mktime(&info) * 1000000000;

  while(*ptr != 0 && *ptr++ != '.');

  uint32_t res = 0;
  for(int places = 100000000; 
      *ptr != 0 && std::isdigit(*ptr) && places >= 1; 
      places/=10, ++ptr) {
    res += (*ptr - 48) * places;
  }
  if(res) {
    if(ns < 0) {
      (ns -= 1000000000 - res) += 1000000000;
    } else {
      (ns += res + 1000000000) -= 1000000000;
    }
  }
  if(*ptr != 0)
    err = EINVAL;
  return ns;
}

std::string fmt_ns(int64_t ns) {
  int64_t secs = ns/1000000000;
  time_t t_secs = (time_t)secs;

  std::string nanos;
  if(ns < 0) {
    uint32_t n = -ns + secs * 1000000000;
    if(n > 0) {
      nanos = std::to_string(1000000000 - n);
      --t_secs; // borrow a second towrads nanos
    }
  } else {
    nanos = std::to_string(ns - secs * 1000000000);
  }

  char res[20];
  std::strftime(res, 20, "%Y/%m/%d %H:%M:%S", std::gmtime(&t_secs));

  if(nanos.size() < 9)
    nanos.insert(nanos.begin(), 9-nanos.size(), '0');
  return std::string(res) + "." + nanos;
}

std::ostream &hires_now_ns(std::ostream &out) {
  auto now = std::chrono::system_clock::now();
  return out << std::chrono::duration_cast<std::chrono::seconds>(
                  now.time_since_epoch()).count() 
             <<'.'<< std::setw(9) << std::setfill('0') 
             << (std::chrono::duration_cast<std::chrono::nanoseconds>(
                now.time_since_epoch()).count() % 1000000000LL);
}

}}

