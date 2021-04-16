/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#ifndef swcdb_common_Files_TSV_h
#define swcdb_common_Files_TSV_h


#include "swcdb/core/BufferStream.h"
#include "swcdb/db/Cells/Cell.h"


namespace SWC { namespace DB { namespace Cells {


//! The SWC-DB Tab-Separated-Values C++ namespace 'SWC::DB::Cells::TSV'
namespace TSV {


class FileWriter {
  public:
  int         err = Error::OK;
  std::string base_path;
  std::string file_ext = ".tsv";

  uint64_t    split_size = 1073741824;
  uint8_t     output_flags = 0;
  size_t      cells_count = 0;
  size_t      cells_bytes = 0;
  size_t      file_num = 0;

  FileWriter() { }

  virtual ~FileWriter() { }

  int set_extension(const std::string& ext, int level) {
    if(ext.empty()) {
      _stream.reset(new Core::BufferStreamOut());
      return _stream->error;
    }

    file_ext.append(".");
    file_ext.append(ext);

    if(ext.length() == 3) {
      if(Condition::str_eq("zst", ext.c_str(), ext.length())) {
        _stream.reset(new Core::BufferStreamOut_ZSTD(level));
        return _stream->error;
      }
    }
    return Error::INVALID_ARGUMENT;
  }

  void initialize(const FS::Interface::Ptr& _interface) {
    interface = _interface;
    if(base_path.back() != '/')
      base_path.append("/");
    interface->get_fs()->mkdirs(err, base_path);
  }

  void finalize() {
    for(auto& s : schemas) {
      if(!_stream->empty()) {
        write();
      } else if(!file_num) { // header-only-file
        roll_file();
        if(!err) {
          write_header(s.second->col_type);
          if(!err)
            write();
        }
      }
      if(err)
        break;
    }
    close();
  }

  void write(
        const client::Query::Select::Handlers::BaseUnorderedMap::Ptr& hdlr) {
    for(cid_t cid : hdlr->get_cids()) {
      schemas.emplace(cid, Env::Clients::get()->schemas->get(err, cid));
      if(err)
        break;
    }

    Types::Column col_type;
    do {
      for(cid_t cid : hdlr->get_cids()) {
        col_type = schemas[cid]->col_type;

        cells.free();
        hdlr->get_cells(cid, cells);

        cells_count += cells.size();
        cells_bytes += cells.size_bytes();

        bool need_roll;
        for(auto cell : cells) {
          need_roll = !smartfd ||
                      smartfd->pos() + _stream->available() > split_size;
          if(need_roll) {
            if(!_stream->empty()) {
              write();
              if(err)
                break;
            }
            roll_file();
            if(err)
              break;
            write_header(col_type);
          }

          write(*cell, col_type);
          if(_stream->full()) {
            write();
            if(err)
              break;
          }
        }
      }
    } while(!cells.empty() && !err);
  }

  void write_header(Types::Column typ) {
    std::string header;

    if(!(output_flags & OutputFlag::NO_TS))
      header.append("TIMESTAMP\t");

    header.append("FLEN\tKEY\tFLAG\t");

    if(!(output_flags & OutputFlag::NO_VALUE))
      header.append(Types::is_counter(typ)
        ? "COUNT\tEQ\tSINCE"
        : "ORDER\tVLEN\tVALUE");

    if(has_encoder)
      header.append("\tENCODER");

    header.append("\n");
    _stream->add(reinterpret_cast<const uint8_t*>(header.c_str()), header.size());
    err = _stream->error;
  }

  void write(const Cell &cell, Types::Column typ) {
    DynamicBuffer cell_buff(8096);

    if(!(output_flags & OutputFlag::NO_TS)) {
      cell_buff.add(std::to_string(cell.timestamp));
      cell_buff.add('\t'); //
    }

    uint32_t len = 0;
    const uint8_t* ptr = cell.key.data;
    for(uint32_t n=1; n<=cell.key.count; ++n, ptr+=len) {
      cell_buff.add(std::to_string((len = Serialization::decode_vi32(&ptr))));
      if(n < cell.key.count)
        cell_buff.add(',');
    }
    cell_buff.add('\t'); //

    ptr = cell.key.data;
    for(uint32_t n=1; n<=cell.key.count; ++n, ptr+=len) {
      if((len = Serialization::decode_vi32(&ptr)))
        cell_buff.add(ptr, len);
      if(n < cell.key.count)
        cell_buff.add(',');
    }
    cell_buff.add('\t'); //

    cell_buff.add(DB::Cells::to_string(DB::Cells::Flag(cell.flag)));

    if(output_flags & OutputFlag::NO_VALUE ||
      cell.flag == Flag::DELETE || cell.flag == Flag::DELETE_VERSION) {
      cell_buff.add('\n');
      return;
    }
    cell_buff.add('\t'); //

    if(Types::is_counter(typ)) {
      uint8_t op;
      int64_t eq_rev = TIMESTAMP_NULL;
      int64_t v = cell.get_counter(op, eq_rev);
      cell_buff.add((v > 0 ? "+" : "") + std::to_string(v));

      if(op & OP_EQUAL) {
        cell_buff.add('\t'); //
        cell_buff.add('=');
        if(eq_rev != TIMESTAMP_NULL) {
          cell_buff.add('\t'); //
          cell_buff.add(std::to_string(eq_rev));
        }
      }
    } else {

      cell_buff.add(cell.control & TS_DESC ? 'D' : 'A');
      cell_buff.add('\t'); //

      if(cell.have_encoder() && output_flags & OutputFlag::NO_ENCODE) {
        StaticBuffer v;
        cell.get_value(v);
        cell_buff.add(std::to_string(v.size));
        cell_buff.add('\t');
        cell_buff.add(v.base, v.size);
      } else {
        cell_buff.add(std::to_string(cell.vlen));
        cell_buff.add('\t');
        cell_buff.add(cell.value, cell.vlen);
        if(cell.have_encoder()) {
          has_encoder = true;
          cell_buff.add(std::string("\t1"));
        }
      }
    }

    cell_buff.add('\n');
    _stream->add(cell_buff.base, cell_buff.fill());
    err = _stream->error;
  }

  void roll_file() {
    close();

    std::string filepath(base_path);
    filepath.append(std::to_string(++file_num));
    filepath.append(file_ext);
    smartfd = fds.emplace_back(
      new FS::SmartFd(filepath, FS::OpenFlags::OPEN_FLAG_OVERWRITE));
    while(interface->create(err, smartfd, 0, 0, 0));
    if(!err)
      flush_vol = 0;
  }

  void write() {
    if((flush_vol += _stream->available()) > 1073741824)
      flush_vol = 0;
    StaticBuffer buff_write;
    _stream->get(buff_write);
    err = _stream->error;
    if(err)
      return;
    interface->get_fs()->append(
      err, smartfd, buff_write,
      flush_vol ? FS::Flags::NONE : FS::Flags::FLUSH
    );
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
    if(smartfd && smartfd->valid()) {
      interface->get_fs()->flush(err, smartfd);
      interface->close(err, smartfd);
    }
  }

  FS::Interface::Ptr             interface;
  FS::SmartFd::Ptr               smartfd = nullptr;
  std::vector<FS::SmartFd::Ptr>  fds;
  DB::Cells::Result              cells;
  bool                           has_encoder = false;
  size_t                         flush_vol = 0;
  std::unordered_map<int64_t, DB::Schema::Ptr>  schemas;
  std::unique_ptr<Core::BufferStreamOut>        _stream;

};



class FileReader {
  public:

  int             err = Error::CANCELLED;
  cid_t           cid = DB::Schema::NO_CID;
  std::string     base_path;
  std::string     message;
  DB::Schema::Ptr schema;
  size_t          cells_count = 0;
  size_t          resend_cells = 0;
  size_t          cells_bytes = 0;

  FileReader() { }

  virtual ~FileReader() { }

  void initialize(const FS::Interface::Ptr& _interface) {
    err = Error::OK;
    interface = _interface;
    if(base_path.back() != '/')
      base_path.append("/");

    FS::DirentList entries;
    interface->get_fs()->readdir(err, base_path, entries);
    if(err)
      return;

    std::vector<FS::Dirent*> files;
    files.reserve(entries.size());
    for(auto& entry : entries) {
      if(entry.is_dir)
        continue;
      for(auto it = files.begin(); ; ++it) {
        if(it == files.end() ||
           Condition::lt_volume(
             reinterpret_cast<const uint8_t*>((*it)->name.c_str()),
             (*it)->name.size(),
             reinterpret_cast<const uint8_t*>(entry.name.c_str()),
             entry.name.size() ) ) {
          files.insert(it, &entry);
          break;
        }
      }
    }
    fds.reserve(files.size());
    for(auto file : files)
      fds.emplace_back(new FS::SmartFd(base_path + file->name, 0));
    if(fds.empty()) {
      err = ENOENT;
    } else {
      schema = Env::Clients::get()->schemas->get(err, cid);
    }
  }

  client::Query::Update::Handlers::Common::Ptr read_and_load() {
    if(err)
      return nullptr;

    auto hdlr = client::Query::Update::Handlers::Common::make();
    hdlr->create(schema);

    for(auto& fd : fds) {
      read(hdlr, fd);
      if(err)
        break;
    }

    hdlr->commit_if_need();
    hdlr->wait();
    resend_cells += hdlr->get_resend_count();
    return hdlr;
  }

  void read(const client::Query::Update::Handlers::Common::Ptr& hdlr,
            FS::SmartFd::Ptr smartfd) {
    size_t length = interface->get_fs()->length(err, smartfd->filepath());
    if(err)
      return;

    auto colp = hdlr->get_base_ptr(cid);

    std::unique_ptr<Core::BufferStreamIn> instream;
    const std::string& path = smartfd->filepath();
    if(Condition::str_eq(
        "zst", path.c_str()+(path.length()-3), path.length()-3)) {
      instream.reset(new Core::BufferStreamIn_ZSTD());
    } else {
      instream.reset(new Core::BufferStreamIn());
    }
    err = instream->error;
    if(err)
      return;

    size_t offset = 0;
    size_t r_sz;
    StaticBuffer  buffer;
    bool ok;
    size_t cell_pos = 0;
    size_t cell_mark = 0;
    std::vector<std::string> header;
    bool                     has_ts = false;
    StaticBuffer             buffer_read;

    auto current_bytes = cells_bytes;
    do {
      err = Error::OK;
      if(!smartfd->valid() && !interface->open(err, smartfd) && err)
        break;
      if(err)
        continue;

      r_sz = length - offset > 1048576 ? 1048576 : length - offset;
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
      instream->add(buffer);

      more_available:
      if(!instream->get(buffer_read)) {
        if(offset < length)
          continue;
        break;
      }

      const uint8_t* ptr = buffer_read.base;
      size_t remain = buffer_read.size;

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

      try {
        while(remain) {
          DB::Cells::Cell cell;
          cell_mark = remain;
          ok = read(&ptr, &remain, has_ts, schema->col_type, cell);
          cell_pos += cell_mark-remain;
          if(ok) {
            colp->add(cell);
            ++cells_count;
            cells_bytes += cell.encoded_length();
            if(cells_bytes - current_bytes >= hdlr->buff_sz) {
              hdlr->commit_or_wait(colp);
              current_bytes = cells_bytes;
            }
          } else {
            instream->put_back(ptr, remain);
            break;
          }
        }
        if(!remain)
          goto more_available;
      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        message.append(e.message());
        message.append(", corrupted '");
        message.append(smartfd->filepath());
        message.append("' starting at-offset=");
        message.append(std::to_string(cell_pos));
        message.append("\n");
        offset = length;
        break;
      }
      buffer_read.free();
      resend_cells += hdlr->get_resend_count();
    } while(offset < length);

    if(smartfd->valid())
      interface->close(err, smartfd);

    if(!instream->empty()) {
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

  bool header_read(const uint8_t** bufp, size_t* remainp, Types::Column, //typ
                   bool& has_ts, std::vector<std::string>& header) {
    const uint8_t* ptr = *bufp;
    size_t remain = *remainp;

    const uint8_t* s = *bufp;
    while(remain && *ptr != '\n') {
      ++ptr;
      --remain;
      if(*ptr == '\t' || *ptr == '\n') {
        header.emplace_back(reinterpret_cast<const char*>(s), ptr-s);
        s = ptr+1;
      }
    }

    if(header.empty() || !remain || *ptr != '\n')
      return false;

    has_ts = Condition::str_case_eq(
      header.front().c_str(), "timestamp", 9);
    bool has_encoder = Condition::str_case_eq(
      header.back().c_str(), "encoder", 7);
    if(header.size() < size_t(6 + has_ts + has_encoder))
      return false;

    ++ptr; // header's newline
    --remain;

    *bufp = ptr;
    *remainp = remain;
    return true;
  }

  bool read(const uint8_t** bufp, size_t* remainp, bool has_ts,
            Types::Column typ, Cell &cell) {
    const uint8_t* ptr = *bufp;
    size_t remain = *remainp;
    const uint8_t* s = ptr;

    if(has_ts) {
      while(remain && *ptr != '\t') {
        --remain;
        ++ptr;
      }
      if(!remain)
        return false;
      cell.set_timestamp(
        std::stoll(std::string(reinterpret_cast<const char*>(s), ptr-s)));
      s = ++ptr; // tab
      --remain;
    }

    std::vector<uint32_t> flen;
    while(remain) {
      if(*ptr == ',' || *ptr == '\t') {
        flen.push_back(
          std::stol(std::string(reinterpret_cast<const char*>(s), ptr-s)));
        if(*ptr == '\t')
          break;
        if(!--remain)
          return false;
        s = ++ptr; // comma
      }
      ++ptr;
      --remain;
    }
    if(!remain)
      return false;

    s = ++ptr; // tab
    --remain;
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
      --remain;
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
    --remain;
    while(remain && (*ptr != '\t' && *ptr != '\n')) {
      --remain;
      ++ptr;
    }
    if(!remain)
      return false;

    if(Types::is_counter(typ)) {
      int64_t counter = std::stol(
        std::string(reinterpret_cast<const char*>(s), ptr-s));
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
            --remain;
            ++ptr;
          }
          if(!remain)
            return false;
          eq_rev = std::stol(
            std::string(reinterpret_cast<const char*>(s), ptr-s));
        }
      }
      cell.set_counter(op, counter, typ, eq_rev);

    } else {

      cell.set_time_order_desc(*s == 'D' || *s == 'd');
      if(!--remain)
        return false;
      s = ++ptr; // tab
      while(remain && *ptr != '\t') {
        --remain;
        ++ptr;
      }
      if(!remain)
        return false;

      cell.vlen = std::stol(
        std::string(reinterpret_cast<const char*>(s), ptr-s));
      if(--remain < cell.vlen+1)
        return false;
      ++ptr; // tab
      if(cell.vlen)
        cell.value = const_cast<uint8_t*>(ptr);
      ptr += cell.vlen;
      remain -= cell.vlen;

      if(remain && *ptr == '\t') {
        --remain;
        ++ptr; // tab
        if(!remain)
          return false;
        if(*ptr == '1') {
          cell.control |= HAVE_ENCODER;
          --remain;
          ++ptr;
        }
      }
    }

    cell_filled:
      if(!remain || *ptr != '\n')
        return false;

      *bufp =  ++ptr; // newline
      *remainp = --remain;

    return true;
  }

  private:
  FS::Interface::Ptr              interface;
  std::vector<FS::SmartFd::Ptr>   fds;

};



}}}} // namespace SWC::DB::Cells::TSV


#endif // swcdb_common_Files_TSV_h
