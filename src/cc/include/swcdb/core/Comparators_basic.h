/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_Comparators_basic_h
#define swcdb_core_Comparators_basic_h


/*
#if defined(__GNUC__ ) && __GNUC__ >= 10
#define SWC_IMPL_COMPARATORS_BASIC ON
#endif
*/


namespace SWC {

/**
 * @brief The SWC-DB Comparators C++ namespace 'SWC::Condition'
 *
 * \ingroup Core
 */


namespace Condition {


extern int
mem_cmp(const uint8_t* b1, const uint8_t* b2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern bool
mem_eq(const uint8_t* b1, const uint8_t* b2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));


extern int
str_cmp(const char* s1, const char* s2) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern int
str_cmp(const char* s1, const char* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));


extern bool
str_eq(const char* s1, const char* s2) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern bool
str_eq(const char* s1, const char* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern bool
str_case_eq(const char* s1, const char* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));


extern bool
str_eq(const std::string& s1, const std::string& s2) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern bool
mem_eq(const std::string& s1, const std::string& s2) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));



namespace { // local namespace


#if defined(SWC_IMPL_COMPARATORS_BASIC)

static int
_mem_cmp(const uint8_t* b1, const uint8_t* b2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

static bool
_mem_eq(const uint8_t* b1, const uint8_t* b2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));



SWC_SHOULD_NOT_INLINE
static int
_mem_cmp(const uint8_t* b1, const uint8_t* b2, size_t count) noexcept {
  for(; count; --count, ++b1, ++b2)
    if(*b1 != *b2)
      return *b1 < *b2 ? -1 : 1;
  return 0;
}

SWC_SHOULD_NOT_INLINE
static bool
_mem_eq(const uint8_t* b1, const uint8_t* b2, size_t count) noexcept {
  for(; count; --count, ++b1, ++b2) {
    if(*b1 != *b2)
      return false;
  }
  return true;
}

static int
_str_cmp(const char* s1, const char* s2) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

static int
_str_cmp(const char* s1, const char* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

static bool
_str_eq(const char* s1, const char* s2) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

static bool
_str_eq(const char* s1, const char* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

static bool
_str_case_eq(const char* s1, const char* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));



SWC_SHOULD_NOT_INLINE
static int
_str_cmp(const char* s1, const char* s2) noexcept {
  for(; ; ++s1, ++s2) {
    if(*s1 != *s2)
      return *s1 < *s2 ? -1 : 1;
    if(!*s1)
      break;
  }
  return 0;
}

SWC_SHOULD_NOT_INLINE
static int
_str_cmp(const char* s1, const char* s2, size_t count) noexcept {
  for(; count; --count, ++s1, ++s2) {
    if(*s1 != *s2)
      return *s1 < *s2 ? -1 : 1;
    if(!*s1)
      break;
  }
  return 0;
}

SWC_SHOULD_NOT_INLINE
static bool
_str_eq(const char* s1, const char* s2) noexcept {
  for(; ; ++s1, ++s2) {
    if(*s1 != *s2)
      return false;
    if(!*s1)
      return true;
  }
}

SWC_SHOULD_NOT_INLINE
static bool
_str_eq(const char* s1, const char* s2, size_t count) noexcept {
  for(; count; --count, ++s1, ++s2) {
    if(*s1 != *s2)
      return false;
    if(!*s1)
      break;
  }
  return true;
}

SWC_SHOULD_NOT_INLINE
static bool
_str_case_eq(const char* s1, const char* s2, size_t count) noexcept {
  for(; count; --count, ++s1, ++s2) {
    if(*s1 - (*s1 > 96 && *s1 < 123 ? 32 : 0) !=
       *s2 - (*s2 > 96 && *s2 < 123 ? 32 : 0))
      return false;
    if(!*s1)
      break;
  }
  return true;
}

#endif // defined(SWC_IMPL_COMPARATORS_BASIC)

} // local namespace





extern SWC_CAN_INLINE
int
mem_cmp(const uint8_t* b1, const uint8_t* b2, size_t count) noexcept {
  #if defined(SWC_IMPL_COMPARATORS_BASIC)
    return _mem_cmp(b1, b2, count);
  #else
    return memcmp(b1, b2, count);
  #endif
}

extern SWC_CAN_INLINE
bool
mem_eq(const uint8_t* b1, const uint8_t* b2, size_t count) noexcept {
  #if defined(SWC_IMPL_COMPARATORS_BASIC)
    return _mem_eq(b1, b2, count);
  #else
    return !memcmp(b1, b2, count);
  #endif
}

extern SWC_CAN_INLINE
int
str_cmp(const char* s1, const char* s2) noexcept {
  #if defined(SWC_IMPL_COMPARATORS_BASIC)
    return _str_cmp(s1, s2);
  #else
    return strcmp(s1, s2);
  #endif
}

extern SWC_CAN_INLINE
int
str_cmp(const char* s1, const char* s2, size_t count) noexcept {
  #if defined(SWC_IMPL_COMPARATORS_BASIC)
    return _str_cmp(s1, s2, count);
  #else
    return strncmp(s1, s2, count);
  #endif
}

extern SWC_CAN_INLINE
bool
str_eq(const char* s1, const char* s2) noexcept {
  #if defined(SWC_IMPL_COMPARATORS_BASIC)
    return _str_eq(s1, s2);
  #else
    return !strcmp(s1, s2);
  #endif
}

extern SWC_CAN_INLINE
bool
str_eq(const char* s1, const char* s2, size_t count) noexcept {
  #if defined(SWC_IMPL_COMPARATORS_BASIC)
    return _str_eq(s1, s2, count);
  #else
    return !strncmp(s1, s2, count);
  #endif
}

extern SWC_CAN_INLINE
bool
str_case_eq(const char* s1, const char* s2, size_t count) noexcept {
  #if defined(SWC_IMPL_COMPARATORS_BASIC)
    return _str_case_eq(s1, s2, count);
  #else
    return !strncasecmp(s1, s2, count);
  #endif
}


extern SWC_CAN_INLINE
bool
str_eq(const std::string& s1, const std::string& s2) noexcept {
  return str_eq(s1.c_str(), s2.c_str());
}

extern SWC_CAN_INLINE
bool
mem_eq(const std::string& s1, const std::string& s2) noexcept {
  return s1.length() == s2.length() &&
         mem_eq(reinterpret_cast<const uint8_t*>(s1.c_str()),
                reinterpret_cast<const uint8_t*>(s2.c_str()),
                s1.length());
}


} } // namespace SWC::Condition



#endif // swcdb_core_Comparators_basic_h
