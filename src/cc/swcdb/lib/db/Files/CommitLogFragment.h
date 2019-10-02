/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#ifndef swcdb_db_Files_CommitLogFragment_h
#define swcdb_db_Files_CommitLogFragment_h

#include "swcdb/lib/core/Checksum.h"

namespace SWC { namespace Files { namespace CommitLogFragment {

static const int TRAILER_SIZE=13;
static const int8_t VERSION=1;
static const uint32_t BUFFER_SZ=1048576;

/* file-format: 
    data:   
      cells [cell + i32(checksum)]
*/

inline static void load(int& err, FS::SmartFdPtr smartfd) {
    
    size_t offset = 0;
    DynamicBuffer read_buf;

    for(;;) {
      err = Error::OK;
    
      if(!Env::FsInterface::fs()->exists(err, smartfd->filepath())) {
        if(err != Error::OK && err != Error::SERVER_SHUTTING_DOWN)
          continue;
        return;
      }
      size_t length = Env::FsInterface::fs()->length(err, smartfd->filepath());
      if(err) {
        if(err == Error::SERVER_SHUTTING_DOWN)
          return;
        continue;
      }
      
      
      /*

      Env::FsInterface::fs()->open(err, smartfd);
      if(err == Error::FS_PATH_NOT_FOUND || err == Error::FS_PERMISSION_DENIED)
        break;
      if(!smartfd->valid())
        continue;
      if(err != Error::OK) {
        Env::FsInterface::fs()->close(err, smartfd);
        continue;
      }

      
      for(length > 0; length-=BUFFER_SZ) {
        ptr = read_buf.base;
        offset
        if(Env::FsInterface::fs()->pread(err, smartfd, offset, read_buf.base, sz) != sz){
          if(err != Error::FS_EOF){
            Env::FsInterface::fs()->close(err, smartfd);
            continue;
          }
          break;
        }
        size_t remain = 4;
        int32_t chksum_trailer = Serialization::decode_i32(&ptr, &remain);
        if(!checksum_i32_chk(Serialization::decode_i32(&ptr, &remain), 
                              buf, TRAILER_SIZE)){
          err != Error::CHECKSUM_MISMATCH;
          break;
        }
      }

      */
    } 

    if(smartfd->valid())
      Env::FsInterface::fs()->close(err, smartfd);
    
  }
}}}

#endif