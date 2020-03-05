/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/core/Error.h"
#include "swcdb/core/config/Property.h"

namespace SWC {


namespace Property {


void from_string(const std::string& s, double* value) {
  char *last;
  double res = strtod(s.c_str(), &last);

  if (s.c_str() == last)
    SWC_THROWF(Error::CONFIG_GET_ERROR, "Bad Value %s", s.c_str());

  switch (*last) {
    case 'k':
    case 'K': res *= 1000LL;         break;
    case 'm':
    case 'M': res *= 1000000LL;      break;
    case 'g':
    case 'G': res *= 1000000000LL;   break;
    case '\0':                          break;
    default: 
      SWC_THROWF(Error::CONFIG_GET_ERROR, 
                "Bad Value %s unknown suffix %s", s.c_str(), last);
  }
  *value = res;
}

void from_string(const std::string& s, int64_t* value) {
  char *last;
  errno = 0;
  *value = strtoll(s.c_str(), &last, 0);

  if (s.c_str() == last)
    SWC_THROWF(Error::CONFIG_GET_ERROR, "Bad Value %s", s.c_str());
  
  if(errno) 
    SWC_THROWF(Error::CONFIG_GET_ERROR, 
              "Bad Value %s, number out of range of 64-bit integer", s.c_str());
  
  switch (*last) {
    case 'k':
    case 'K': *value *= 1000LL;         break;
    case 'm':
    case 'M': *value *= 1000000LL;      break;
    case 'g':
    case 'G': *value *= 1000000000LL;   break;
    case '\0':                          break;
    default: 
      SWC_THROWF(Error::CONFIG_GET_ERROR, 
                "Bad Value %s unknown suffix %s", s.c_str(), last);
  }
}

void from_string(const std::string& s, uint8_t* value) {
  int64_t res;
  from_string(s, &res);

  if (res > UINT8_MAX || res < 0) 
    SWC_THROWF(Error::CONFIG_GET_ERROR, 
      "Bad Value %s, number out of range of 8-bit unsigned integer", s.c_str());
  *value = (uint8_t)res;
}

void from_string(const std::string& s, uint16_t* value) {
  int64_t res;
  from_string(s, &res);

  if (res > UINT16_MAX || res < 0) 
    SWC_THROWF(Error::CONFIG_GET_ERROR, 
      "Bad Value %s, number out of range of 16-bit unsigned integer", s.c_str());
  *value = (uint16_t)res;
}

void from_string(const std::string& s, int32_t* value) {
  int64_t res;
  from_string(s, &res);

  if (res > INT32_MAX || res < INT32_MIN) 
    SWC_THROWF(Error::CONFIG_GET_ERROR, 
        "Bad Value %s, number out of range of 32-bit integer", s.c_str());
  *value = (int32_t)res;
}



Value::Value(bool skippable, bool guarded)
            : m_skippable(skippable), m_guarded(guarded) {
}
  
Value::Value(Value::Ptr ptr) 
            : m_skippable(ptr->is_skippable()),
              m_guarded(ptr->is_guarded()),
              m_defaulted(ptr->is_default()),
              m_no_token(ptr->is_zero_token()) {
}

Value::~Value() { }

std::ostream& Value::operator<<(std::ostream& ostream) {
  return ostream << to_string();
}
  
const bool Value::is_skippable() const {
  return m_skippable;
}

const bool Value::is_guarded() const {
  return m_guarded;
}

void Value::guarded(bool guarded) {
  m_guarded = guarded;
}

Value::Ptr Value::default_value(bool defaulted) {
  m_defaulted = defaulted;
  return this;
}

const bool Value::is_default() const {
  return m_defaulted;
}
  
Value::Ptr Value::zero_token() {
  m_no_token = true;
  return this;
}

const bool Value::is_zero_token() const {
  return m_no_token;
}





}} // namespace SWC::Property

