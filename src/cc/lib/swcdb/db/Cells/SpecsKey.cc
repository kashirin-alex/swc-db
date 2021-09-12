/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */



#include "swcdb/db/Cells/SpecsKey.h"


namespace SWC { namespace DB { namespace Specs {



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



Key::Key(const Key& other) : Vec(other) { }

Key::~Key() { }

void Key::copy(const Key &other) {
  clear();
  assign(other.cbegin(), other.cend());
}

bool Key::equal(const Key &other) const noexcept {
  return *this == other;
}

void Key::set(const DB::Cell::Key &cell_key, Condition::Comp comp) {
  clear();
  resize(cell_key.count);

  uint32_t len;
  const uint8_t* ptr = cell_key.data;
  for(auto it=begin(); it != cend(); ++it) {
    it->comp = comp;
    if((len = Serialization::decode_vi32(&ptr))) {
      it->append(reinterpret_cast<const char*>(ptr), len);
      ptr += len;
    }
  }
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


Fraction& Key::insert(uint32_t idx, Fraction&& other) {
  return *Vec::insert(idx, std::move(other));
}

Fraction& Key::insert(uint32_t idx, std::string&& fraction,
                      Condition::Comp comp) {
  return *Vec::insert(idx, std::move(fraction), comp);
}

Fraction& Key::insert(uint32_t idx, const char* buf, uint32_t len,
                      Condition::Comp comp) {
  return *Vec::insert(idx, buf, len, comp);
}


void Key::get(DB::Cell::Key& key) const {
  key.free();
  key.add(*this);
}

void Key::remove(uint32_t idx, bool recursive) {
  if(recursive)
    erase(cbegin()+idx, cend());
  else
    erase(cbegin()+idx);
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
