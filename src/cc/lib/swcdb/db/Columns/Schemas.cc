/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/Exception.h"
#include "swcdb/db/Columns/Schemas.h"


namespace SWC { namespace DB {


uint64_t Schemas::size()  {
  Core::MutexSptd::scope lock(m_mutex);
  return Map::size();
}

void Schemas::add(int& err, const Schema::Ptr& schema) {
  Core::MutexSptd::scope lock(m_mutex);
  _add(err, schema);
}

void Schemas::_add(int& err, const Schema::Ptr& schema) {
  if(!emplace(schema->cid, schema).second) {
    err = Error::COLUMN_SCHEMA_NAME_EXISTS;
    SWC_LOG_OUT(LOG_WARN,
      schema->print(SWC_LOG_OSTREAM << "Unable to add column ");
      Error::print(SWC_LOG_OSTREAM << ", remove first ", err);
    );
  }
}

void Schemas::remove(cid_t cid) {
  Core::MutexSptd::scope lock(m_mutex);
  _remove(cid);
}

void Schemas::_remove(cid_t cid) {
  auto it = find(cid);
  if(it != cend())
    erase(it);
}

void Schemas::replace(const Schema::Ptr& schema) {
  Core::MutexSptd::scope lock(m_mutex);
  _replace(schema);
}

void Schemas::_replace(const Schema::Ptr& schema) {
  auto res = emplace(schema->cid, schema);
  if(!res.second)
    res.first->second = schema;
}

Schema::Ptr Schemas::get(cid_t cid) noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return _get(cid);
}

Schema::Ptr Schemas::_get(cid_t cid) const noexcept {
  auto it = find(cid);
  return it == cend() ? nullptr : it->second;
}

Schema::Ptr Schemas::get(const std::string& name) noexcept {
  Core::MutexSptd::scope lock(m_mutex);
  return _get(name);
}

Schema::Ptr Schemas::_get(const std::string& name) const noexcept {
  for(const auto& it : *this ) {
    if(Condition::str_eq(name, it.second->col_name))
      return it.second;
  }
  return nullptr;
}

void Schemas::all(std::vector<Schema::Ptr>& entries) {
  entries.reserve(entries.size() + size());
  Core::MutexSptd::scope lock(m_mutex);
  for(const auto& it : *this)
    entries.push_back(it.second);
}


// local-declaration
namespace {
bool is_matching(const Schema::Tags& s_tags,
                 const Schemas::TagsPattern& tags,
                 Core::Vector<bool>& found);
}

void Schemas::matching(const Schemas::SelectorPatterns& patterns,
                       std::vector<Schema::Ptr>& entries,
                       bool no_sys) {
  Core::Vector<bool> found;
  if(patterns.tags.comp == Condition::SBS ||
     patterns.tags.comp == Condition::SPS)
    found.resize(patterns.tags.size());
  bool match;

  Core::MutexSptd::scope lock(m_mutex);
  for(const auto& it : *this) {
    if(no_sys && it.second->cid <= DB::Types::SystemColumn::SYS_CID_END)
      continue;
    const Schema& schema = *it.second.get();
    match = false;
    for(auto& pattern : patterns.names) {
      match = Condition::is_matching_extended(
        pattern.comp,
        reinterpret_cast<const uint8_t*>(pattern.c_str()),
        pattern.size(),
        reinterpret_cast<const uint8_t*>(schema.col_name.c_str()),
        schema.col_name.size()
      );
      if(match)
        break;
    }
    if(match || is_matching(schema.tags, patterns.tags, found))
      entries.push_back(it.second);
  }
}

// local-definition
namespace {
SWC_CAN_INLINE
bool is_matching(const Schema::Tags& s_tags, const Schemas::TagsPattern& tags,
                 Core::Vector<bool>& found) {
  switch(tags.comp) {
    case Condition::NE: {
      auto it = tags.cbegin();
      auto itt = s_tags.cbegin();
      for(; it != tags.cend() && itt != s_tags.cend(); ++it, ++itt) {
        if(!Condition::is_matching_extended(
              it->comp,
              reinterpret_cast<const uint8_t*>(it->c_str()), it->size(),
              reinterpret_cast<const uint8_t*>(itt->c_str()), itt->size())) {
          return true;
        }
      }
      return itt != s_tags.cend() || it != tags.cend();
    }

    case Condition::GT:
    case Condition::LT:
    case Condition::GE:
    case Condition::LE:
    case Condition::EQ: {
      auto it = tags.cbegin();
      auto itt = s_tags.cbegin();
      for(; it != tags.cend() && itt != s_tags.cend(); ++it, ++itt) {
        if(!Condition::is_matching_extended(
              it->comp,
              reinterpret_cast<const uint8_t*>(it->c_str()), it->size(),
              reinterpret_cast<const uint8_t*>(itt->c_str()), itt->size()))
          break;
      }
      return itt != s_tags.cend()
        ? it == tags.cend() &&
          (tags.comp == Condition::GT || tags.comp == Condition::GE)
        : (it == tags.cend()
            ? (tags.comp == Condition::EQ ||
               tags.comp == Condition::LE || tags.comp == Condition::GE)
            : (tags.comp == Condition::LT || tags.comp == Condition::LE));
    }

    case Condition::SBS: {
      uint32_t sz = tags.size();
      if(!sz)
        return true;
      if(sz > s_tags.size())
        return false;
      for(auto& f : found)
        f = false;
      uint32_t count = sz;
      for(auto itt = s_tags.cbegin(); itt != s_tags.cend(); ++itt) {
        for(uint32_t i = 0; i < sz; ++i) {
          if(!found[i] &&
             Condition::is_matching_extended(
              tags[i].comp,
              reinterpret_cast<const uint8_t*>(tags[i].c_str()),
              tags[i].size(),
              reinterpret_cast<const uint8_t*>(itt->c_str()),
              itt->size())) {
            if(!--count)
              return true;
            found[i] = true;
            break;
          }
        }
      }
      return false;
    }

    case Condition::SPS: {
      uint32_t sz = s_tags.size();
      if(!sz)
        return true;
      if(sz > tags.size())
        return false;
      for(auto& f : found)
        f = false;
      uint32_t count = sz;
      for(auto it = tags.cbegin(); it != tags.cend(); ++it) {
        for(uint32_t i = 0; i < sz; ++i) {
          if(!found[i] &&
             Condition::is_matching_extended(
              it->comp,
              reinterpret_cast<const uint8_t*>(it->c_str()),
              it->size(),
              reinterpret_cast<const uint8_t*>(s_tags[i].c_str()),
              s_tags[i].size())) {
            if(!--count)
              return true;
            found[i] = true;
            break;
          }
        }
      }
      return false;
    }

    case Condition::POSBS: {
      auto it = tags.cbegin();
      for(auto itt = s_tags.cbegin();
          itt != s_tags.cend() && it != tags.cend(); ++itt) {
        if(Condition::is_matching_extended(
            it->comp,
            reinterpret_cast<const uint8_t*>(it->c_str()), it->size(),
            reinterpret_cast<const uint8_t*>(itt->c_str()), itt->size()))
          ++it;
      }
      return it == tags.cend();
    }

    case Condition::FOSBS: {
      auto it = tags.cbegin();
      auto itt = s_tags.cbegin();
      for(bool start = false;
          itt != s_tags.cend() && it != tags.cend(); ++itt) {
        if(Condition::is_matching_extended(
            it->comp,
            reinterpret_cast<const uint8_t*>(it->c_str()), it->size(),
            reinterpret_cast<const uint8_t*>(itt->c_str()), itt->size())) {
          start = true;
          ++it;
        } else if(start) {
          return false;
        }
      }
      return it == tags.cend();
    }

    case Condition::POSPS: {
      auto itt = s_tags.cbegin();
      for(auto ord = tags.cbegin();
          itt != s_tags.cend() && ord != tags.cend(); ++itt) {
        for(auto it = ord; ;) {
          if(Condition::is_matching_extended(
              it->comp,
              reinterpret_cast<const uint8_t*>(it->c_str()), it->size(),
              reinterpret_cast<const uint8_t*>(itt->c_str()), itt->size())) {
            ++ord;
            break;
          }
          if(++it == tags.cend())
            return false;
        }
      }
      return itt == s_tags.cend();
    }

    case Condition::FOSPS: {
      auto itt = s_tags.cbegin();
      bool start = false;
      for(auto ord = tags.cbegin();
          itt != s_tags.cend() && ord != tags.cend(); ++itt) {
        for(auto it = ord; ;) {
          if(Condition::is_matching_extended(
              it->comp,
              reinterpret_cast<const uint8_t*>(it->c_str()), it->size(),
              reinterpret_cast<const uint8_t*>(itt->c_str()), itt->size())) {
            start = true;
            ord = ++it;
            break;
          } else if(start) {
            return false;
          }
          if(++it == tags.cend())
            return false;
        }
      }
      return itt == s_tags.cend();
    }

    default:
      break;
  }
  return false;
}
}


void Schemas::reset() {
  Core::MutexSptd::scope lock(m_mutex);
  clear();
}

}}
