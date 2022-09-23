/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/core/Time.h"

#include <ratio>
#include <ctime>

extern "C" {
#include <sys/time.h>
}


namespace SWC { namespace Time {

SWC_CAN_INLINE
int64_t parse_digit(int& err, const char** bufp,
                    int8_t default_v = 0) noexcept {
  errno = 0;
  while(**bufp && **bufp == '0') ++*bufp;
  if(**bufp) {
    char* last = nullptr;
    int64_t v = strtoll(*bufp, &last, 0);
    if(errno)
      err = errno;
    else
      *bufp = last;
    return v;
  }
  return default_v;
}

int64_t parse_ns(int& err, const std::string& buf) {
  /*if(!strptime(ptr, "%Y/%m/%d %H:%M:%S", &info)) {
    err = EINVAL;
    return ns;
  }*/

  const char* ptr = buf.c_str();
  int64_t ns = parse_digit(err, &ptr);
  if(err)
    return 0;
  if(!*ptr)
    return ns;
  if(*ptr != '/') {
    err = EINVAL;
    return 0;
  }

  struct tm info;
  info.tm_yday = info.tm_wday = 0;
  info.tm_isdst = -1;
  info.tm_year = ns - 1900;
  info.tm_mon = parse_digit(err, &++ptr, 1);
  if(err)
    return 0;
  if(info.tm_mon < 1 || info.tm_mon > 12) {
    err = EINVAL;
    return 0;
  }
  info.tm_mon -= 1;
  info.tm_mday = 1;
  info.tm_hour = 0;
  info.tm_min = 0;
  info.tm_sec = 0;

  if(*ptr == '/') {
    info.tm_mday = parse_digit(err, &++ptr, 1);
    if(err)
      return 0;
    if(info.tm_mday < 1 || info.tm_mday > 31) {
      err = EINVAL;
      return 0;
    }
    if(*ptr == ' ') {
      info.tm_hour = parse_digit(err, &++ptr);
      if(err)
        return 0;
      if(info.tm_hour < 0 || info.tm_hour > 23) {
        err = EINVAL;
        return 0;
      }
      if(*ptr == ':') {
        info.tm_min = parse_digit(err, &++ptr);
        if(err)
          return 0;
        if(info.tm_min < 0 || info.tm_min > 59) {
          err = EINVAL;
          return 0;
        }
        if(*ptr == ':') {
          info.tm_sec = parse_digit(err, &++ptr);
          if(err)
            return 0;
          if(info.tm_sec < 0 || info.tm_sec > 59) {
            err = EINVAL;
            return 0;
          }
          if(*ptr == '.') {
            ns = mktime(&info) * 1000000000;
            ++ptr;
            uint32_t res = 0;
            for(int places = 100000000;
                *ptr && std::isdigit(*ptr) && places >= 1;
                places/=10, ++ptr) {
              res += (*ptr - 48) * places;
            }
            if(*ptr) {
              err = EINVAL;
              return 0;
            }
            if(res) {
              if(ns < 0) {
                (ns -= 1000000000 - res) += 1000000000;
              } else {
                (ns += res + 1000000000) -= 1000000000;
              }
            }
            return ns;
          }
        }
      }
    }
  }

  return mktime(&info) * 1000000000;
}

std::string fmt_ns(int64_t ns) {
  int64_t secs = ns/1000000000;
  time_t t_secs = secs;

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
    nanos.insert(nanos.cbegin(), 9-nanos.size(), '0');
  return std::string(res) + "." + nanos;
}

}}

