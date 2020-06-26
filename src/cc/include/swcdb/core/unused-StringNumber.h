/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_core_StringNumber_h
#define swc_core_StringNumber_h

#include <iostream>
#include <string>
#include <cstring>

class StringNumber final {
  public:
  
  StringNumber() : data(0), length(0), negative(false) {}
  
  StringNumber(const std::string& data2) : data(0), length(0), negative(false) {
    equal(data2.data(), data2.length());
  }

  StringNumber(const char* data2) : data(0), length(0), negative(false) {
    equal(data2, strlen(data2));
  }

  ~StringNumber() { 
    if(data)
      delete data;
  }

  StringNumber& operator=(const StringNumber& other){
    equal((const char*)other.data, other.length);
    return *this;
  }

  StringNumber& operator=(const std::string& data2) {
    equal(data2.data(), data2.length());
    return *this;
  }

  StringNumber& operator=(const char* data2) {
    equal(data2, strlen(data2));
    return *this;
  }

  StringNumber& operator+=(const StringNumber& other){
    do_math((const char*)other.data, other.length);
    return *this;
  }

  StringNumber& operator+=(const std::string& data2) {
    do_math(data2.data(), data2.length());
    return *this;
  }

  StringNumber& operator+=(const char* data2) {
    do_math(data2, strlen(data2));
    return *this;
  }

  StringNumber& operator++(int n) {
    plus("1", 0);
    return *this;
  }
  
  StringNumber& operator-=(const StringNumber& other){
    do_math((const char*)other.data, other.length);
    return *this;
  }

  StringNumber& operator-=(const std::string& data2) {
    do_math(data2.data(), data2.length());
    return *this;
  }

  StringNumber& operator-=(const char* data2) {
    do_math(data2, strlen(data2));
    return *this;
  }

  StringNumber& operator--(int n) {
    minus("1", 0);
    return *this;
  }

  void do_math(const std::string& data2) {
    do_math(data2.data(), data2.length());
  }

  void do_math(const char* data2) {
    do_math(data2, strlen(data2));
  }

  void do_math(const char* data2, uint32_t len) {
    if(!len)
      return;

    if(*data2 == '=') {
      equal(++data2, --len);
      return;  
    }

    if(*data2 == '-') {
      data2++;
      len--;
      if(len && !negative)
        minus(data2, --len);
      return;
    } 
    if(*data2 == '+') {
      data2++;
      len--;
    } 
    if(len)
      plus(data2, --len);
  }

  const std::string to_string() const {
    if(data)
      return (negative ? "-" : "") + std::string((char*)data, length);
    return std::string();
  }

  private:

  void equal(const char* data2, uint32_t len) {      
    if(data) 
      delete data;
    
    if(*data2 == '-') {
      data2++;
      negative = --len;
    }

    length = len;
    if(!length) {
      data = new uint8_t[++length];
      *data = 48;
    } else {
      data = new uint8_t[length];
      memcpy(data, data2, length);
    }
  }
  
  void plus(const char* data2, uint32_t at) {
    uint32_t len=at;
    do plus(*(data2+at)-48, len-at);
    while(at--);
  }

  void plus(int16_t num2, uint32_t at) {
    int16_t num1;
    uint32_t sz = length > at+1 ? length : at+1;
    uint8_t* ptr;

    while(true) {
      resize(sz);
      ptr = data+length-1-at;
      num1 = *ptr-48;

      if((num1 += num2) < 10) {
        *ptr = num1 + 48;
        return;
      }
      *ptr = num1 + 38; // - 10 + 48
      num2 = 1;
      if(ptr == data)
        sz++;
      at++;
    }
  }

  void resize(uint32_t size) {
    if(size > length) {
      uint32_t diff = size-length;
      uint8_t* tmp = new uint8_t[size];
      if(data) {
        memcpy(tmp+diff, data, length);
        delete data;
      }
      memset(tmp, '0', diff);
      data = tmp;
      length = size;
    }
  }

  void minus(const char* data2, uint32_t at) {
    uint32_t len=at;
    do minus(*(data2+at)-48, len-at);
    while(at--);
  }

  void minus(int16_t num2, uint32_t at) {
    int16_t num1;
    uint32_t sz = length > at+1 ? length : at+1;
    uint8_t* ptr;

    while(true) {
      resize(sz);
      ptr = data+length-1-at;
      num1 = *ptr-48;

      if((num1 -= num2) >= 0) {
        *ptr = num1 + 48;
        break;
      }
      
      *ptr = 58+num1;
      num2 = 1;

      if(ptr == data) {
        
        break;
      }
      at++;
    }

    if(length > 1) {
      ptr = data;
      at = length;
      while(*ptr == '0') {
        if(--at)
          ptr++;
        else {
          at++;
          break;
        }
      }
      if(ptr != data) {
        uint8_t* tmp = new uint8_t[at];
        memcpy(tmp, ptr, at);
        delete data;
        data = tmp;
        length = at;
      }
    }
  }
  

  uint8_t*  data; 
  uint32_t  length;
  bool      negative;
};





int main() { 

  StringNumber num;  
  
  num.do_math("9999999999999999999999999999999999999999999900000");
    
  std::cout << "   result=" << num.to_string() << "\n";

  for(int n=100; n--; )
    num.do_math("1000");

  std::cout << "   result=" << num.to_string() << "\n";

  num.do_math("100000");
  std::cout << "   result=" << num.to_string() << "\n";

  num.do_math("9999999999999999999999999999999999999999999900000");

  std::cout << "   result=" << num.to_string() << "\n";
  
  StringNumber num2;  
  num2.do_math("1");
  num = num2;;

  std::cout << "   result=" << num.to_string() << "\n";

  StringNumber num3 = "3";
  std::cout << "   result=" << num3.to_string() << "\n";
  num3 = "4";
  std::cout << "   result=" << num3.to_string() << "\n";

  num3 += "9999999999999999999999999999999999999999999996";
  std::cout << "   result=" << num3.to_string() << "\n";
  num3++;num3++;num3++;
  std::cout << "   result=" << num3.to_string() << "\n";

  
  num3--;num3--;num3--;
  std::cout << "   result=" << num3.to_string() << "\n";
  num3--;num3--;num3--;
  std::cout << "   result=" << num3.to_string() << "\n";
  num3 -= "-9999999999999999999999999999999999999999999997";
  std::cout << "   result=" << num3.to_string() << "\n";

  num3--;
  std::cout << "   result=" << num3.to_string() << "\n";

  
  
  num3 = "-1000";
  num3+= "200";
  num3+= "200";
  num3+= "200";
  num3+= "200";
  std::cout << "   result=" << num3.to_string() << "\n";
}

#endif // swc_core_StringNumber_h





