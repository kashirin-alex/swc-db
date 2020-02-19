/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_CellKeyVec_h
#define swcdb_db_cells_CellKeyVec_h


namespace SWC { namespace DB { namespace Cell {

class KeyVec : public std::vector<std::string> {
  public:

  using std::vector<std::string>::vector;

  void free() {
    clear();
  }

  KeyVec operator=(const KeyVec &other) = delete;
  
  void copy(const KeyVec &other) {
    free();
    assign(other.begin(), other.end());
  }

  const bool equal(const KeyVec &other) const {
    return *this == other;
  }

  void add(const char* fraction, const uint32_t len) {
    emplace_back(fraction, len);
  }

  void add(const std::string& fraction) {
    add(fraction.data(), fraction.length());
  }

  void add(const char* fraction) {
    add(fraction, strlen(fraction));
  }

  void add(const uint8_t* fraction, const uint32_t len) {
    add((const char*)fraction, len);
  }


  void insert(const uint32_t idx, const char* fraction, 
              const uint32_t len) {
    emplace(begin()+idx, fraction, len);
  }

  void insert(const uint32_t idx, const std::string& fraction) {
    insert(idx, fraction.data(), fraction.length());
  }

  void insert(const uint32_t idx, const char* fraction) {
    insert(idx, fraction, strlen(fraction));
  }

  void insert(const uint32_t idx, const uint8_t* fraction, 
              const uint32_t len) {
    insert(idx, (const char*)fraction, len);
  }

  void set(const uint32_t idx, const char* fraction, const uint32_t len) {
    auto& r = (*this)[idx];
    r.clear();
    r.append(fraction, len);
  }

  void set(const uint32_t idx, const uint8_t* fraction, const uint32_t len) {
    set(idx, (const char*) fraction, len);
  }

  void set(const uint32_t idx, const std::string& fraction) {
    set(idx, fraction.data(), fraction.length());
  }


  void remove(const uint32_t idx) {
    if(idx >= size())
      return;
    erase(begin()+idx);
  }


  const std::string get(const uint32_t idx) const {
    return (*this)[idx];
  }

  void get(const uint32_t idx, std::string& fraction) const {
    fraction = (*this)[idx];
  }


  const bool align(const KeyVec& other, Condition::Comp comp) {
    bool chg;
    if(chg = empty()) {
      if(chg = !other.empty())
        assign(other.begin(), other.end());
      return chg;
    }
    bool smaller = size() < other.size();
    uint32_t min = smaller ? size() : other.size();
    for(uint32_t c = 0; c < min; ++c) {
      std::string& r1 = (*this)[c];
      const std::string& r2 = other[c];
      if(Condition::condition((const uint8_t*)r1.data(), r1.length(),
                              (const uint8_t*)r2.data(), r2.length()
                              ) == comp) {
        r1 = r2;
        chg = true;
      }
    }
    if(smaller) {
      for(uint32_t c = size(); c < other.size(); ++c) {
        add(other[c]);
        chg = true;
      }
    }
    return chg;
  }


  const uint32_t encoded_length() const {
    uint32_t len = Serialization::encoded_length_vi32(size());
    for(auto it = begin(); it < end(); ++it)
      len += Serialization::encoded_length_vi32(it->length()) + it->length();
    return len;
  }
  
  void encode(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, size());
    uint32_t len;
    for(auto it = begin(); it < end(); ++it) {
      Serialization::encode_vi32(bufp, len = it->length());
      memcpy(*bufp, it->data(), len);
      *bufp += len;
    }
  }

  void decode(const uint8_t **bufp, size_t* remainp) {
    clear();
    resize(Serialization::decode_vi32(bufp, remainp));
    uint32_t len;
    for(auto it = begin(); it < end(); ++it) {
      len = Serialization::decode_vi32(bufp);
      it->append((const char*)*bufp, len);
      *bufp += len;
    }
  }


  const std::string to_string() const {
    std::string s("Key(");
    s.append("sz=");
    s.append(std::to_string(size()));
    s.append(" fractions=[");
    for(auto it = begin(); it < end(); ++it) {
      s.append(*it);
      s.append(", ");
    }
    s.append("])");
    return s;
  }

};

}}}

#endif