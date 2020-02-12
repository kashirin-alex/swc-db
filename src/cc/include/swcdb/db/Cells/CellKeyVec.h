/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_CellKeyVec_h
#define swcdb_db_cells_CellKeyVec_h


namespace SWC { namespace DB { namespace Cell {

class KeyVec {
  public:

  KeyVec() { }

  KeyVec(const KeyVec &other) : key(other.key) { }

  virtual ~KeyVec() { }

  void free() {
    key.clear();
  }
  
  const uint32_t size() const {
    return key.size();
  }

  const bool equal(const KeyVec &other) const {
    return key == other.key;
  }

  const bool empty() const {
    return key.empty();
  }


  void add(const char* fraction, const uint32_t len) {
    key.emplace_back(fraction, strlen(fraction));
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
    key.emplace(key.begin()+idx, fraction, len);
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
    key[idx].clear();
    key[idx].append(fraction, len);
  }

  void set(const uint32_t idx, const uint8_t* fraction, const uint32_t len) {
    set(idx, (const char*) fraction, len);
  }

  void set(const uint32_t idx, const std::string& fraction) {
    set(idx, fraction.data(), fraction.length());
  }


  void remove(const uint32_t idx) {
    if(idx >= key.size())
      return;
    key.erase(key.begin()+idx);
  }


  const std::string get(const uint32_t idx) const {
    return idx >= key.size() ? std::string() : key[idx];
  }

  void get(const uint32_t idx, std::string& fraction) const {
    fraction.clear();
    fraction.append(idx >= key.size() ? std::string() : key[idx]);
  }


  const bool align_start(const KeyVec& other) {
    bool chg = false;
    uint32_t min = size() < other.size() ? size() : other.size();
    for(uint32_t c = 0; c < min; ++c) {
      if(Condition::condition(
                  (const uint8_t*)other.key[c].data(), other.key[c].length(),
                  (const uint8_t*)key[c].data(), key[c].length()
              ) == Condition::GT) {
        set(c, other.key[c]);
        chg = true;
      }
    }
    if(size() < other.size()) {
      for(uint32_t c = size(); c < other.size(); ++c) {
        add("", 0);
        chg = true;
      }
    }
    return chg;
  }
 
  const bool align_finish(const KeyVec& other) {
    bool chg = false;
    uint32_t min = size() < other.size() ? size() : other.size();
    for(uint32_t c = 0; c < min; ++c) {
      if(Condition::condition(
                  (const uint8_t*)other.key[c].data(), other.key[c].length(),
                  (const uint8_t*)key[c].data(), key[c].length()
              ) == Condition::LT) {
        set(c, other.key[c]);
        chg = true;
      }
    }
    if(size() < other.size()) {
      for(uint32_t c = size(); c < other.size(); ++c) {
        add(other.key[c]);
        chg = true;
      }
    }
    return chg;
  }


  const uint32_t encoded_length() const {
    uint32_t len = Serialization::encoded_length_vi32(key.size());
    for(auto& f : key)
      len += Serialization::encoded_length_vi32(f.length()) + f.length();
    return len;
  }
  
  void encode(uint8_t **bufp) const {
    Serialization::encode_vi32(bufp, key.size());
    for(auto& f : key) { 
      Serialization::encode_vi32(bufp, f.length());
      memcpy(*bufp, f.data(), f.length());
      *bufp += f.length();
    } 
  }

  void decode(const uint8_t **bufp, size_t* remainp) {
    key.clear();
    uint32_t count = Serialization::decode_vi32(bufp, remainp);
    key.resize(count);
    uint32_t len;
    for(uint32_t n=0; n<count; ++n) {
      len = Serialization::decode_vi32(bufp);
      key[n].append((const char*)*bufp, len);
      *bufp += len;
    }
  }


  const std::string to_string() const {
    std::string s("Key(");
    s.append("sz=");
    s.append(std::to_string(key.size()));
    s.append(" fractions=[");
    for(auto& f : key) {
      s.append(f);
      s.append(", ");
    }
    s.append("])");
    return s;
  }
  
  std::vector<std::string> key;

};

}}}

#endif