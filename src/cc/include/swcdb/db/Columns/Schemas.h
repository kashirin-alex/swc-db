/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_db_Columns_Schemas_h
#define swcdb_db_Columns_Schemas_h


#include "swcdb/core/MutexSptd.h"
#include "swcdb/core/Comparators.h"
#include "swcdb/db/Columns/Schema.h"


namespace SWC { namespace DB {

class Schemas : private std::unordered_map<cid_t, Schema::Ptr> {
  using Map = std::unordered_map<cid_t, Schema::Ptr>;

  public:


  struct Pattern : public std::string {

    Condition::Comp comp;

    SWC_CAN_INLINE
    Pattern() noexcept { }

    SWC_CAN_INLINE
    Pattern(Condition::Comp a_comp, std::string&& value) noexcept
            : std::string(std::move(value)), comp(a_comp) {
    }

    SWC_CAN_INLINE
    Pattern(Condition::Comp a_comp, const std::string& value)
            : std::string(value), comp(a_comp) {
    }

    SWC_CAN_INLINE
    Pattern(Pattern&& other) noexcept
            : std::string(std::move(other)), comp(other.comp) {
    }

    SWC_CAN_INLINE
    Pattern(const Pattern& other)
            : std::string(other), comp(other.comp) {
    }

    ~Pattern() noexcept { }

    SWC_CAN_INLINE
    Pattern& operator=(Pattern&& other) noexcept {
      std::string::operator=(std::move(other));
      comp = other.comp;
      return *this;
    }

    SWC_CAN_INLINE
    Pattern& operator=(const Pattern& other) {
      std::string::operator=(other);
      comp = other.comp;
      return *this;
    }

    SWC_CAN_INLINE
    void set(Condition::Comp _comp, std::string&& value) {
      std::string::operator=(std::move(value));
      comp = _comp;
    }

    SWC_CAN_INLINE
    void set(Condition::Comp _comp, const std::string& value) {
      std::string::operator=(value);
      comp = _comp;
    }

    void print(std::ostream& out) const {
      out << Condition::to_string(comp, true) << '"' << *this << '"';
    }

  };
  typedef Core::Vector<Pattern> NamePatterns;

  struct TagsPattern : public NamePatterns {
    Condition::Comp comp;

    SWC_CAN_INLINE
    TagsPattern() noexcept : comp(Condition::NONE) { }

    ~TagsPattern() noexcept { }
  };

  struct SelectorPatterns {
    NamePatterns  names;
    TagsPattern   tags;

    ~SelectorPatterns() noexcept { }
  };



  SWC_CAN_INLINE
  Schemas() noexcept { }

  SWC_CAN_INLINE
  ~Schemas() noexcept { }

  uint64_t size();

  void add(int& err, const Schema::Ptr& schema);

  void remove(cid_t cid);

  void replace(const Schema::Ptr& schema);

  Schema::Ptr get(cid_t cid) noexcept;

  Schema::Ptr get(const std::string& name) noexcept;

  void all(SchemasVec& entries);

  void matching(const SelectorPatterns& patterns, SchemasVec& entries,
                bool no_sys=true);

  void reset();

  protected:

  void _add(int& err, const Schema::Ptr& schema);

  void _remove(cid_t cid);

  void _replace(const Schema::Ptr& schema);

  Schema::Ptr _get(cid_t cid) const noexcept;

  Schema::Ptr _get(const std::string& name) const noexcept;

  Core::MutexSptd  m_mutex;
};



}}


#ifdef SWC_IMPL_SOURCE
#include "swcdb/db/Columns/Schemas.cc"
#endif

#endif // swcdb_db_Columns_Schemas_h
