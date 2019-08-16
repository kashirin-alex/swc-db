/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Cells_KeysInterval_h
#define swcdb_db_Cells_KeysInterval_h


namespace SWC { namespace Cells {

class KeysInterval;
typedef std::shared_ptr<KeysInterval> KeysIntervalPtr;

class KeysInterval {

  public:
  KeysInterval() { }

  virtual ~KeysInterval(){ 
    clear_begin();
    clear_end();
  }

  const size_t encoded_length(){
  std::lock_guard<std::mutex> lock(m_mutex);

   return Serialization::encoded_length(begin) 
          + Serialization::encoded_length(end);
  }

  void encode(uint8_t **ptr) {
    std::lock_guard<std::mutex> lock(m_mutex);

    Serialization::encode(begin, ptr);
    Serialization::encode(end, ptr);
  }

  void decode(const uint8_t **ptr, size_t *remain){
    decode_begin(ptr, remain);
    decode_end(ptr, remain);
  }

  void decode_begin(const uint8_t **ptr, size_t *remain){
    std::lock_guard<std::mutex> lock(m_mutex);

    clear_begin();
    Serialization::decode(begin, m_serial_begin, ptr, remain);
  }

  void decode_end(const uint8_t **ptr, size_t *remain){
    std::lock_guard<std::mutex> lock(m_mutex);

    clear_end();
    Serialization::decode(end, m_serial_end,  ptr, remain);
  }

  const std::string to_string(){
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string s("KeysInterval(begin=[");
    for(auto k : begin){
      s.append(Key::to_string(k));
      s.append(",");
    }
    s.append("], end=[");
    for(auto k : end){
      s.append(Key::to_string(k));
      s.append(",");
    }
    s.append("])");
    return s;
  } 

  
  private:

  void clear_begin(){
    if(m_serial_begin != 0 ){
      begin.clear();
      delete [] m_serial_begin;
    }
  }
  
  void clear_end(){
    if(m_serial_end != 0) {
      end.clear();
      delete [] m_serial_end;
    }
  }

  std::mutex     m_mutex;

  uint8_t*       m_serial_begin = 0;
  uint8_t*       m_serial_end   = 0;
  
  ListKeys       begin;
  ListKeys       end;

};

} // Cells namespace

}
#endif