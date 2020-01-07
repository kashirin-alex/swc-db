/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_cells_TSV_h
#define swcdb_db_cells_TSV_h

#include <vector>

#include "swcdb/core/DynamicBuffer.h"

#include "swcdb/db/Cells/Cell.h"


namespace SWC { namespace DB { namespace Cells {

namespace TSV {

void header_write(Types::Column typ, DynamicBuffer& buffer) {
  std::string header;

  // if(!(out_flag & OUT_NO_TS))
  header.append("TIMESTAMP\t");

  header.append("FLEN\tKEY\t");
  // if(!(out_flag & OUT_NO_VALUE)) {
  if(Types::is_counter(typ))
    header.append("COUNT\tEQ\tSINCE");
  else
    header.append("VLEN\tVALUE");
  // }
  header.append("\n");
  buffer.add(header.data(), header.length());
}

void header_read(const uint8_t **bufp, size_t* remainp, 
                 bool& has_ts, std::vector<std::string>& header) {
  const uint8_t* ptr = *bufp;
  size_t remain = *remainp;

  const uint8_t* s = *bufp;
  while(remain && *ptr != '\n') {
    ptr++;
    remain--;
    if(*ptr == '\t' || *ptr == '\n') {
      header.push_back(std::string((const char*)s, ptr-s));
      s = ptr+1;
    }
  }
    
  if(header.empty())
    return;
  if(!remain || *ptr != '\n') {
    header.clear();
    return;
  }

  has_ts = strncasecmp(header.front().data(), "timestamp", 9) == 0;

  ptr++; // header's newline
  remain--;

  *bufp = ptr;
  *remainp = remain;
}


void write(const Cell &cell, Types::Column typ, DynamicBuffer& buffer) {
  buffer.ensure(1024);

  std::string ts = std::to_string(cell.timestamp);
  buffer.add(ts.data(), ts.length());
  buffer.add("\t", 1); //

  uint32_t len = 0;
  std::string f_len;
  const uint8_t* ptr = cell.key.data;
  for(uint32_t n=1; n<=cell.key.count; n++,ptr+=len) {
    len = Serialization::decode_vi32(&ptr);
    f_len = std::to_string(len);
    buffer.add(f_len.data(), f_len.length());
    if(n < cell.key.count)
      buffer.add(",", 1);
  }
  buffer.add("\t", 1); //
  
  ptr = cell.key.data;
  for(uint32_t n=1; n<=cell.key.count; n++,ptr+=len) {
    len = Serialization::decode_vi32(&ptr);
    if(len)
      buffer.add(ptr, len);
    if(n < cell.key.count)
      buffer.add(",", 1);
  }
  buffer.add("\t", 1); //

  if(Types::is_counter(typ)) {
    uint8_t op;
    int64_t eq_rev = TIMESTAMP_NULL;
    int64_t v = cell.get_counter(op, eq_rev);
    std::string counter = (v > 0 ? "+" : "") + std::to_string(v);
    buffer.add(counter.data(), counter.length());

    if(op & OP_EQUAL) {
      buffer.add("\t", 1); //
      buffer.add("=", 1);
      if(eq_rev != TIMESTAMP_NULL) {
        ts = std::to_string(eq_rev);
        buffer.add("\t", 1); //
        buffer.add(ts.data(), ts.length());
      }
    }
  } else {
    std::string value_length = std::to_string(cell.vlen);
    buffer.add(value_length.data(), value_length.length());
    buffer.add("\t", 1);
    buffer.add(cell.value, cell.vlen);
  }

  buffer.add("\n", 1);
}

const bool read(const uint8_t **bufp, size_t* remainp, 
                bool has_ts, Types::Column typ, Cell &cell) {

  const uint8_t* ptr = *bufp;
  size_t remain = *remainp;
  const uint8_t* s = ptr;

  if(has_ts) {
    while(remain && *ptr != '\t') {
      remain--;
      ++ptr;
    }
    if(!remain)
      return false;
    cell.set_timestamp(std::stoll(std::string((const char*)s, ptr-s)));
    s = ++ptr; // tab
    remain--;
  }

  std::vector<uint32_t> flen;
  while(remain) {
    if(*ptr == ',' || *ptr == '\t') {
      flen.push_back(std::stol(std::string((const char*)s, ptr-s)));
      if(*ptr == '\t')
        break;
      s = ++ptr; // comma
      remain--;
      if(!remain)
        return false;
    }
    ++ptr;
    remain--;
  }
  if(!remain)
    return false;
    
  s = ++ptr; // tab
  remain--;
  for(auto len : flen) {
    if(remain <= len+1) 
      return false;
    cell.key.add(ptr, len);
    ptr += len+1;
    remain -= (len+1);
  };
  if(!remain)
    return false;
      

  s = ptr;
  while(remain && (*ptr != '\t' && *ptr != '\n')) {
    remain--;
    ++ptr;
  }
  if(!remain)
    return false;

  if(Types::is_counter(typ)) {
    int64_t counter = std::stol(std::string((const char*)s, ptr-s));      
    int64_t eq_rev = TIMESTAMP_NULL;
    uint8_t op = 0;
    if(*ptr == '\t') {
      remain--;
      ++ptr; 
      if(!remain)
        return false; 
      if(*ptr == '=')
        op = OP_EQUAL;
      remain--;
      ++ptr;
      
      if(*ptr == '\t') {
        if(!remain)
          return false; 
        s = ++ptr; // tab
        remain--;        
        while(remain && *ptr != '\n') {
          remain--;
          ++ptr;
        }
        if(!remain)
          return false; 
        eq_rev = std::stol(std::string((const char*)s, ptr-s));
      }
    }
    cell.set_counter(op, counter, typ, eq_rev);

  } else {
    cell.vlen = std::stol(std::string((const char*)s, ptr-s));
    ++ptr; // tab
    remain--;
    if(remain < cell.vlen+1) 
      return false;
    cell.value = (uint8_t*)ptr;
    ptr += cell.vlen;
    remain -= cell.vlen;
  }

  if(!remain || *ptr != '\n')
    return false;
      
  ++ptr; // newline
  remain--;

  cell.flag = INSERT;
    
  //display(std::cout); std::cout << "\n";

  *bufp = ptr;
  *remainp = remain;
  return true;
}


} // namespace TSV
}}} // namespace SWC::DB::Cells
#endif