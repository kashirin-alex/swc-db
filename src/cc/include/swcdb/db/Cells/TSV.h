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
  
std::string get_filepath(const std::string& base_path, size_t& file_num) {
  std::string filepath(base_path);
  filepath.append(std::to_string(++file_num));
  filepath.append(file_num == 1 ? ".tsv" : ".tsv.part");
  return filepath;
}

class FileWriter {
  public:
  int         err = Error::OK;
  std::string base_path;
  uint8_t     output_flags = 0;
  size_t      cells_count = 0;
  size_t      cells_bytes = 0;
  size_t      file_num = 0;

  FileWriter(FS::Interface::Ptr interface) : interface(interface) {}

  virtual ~FileWriter() { }

  void initialize() {
    if(base_path.back() != '/')
      base_path.append("/");
    interface->get_fs()->mkdirs(err, base_path);
  }

  void finalize() {
    for(auto s : schemas)
      write(s.second->col_type);
    close();
  }

  void write(Protocol::Common::Req::Query::Select::Result::Ptr result) {
    for(auto cid : result->get_cids()) {
      schemas.insert(
        std::make_pair(cid, Env::Clients::get()->schemas->get(err, cid)));
      if(err)
        break;
    }

    Types::Column col_type;
    do {
      for(auto cid : result->get_cids()) {
        col_type = schemas[cid]->col_type;

        cells.free();
        result->get_cells(cid, cells);
        for(auto& cell : cells.cells) {

          cells_count++;
          cells_bytes += cell->encoded_length();
          
          write(*cell, col_type);
          
          delete cell;
          cell = nullptr;

          if(buffer.fill() >= 8388608) {
            write(col_type);
            if(err)
              break;
          }
        }
      }
    } while(!cells.cells.empty() && !err);
  }

  void write(const Cell &cell, Types::Column typ) {
    buffer.ensure(1024);

    std::string ts;
    if(!(output_flags & OutputFlag::NO_TS)) {
      ts = std::to_string(cell.timestamp);
      buffer.add(ts.data(), ts.length());
      buffer.add("\t", 1); //
    }

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

    std::string flag = DB::Cells::to_string((DB::Cells::Flag)cell.flag);
    buffer.add(flag.data(), flag.length());

    if(output_flags & OutputFlag::NO_VALUE || 
      cell.flag == Flag::DELETE || cell.flag == Flag::DELETE_VERSION) {
      buffer.add("\n", 1);
      return;
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
  
      buffer.add(cell.control & TS_DESC ? "D" : "A", 1);
      buffer.add("\t", 1); //

      std::string value_length = std::to_string(cell.vlen);
      buffer.add(value_length.data(), value_length.length());
      buffer.add("\t", 1);
      buffer.add(cell.value, cell.vlen);
    }

    buffer.add("\n", 1);
  }

  void write_header(Types::Column typ, DynamicBuffer& header_buffer) {
    std::string header;

    if(!(output_flags & OutputFlag::NO_TS))
      header.append("TIMESTAMP\t");

    header.append("FLEN\tKEY\tFLAG\t");

    if(!(output_flags & OutputFlag::NO_VALUE))
      header.append(Types::is_counter(typ) 
        ? "COUNT\tEQ\tSINCE" 
        : "ORDER\tVLEN\tVALUE");

    header.append("\n");
    header_buffer.add(header.data(), header.length());
  }

  void write(Types::Column typ) {

    if(smartfd == nullptr || smartfd->pos() + buffer.fill() > 4294967296) {
      close();

      smartfd = FS::SmartFd::make_ptr(
        get_filepath(base_path, file_num), FS::OpenFlags::OPEN_FLAG_OVERWRITE);
      while(interface->create(err, smartfd, 0, 0, 0));
      if(err) 
        return;

      DynamicBuffer header_buffer;
      write_header(typ, header_buffer);
      StaticBuffer buff_write(header_buffer);
      interface->get_fs()->append(err, smartfd, buff_write, FS::Flags::NONE);
      if(err) 
        return;
      fds.push_back(smartfd);
    }
    
    if(buffer.fill()) {
      StaticBuffer buff_write(buffer);
      interface->get_fs()->append(err, smartfd, buff_write, FS::Flags::NONE);
    }
  }
  
  void get_length(std::vector<FS::SmartFd::Ptr>& files) {
    for(auto fd : fds) {
      fd->pos(interface->get_fs()->length(err, fd->filepath()));
      files.push_back(fd);
      if(err)
        break;
    }
  }

  private:

  void close() {
    if(smartfd != nullptr && smartfd->valid()) {
      interface->get_fs()->flush(err, smartfd);
      interface->close(err, smartfd);
    }
  }

  FS::Interface::Ptr             interface;
  FS::SmartFd::Ptr               smartfd = nullptr;
  std::vector<FS::SmartFd::Ptr>  fds;
  DynamicBuffer                  buffer;
  DB::Cells::Vector              cells; 
  std::unordered_map<int64_t, DB::Schema::Ptr>  schemas;

};



class FileReader {
  public:
  
  int             err = Error::OK;
  int64_t         cid = DB::Schema::NO_CID;
  std::string     base_path;
  std::string     message;
  DB::Schema::Ptr schema;
  size_t          cells_count = 0;
  size_t          cells_bytes = 0;

  FileReader(FS::Interface::Ptr interface) : interface(interface) {}

  virtual ~FileReader() { }

  void initialize() {
    if(base_path.back() != '/')
      base_path.append("/");

    size_t file_num = 0;
    std::string filepath;
    do {
      filepath = get_filepath(base_path, file_num);
      if(!interface->exists(err, filepath))
        break;
      fds.push_back(FS::SmartFd::make_ptr(filepath, 0));
    } while(!err);

    if(fds.empty())
      err = ENOENT;
    else
      schema = Env::Clients::get()->schemas->get(err, cid);
  }

  void read_and_load() {
    initialize();
    if(err)
      return;

    auto updater = std::make_shared<Protocol::Common::Req::Query::Update>();
    updater->columns->create(schema);
    
    for(auto& fd : fds) {
      read(updater, fd);
      if(err)
        break;
    }

    auto col = updater->columns->get_col(cid);
    if(col->size_bytes() && !updater->result->completion)
      updater->commit(col);
    
    updater->wait();
  }

  void read(Protocol::Common::Req::Query::Update::Ptr updater, 
            FS::SmartFd::Ptr smartfd) {
    size_t length = interface->get_fs()->length(err, smartfd->filepath());
    if(err)
      return;

    size_t offset = 0;
    size_t r_sz;
    
    auto col = updater->columns->get_col(cid);
    StaticBuffer  buffer;
    DynamicBuffer buffer_remain;
    DynamicBuffer buffer_write;
    bool ok;
    size_t s_sz;
    size_t cell_pos = 0;
    size_t cell_mark = 0;
    std::vector<std::string> header;
    bool                     has_ts;

    do {
      err = Error::OK;
      if(!smartfd->valid() && !interface->open(err, smartfd) && err)
        break;
      if(err)
        continue;
      
      r_sz = length - offset > 8388608 ? 8388608 : length - offset;
      for(;;) {
        buffer.free();
        err = Error::OK;

        if(interface->get_fs()->pread(
            err, smartfd, offset, &buffer, r_sz) != r_sz) {
          int tmperr = Error::OK;
          interface->close(tmperr, smartfd);
          if(err != Error::FS_EOF){
            if(!interface->open(err, smartfd))
              break;
            continue;
          }
        }
        break;
      }
      if(err)
        break;
      offset += r_sz;
      
      if(buffer_remain.fill()) {
        buffer_write.add(buffer_remain.base, buffer_remain.fill());
        buffer_write.add(buffer.base, buffer.size);
        buffer_remain.free();
      } else {
        buffer_write.set(buffer.base, buffer.size);
      }

      const uint8_t* ptr = buffer_write.base;
      size_t remain = buffer_write.fill();
      
      if(header.empty() && 
         !header_read(&ptr, &remain,  schema->col_type, has_ts, header)) {
        message.append("TSV file '");
        message.append(smartfd->filepath());
        message.append("' missing ");
        message.append(
          header.empty() ? "columns defintion" : "value columns");
        message.append(" in header\n");
        break;
      }

      while(remain) {
        DB::Cells::Cell cell;
        try {
          cell_mark = remain;
          ok = read(&ptr, &remain, has_ts, schema->col_type, cell);
          cell_pos += cell_mark-remain;
        } catch(const std::exception& ex) {
          message.append(ex.what());
          message.append(", corrupted '");
          message.append(smartfd->filepath());
          message.append("' starting at-offset=");
          message.append(std::to_string(cell_pos));
          message.append("\n");
          offset = length;
          break;
        }
        if(ok) {
          col->add(cell);
          cells_count++;
          cells_bytes += cell.encoded_length();
          
          s_sz = col->size_bytes();
          if(updater->result->completion && s_sz > updater->buff_sz*3)
            updater->wait();
          if(!updater->result->completion && s_sz >= updater->buff_sz)
            updater->commit(col);
          
        } else  {
          buffer_remain.add(ptr, remain);
          break;
        }
      }
      
      buffer_write.free();
      buffer.free();

    } while(offset < length);

    if(smartfd->valid())
      interface->close(err, smartfd);
    
    if(buffer_remain.fill()) {
      message.append("early file end");
      message.append(", corrupted '");
      message.append(smartfd->filepath());
      message.append("' starting at-offset=");
      message.append(std::to_string(cell_pos));
      message.append("\n");
    }
    if(!message.empty())
      err = Error::SQL_BAD_LOAD_FILE_FORMAT;
  }

  const bool header_read(const uint8_t **bufp, size_t* remainp, 
                         Types::Column typ, 
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
    
    if(header.empty() || !remain || *ptr != '\n')
      return false;

    has_ts = strncasecmp(header.front().data(), "timestamp", 9) == 0;
    if(header.size() < 6 + has_ts)
      return false;

    ptr++; // header's newline
    remain--;

    *bufp = ptr;
    *remainp = remain;
    return true;
  }
  
  const bool read(const uint8_t **bufp, size_t* remainp, 
                  bool has_ts, Types::Column typ, 
                  Cell &cell) {
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
        if(!--remain)
          return false;
        s = ++ptr; // comma
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
    if((cell.flag = DB::Cells::flag_from(s, ptr-s)) == Flag::NONE)
      throw std::runtime_error("Bad cell Flag");
  
    if(cell.flag == Flag::DELETE || cell.flag == Flag::DELETE_VERSION) {
      if(*ptr == '\t')
        throw std::runtime_error("Expected end of line");
      goto cell_filled;
    }
    if(*ptr == '\n')
      throw std::runtime_error("Expected a tab");

    s = ++ptr; // tab
    remain--;
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
        if(!--remain)
          return false; 
        ++ptr;  // tab
        if(*ptr != '=')
          throw std::runtime_error("Expected EQ symbol");
        op = OP_EQUAL;
        if(!--remain)
          return false;
        ++ptr;
      
        if(*ptr == '\t') {
          if(!--remain)
            return false; 
          s = ++ptr; // tab
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
    
      cell.set_time_order_desc(*s == 'D' || *s == 'd');
      if(!--remain)
        return false;
      s = ++ptr; // tab
      while(remain && *ptr != '\t') {
        remain--;
        ++ptr;
      }
      if(!remain)
        return false;

      cell.vlen = std::stol(std::string((const char*)s, ptr-s));
      if(--remain < cell.vlen+1) 
        return false;
      ++ptr; // tab
      if(cell.vlen)
        cell.value = (uint8_t*)ptr;
      ptr += cell.vlen;
      remain -= cell.vlen;
    }

    cell_filled:
      if(!remain || *ptr != '\n')
        return false;
      
      ++ptr; // newline
      remain--;

    
      //display(std::cout); std::cout << "\n";

      *bufp = ptr;
      *remainp = remain;

    return true;
  }

  private:
  FS::Interface::Ptr              interface;
  std::vector<FS::SmartFd::Ptr>   fds;

};

} // namespace TSV
}}} // namespace SWC::DB::Cells
#endif