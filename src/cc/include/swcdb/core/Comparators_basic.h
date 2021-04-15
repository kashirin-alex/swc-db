/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_core_Comparators_basic_h
#define swcdb_core_Comparators_basic_h


namespace SWC {

/**
 * @brief The SWC-DB Comparators C++ namespace 'SWC::Condition'
 *
 * \ingroup Core
 */


namespace Condition {

namespace { // local namespace


static int
_memcomp(const uint8_t* s1, const uint8_t* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

SWC_SHOULD_NOT_INLINE
static int
_memcomp(const uint8_t* s1, const uint8_t* s2, size_t count) noexcept {
  for(; count; --count, ++s1, ++s2)
    if(*s1 != *s2)
      return *s1 < *s2 ? -1 : 1;
  return 0;
}


static bool
_memequal(const uint8_t* s1, const uint8_t* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

SWC_SHOULD_NOT_INLINE
static bool
_memequal(const uint8_t* s1, const uint8_t* s2, size_t count) noexcept {
  for(; count; --count, ++s1, ++s2) {
    if(*s1 != *s2)
      return false;
  }
  return true;
}



static int
_strncomp(const char* s1, const char* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

SWC_SHOULD_NOT_INLINE
static int
_strncomp(const char* s1, const char* s2, size_t count) noexcept {
  for(uint8_t b1, b2; count; --count, ++s1, ++s2) {
    if((b1 = *s1) != (b2 = *s2))
      return b1 < b2 ? -1 : 1;
    if(!b1)
      break;
  }
  return 0;
}


static int
_strcomp(const char* s1, const char* s2) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

SWC_SHOULD_NOT_INLINE
static int
_strcomp(const char* s1, const char* s2) noexcept {
  for(uint8_t b1, b2; ; ++s1, ++s2) {
    if((b1 = *s1) != (b2 = *s2))
      return b1 < b2 ? -1 : 1;
    if(!b1)
      break;
  }
  return 0;
}


static bool
_strequal(const char* s1, const char* s2) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

SWC_SHOULD_NOT_INLINE
static bool
_strequal(const char* s1, const char* s2) noexcept {
  for(; ; ++s1, ++s2) {
    if(*s1 != *s2)
      return false;
    if(!*s1)
      return true;
  }
}


static bool
_strnequal(const char* s1, const char* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

SWC_SHOULD_NOT_INLINE
static bool
_strnequal(const char* s1, const char* s2, size_t count) noexcept {
  for(; count; --count, ++s1, ++s2) {
    if(*s1 != *s2)
      return false;
    if(!*s1)
      break;
  }
  return true;
}


static bool
_strncaseequal(const char* s1, const char* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

SWC_SHOULD_NOT_INLINE
static bool
_strncaseequal(const char* s1, const char* s2, size_t count) noexcept {
  for(; count; --count, ++s1, ++s2) {
    if(*s1 - (*s1 > 96 && *s1 < 123 ? 32 : 0) !=
       *s2 - (*s2 > 96 && *s2 < 123 ? 32 : 0))
      return false;
    if(!*s1)
      break;
  }
  return true;
}

} // local namespace




// performance equal to builtins
extern int
memcomp(const uint8_t* s1, const uint8_t* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern SWC_CAN_INLINE
int
memcomp(const uint8_t* s1, const uint8_t* s2, size_t count) noexcept {
  return _memcomp(s1, s2, count);
}


extern bool
memequal(const uint8_t* s1, const uint8_t* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern SWC_CAN_INLINE
bool
memequal(const uint8_t* s1, const uint8_t* s2, size_t count) noexcept {
  return _memequal(s1, s2, count);
}


extern int
strncomp(const char* s1, const char* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern SWC_CAN_INLINE
int
strncomp(const char* s1, const char* s2, size_t count) noexcept {
  return _strncomp(s1, s2, count);
}


extern int
strcomp(const char* s1, const char* s2) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern SWC_CAN_INLINE
int
strcomp(const char* s1, const char* s2) noexcept {
  return _strcomp(s1, s2);
}


extern bool
strequal(const char* s1, const char* s2) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern SWC_CAN_INLINE
bool
strequal(const char* s1, const char* s2) noexcept {
  return _strequal(s1, s2);
}


extern bool
strnequal(const char* s1, const char* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern SWC_CAN_INLINE
bool
strnequal(const char* s1, const char* s2, size_t count) noexcept {
  return _strnequal(s1, s2, count);
}


extern bool
strncaseequal(const char* s1, const char* s2, size_t count) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern SWC_CAN_INLINE
bool
strncaseequal(const char* s1, const char* s2, size_t count) noexcept {
  return _strncaseequal(s1, s2, count);
}


extern bool
eq(const std::string& s1, const std::string& s2) noexcept
  SWC_ATTRIBS((SWC_ATTRIB_O3));

extern SWC_CAN_INLINE
bool
eq(const std::string& s1, const std::string& s2) noexcept {
  // s1.length() == s2.length() &&
  return _strequal(s1.c_str(), s2.c_str());
}


} } // namespace SWC::Condition



#endif // swcdb_core_Comparators_basic_h
