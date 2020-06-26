/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swc_core_NumberArray_h
#define swc_core_NumberArray_h

#include <iostream>
#include <string>
#include <cstring>

/*
0 000 000 000 000 000 000
1000^6+1
9999999999999999999
*/

class NumberArray final {
  public:
  
  NumberArray() : data(0), length(0), negative(false) {}
   
  ~NumberArray() { 
    if(data)
      delete data;
  }
  
  bool resize(uint32_t size) {
    if(size <= length) 
      return false;

    uint32_t diff = size-length;
    uint8_t* tmp = new uint8_t[size];
    if(data) {
      memcpy(tmp+diff, data, length);
      delete data;
    }
    memset(tmp, 0, diff);
    data = tmp;
    length = size;
    return true;
  }

  void plus(const uint8_t* data2, uint32_t len, bool neg=false) {
    if(!len)
      return;

    uint32_t sz = len > length ? len : length;
    resize(sz);
    uint32_t at = sz; 
    uint32_t at2 = len; 

    int16_t num = 0;
    int16_t nxt = 0;

    while(at2-- && at--) {
        
      if(neg) {
        num -= *(data + at) - *(data2 + at2);

      } else {
        num += *(data + at) + *(data2 + at2) + nxt;
        nxt = 0;
        
        std::cout << " at " << at2 << " num=" << num<< " d=" << (int)*(data2 + at2) << "\n";
        if(num > 255) {
          *(data + at) = 255;
          do nxt++;
          while((num -= 255) > 255);
          if(resize(sz++))
            at++;
        } else {
          *(data + at) = num;
          num = 0;
        }
        if(nxt && !at) {
          resize(sz++);
          *(data) += nxt;
          break;
        }
      }
    }
  }

  const std::string to_string() const {
    if(!data || !length)
      return std::string();

    std::cout << "\n";
    std::string s(negative ? "-" : "");
    uint8_t* end = data+length-1;
    int64_t num = 0;
    uint8_t* c = ((uint8_t*)&num)+7;
    do {
      *c = *end;
      c--; 
      std::cout << "at=" << (size_t)(end-data)<< " d=" << (int)*end << "\n";
    } while(data != end--);
    
    s.append(std::to_string(num));
    return s;
  }

  uint8_t*  data; 
  uint32_t  length;
  bool      negative;
};





int main() { 

  NumberArray num;  
  
  uint32_t a = 255;
  num.plus((const uint8_t*)&a, 4);
  
  a = 255;
  num.plus((const uint8_t*)&a, 4);
    
  a = 255;
  num.plus((const uint8_t*)&a, 4);
    
  std::cout << "   result=" << num.to_string() << "\n";
}

#endif // swc_core_NumberArray_h





