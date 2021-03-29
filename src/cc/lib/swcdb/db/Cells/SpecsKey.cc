/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Cells/SpecsKey.h"
#include "swcdb/core/Serialization.h"


namespace SWC { namespace DB { namespace Specs {



Fraction::Fraction(const char* buf, uint32_t len, Condition::Comp comp)
                  : std::string(buf, len), comp(comp) {
}

Fraction::Fraction(std::string&& fraction, Condition::Comp comp) noexcept
                  : std::string(std::move(fraction)), comp(comp) {
}

Fraction::Fraction(const Fraction& other)
                  : std::string(other), comp(other.comp) {
}

Fraction::Fraction(Fraction&& other) noexcept
                  : std::string(std::move(other)), comp(other.comp) {
}

Fraction& Fraction::operator=(const Fraction& other) {
  std::string::operator=(other);
  comp = other.comp;
  return *this;
}

Fraction& Fraction::operator=(Fraction&& other) noexcept {
  std::string::operator=(std::move(other));
  comp = other.comp;
  return *this;
}

Fraction& Fraction::operator=(std::string&& other) noexcept {
  std::string::operator=(std::move(other));
  return *this;
}

Fraction::~Fraction() {
  if(compiled) switch(comp) {
    case Condition::RE:
      delete static_cast<re2::RE2*>(compiled);
      break;
    default: break;
  }
}

bool Fraction::operator==(const Fraction &other) const {
  return other.comp == comp && length() == other.length() &&
         !memcmp(data(), other.data(), length());
}

uint32_t Fraction::encoded_length() const noexcept {
  return 1 + Serialization::encoded_length_vi32(size()) + size();
}

void Fraction::encode(uint8_t** bufp) const {
  Serialization::encode_i8(bufp, comp);
  Serialization::encode_vi32(bufp, size());
  if(!empty()) {
    memcpy(*bufp, data(), size());
    *bufp += size();
  }
}

void Fraction::decode(const uint8_t** bufp, size_t* remainp) {
  clear();
  comp = Condition::Comp(Serialization::decode_i8(bufp, remainp));
  if(uint32_t len = Serialization::decode_vi32(bufp, remainp)) {
    append(reinterpret_cast<const char*>(*bufp), len);
    *bufp += len;
    *remainp -= len;
  }
}

void Fraction::print(std::ostream& out, bool pretty) const {
  out << Condition::to_string(comp) << '"';
  char hex[5];
  hex[4] = 0;
  uint8_t byte;
  for(auto chrp = cbegin(); chrp != cend(); ++chrp) {
    byte = *chrp;
    if(byte == '"')
      out << '\\';
    if(!pretty || (31 < byte && byte < 127)) {
      out << *chrp;
    } else {
      sprintf(hex, "0x%X", byte);
      out << hex;
    }
  }
  out << '"';
}

template<Types::KeySeq T_seq>
SWC_CAN_INLINE
bool
Fraction::_is_matching(const uint8_t* ptr, uint32_t len) {
  switch(comp) {
    case Condition::RE: {
      if(empty())
        return !ptr|| !len;
      if(!compiled)
        compiled = new re2::RE2(re2::StringPiece(data(), size()));
      return Condition::re(
        *static_cast<re2::RE2*>(compiled),
        reinterpret_cast<const char*>(ptr), len
      );
    }
    default:
      return KeySeq::is_matching<T_seq>(
        comp, reinterpret_cast<const uint8_t*>(c_str()), size(), ptr, len);
  }
}



Key::Key() noexcept { }

Key::Key(const Key& other) : std::vector<Fraction>(other) { }

Key::Key(Key&& other) noexcept : std::vector<Fraction>(std::move(other)) { }

Key::~Key() { }

Key::Key(const DB::Cell::Key &cell_key, Condition::Comp comp) {
  set(cell_key, comp);
}

void Key::free() {
  clear();
}

size_t Key::size_of_internal() const noexcept {
  size_t sz = 0;
  for(auto& f : *this) {
    sz += sizeof(f);
    sz += f.size();
  }
  return sz;
}

void Key::copy(const Key &other) {
  clear();
  assign(other.begin(), other.end());
}

Key& Key::operator=(Key&& other) noexcept {
  move(other);
  return *this;
}

void Key::move(Key& other) noexcept {
  std::vector<Fraction>::operator=(std::move(other));
}

bool Key::equal(const Key &other) const noexcept {
  return *this == other;
}

void Key::set(const DB::Cell::Key &cell_key, Condition::Comp comp) {
  clear();
  resize(cell_key.count);

  uint32_t len;
  const uint8_t* ptr = cell_key.data;
  for(auto it=begin(); it != end(); ++it) {
    it->comp = comp;
    if((len = Serialization::decode_vi32(&ptr))) {
      it->append(reinterpret_cast<const char*>(ptr), len);
      ptr += len;
    }
  }
}

void Key::set(int32_t idx, Condition::Comp comp) {
  if(empty())
    return;
  (begin()+idx)->comp = comp;
}


Fraction& Key::add(Fraction&& other) {
  return emplace_back(std::move(other));
}

Fraction& Key::add(std::string&& fraction, Condition::Comp comp) {
  return emplace_back(std::move(fraction), comp);
}

Fraction& Key::add(const char* buf, uint32_t len,
                   Condition::Comp comp) {
  return emplace_back(buf, len, comp);
}

Fraction& Key::add(const std::string& fraction,
                   Condition::Comp comp) {
  return add(fraction.c_str(), fraction.length(), comp);
}

Fraction& Key::add(const std::string_view& fraction,
                   Condition::Comp comp) {
  return add(fraction.data(), fraction.length(), comp);
}

Fraction& Key::add(const char* fraction,
                   Condition::Comp comp) {
  return add(fraction, strlen(fraction), comp);
}

Fraction& Key::add(const uint8_t* fraction, uint32_t len,
                   Condition::Comp comp) {
  return add(reinterpret_cast<const char*>(fraction), len, comp);
}


Fraction& Key::insert(uint32_t idx, Fraction&& other) {
  return *emplace(begin() + idx, std::move(other));
}

Fraction& Key::insert(uint32_t idx, std::string&& fraction,
                      Condition::Comp comp) {
  return *emplace(begin() + idx, std::move(fraction), comp);
}

Fraction& Key::insert(uint32_t idx, const char* buf, uint32_t len,
                      Condition::Comp comp) {
  return *emplace(begin() + idx, buf, len, comp);
}

Fraction& Key::insert(uint32_t idx, const std::string& fraction,
                      Condition::Comp comp) {
  return insert(idx, fraction.c_str(), fraction.length(), comp);
}

Fraction& Key::insert(uint32_t idx, const std::string_view& fraction,
                      Condition::Comp comp) {
  return insert(idx, fraction.data(), fraction.length(), comp);
}

Fraction& Key::insert(uint32_t idx, const uint8_t* fraction, uint32_t len,
                      Condition::Comp comp) {
  return insert(idx, reinterpret_cast<const char*>(fraction), len, comp);
}

Fraction& Key::insert(uint32_t idx, const char* fraction,
                      Condition::Comp comp) {
  return insert(idx, fraction, strlen(fraction), comp);
}


std::string_view Key::get(const uint32_t idx, Condition::Comp& comp) const {
  auto& f = (*this)[idx];
  comp = f.comp;
  return f;
}

std::string_view Key::get(const uint32_t idx) const {
  return (*this)[idx];
}

void Key::get(DB::Cell::Key& key) const {
  key.free();
  if(!empty())
    for(auto it=begin(); it != end(); ++it)
      key.add(*it);
}

void Key::remove(uint32_t idx, bool recursive) {
  if(recursive)
    erase(begin()+idx, end());
  else
    erase(begin()+idx);
}

uint32_t Key::encoded_length() const noexcept {
  uint32_t len = Serialization::encoded_length_vi32(size());
  for(auto it = begin(); it != end(); ++it)
    len += it->encoded_length();
  return len;
}

void Key::encode(uint8_t** bufp) const {
  Serialization::encode_vi32(bufp, size());
  for(auto it = begin(); it != end(); ++it)
    it->encode(bufp);
}

void Key::decode(const uint8_t** bufp, size_t* remainp) {
  clear();
  resize(Serialization::decode_vi32(bufp, remainp));
  for(auto it = begin(); it != end(); ++it)
    it->decode(bufp, remainp);
}


bool Key::is_matching(const Types::KeySeq seq, const Cell::Key &key) {
  switch(seq) {
    case Types::KeySeq::LEXIC:
    case Types::KeySeq::FC_LEXIC:
      return is_matching_lexic(key);
    case Types::KeySeq::VOLUME:
    case Types::KeySeq::FC_VOLUME:
      return is_matching_volume(key);
    default:
      return false;
  }
}

bool Key::is_matching_lexic(const Cell::Key &key) {
  return _is_matching<Types::KeySeq::LEXIC>(key);
}

bool Key::is_matching_volume(const Cell::Key &key) {
  return _is_matching<Types::KeySeq::VOLUME>(key);
}

template<Types::KeySeq T_seq>
SWC_CAN_INLINE
bool
Key::_is_matching(const Cell::Key &key) {
  if(empty())
    return true;
  Condition::Comp comp = Condition::NONE;

  const uint8_t* ptr = key.data;
  uint32_t len;
  auto it = begin();
  for(uint24_t c = key.count; c && it != end(); ++it, --c, ptr += len) {
    len = Serialization::decode_vi24(&ptr);
    if(!it->_is_matching<T_seq>(ptr, len))
      return false;
    comp = it->comp;
  }
  if(size() == key.count || // [,,>=''] spec incl. prior-match
     (size() == key.count + 1 && it->empty() && it->comp == Condition::GE))
    return true;

  switch(comp) {
    case Condition::LT:
    case Condition::LE:
      return empty() || size() > key.count;
    case Condition::GT:
      return empty() || size() < key.count;
    case Condition::GE:
      return empty() || size() < key.count;
    case Condition::PF:
    case Condition::RE:
      return size() < key.count;
    case Condition::NE:
    case Condition::NONE:
      return true;
    default: // Condition::EQ:
      return false;
  }
}


std::string Key::to_string() const {
  std::stringstream ss;
  print(ss);
  return ss.str();
}

void Key::print(std::ostream& out) const {
  out << "Key(";
  if(size())
    display(out << "size=" << size() << " fractions=");
  out << ')';
}

void Key::display(std::ostream& out, bool pretty) const {
  out << '[';
  for(auto it = cbegin(); it != cend(); ) {
    it->print(out, pretty);
    if(++it != cend())
      out << ", ";
  }
  out << "]";
}


}}}
