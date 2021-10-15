/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Broker/Protocol/req/Base.h"

#include "swcdb/fs/Broker/Protocol/params/Seek.h"
#include "swcdb/fs/Broker/Protocol/params/Readdir.h"
#include "swcdb/fs/Broker/Protocol/params/Read.h"
#include "swcdb/fs/Broker/Protocol/params/Open.h"
#include "swcdb/fs/Broker/Protocol/params/Length.h"
#include "swcdb/fs/Broker/Protocol/params/Exists.h"
#include "swcdb/fs/Broker/Protocol/params/Append.h"


namespace SWC { namespace Comm { namespace Protocol {
namespace FsBroker {  namespace Req {


bool Base::is_rsp(const Event::Ptr& ev, int cmd,
                  const uint8_t** ptr, size_t* remain) {
  // SWC_LOGF(LOG_DEBUG, "handle: %s", ev->to_str().c_str());
  if(!ev) {
    error = Error::COMM_NOT_CONNECTED;
    *remain = 0;
  } else if(!(error = ev->error)) {
    if(ev->header.command != cmd) {
      error = Error::NOT_IMPLEMENTED;
      *remain = 0;
      SWC_LOGF(LOG_ERROR, "error=%d(%s) cmd=%d",
                error, Error::get_text(error), ev->header.command);
    } else if((!(error = ev->response_code())) || error == Error::FS_EOF) {
      *ptr = ev->data.base + 4;
      *remain = ev->data.size - 4;
    } else {
      *remain = 0;
    }
  } else {
    *remain = 0;
  }
  return true;
}



/* common (Sync & Async) handlers */

void Base::handle_write(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_WRITE, &ptr, &remain))
    return;

  smartfd->fd(-1);
  smartfd->pos(0);

  SWC_FS_WRITE_FINISH(error, smartfd, tracker);
}

void Base::handle_sync(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_SYNC, &ptr, &remain))
    return;

  switch(error) {
    case Error::OK:
      break;
    case EBADR:
    case Error::FS_BAD_FILE_HANDLE:
      smartfd->fd(-1);
    default:
      break;
  }
  SWC_FS_SYNC_FINISH(error, smartfd, tracker);
}

void Base::handle_seek(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_SEEK, &ptr, &remain))
    return;

  switch(error) {
    case Error::OK: {
      try {
        Params::SeekRsp params;
        params.decode(&ptr, &remain);
        smartfd->pos(params.offset);
      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        error = e.code();
      }
      break;
    }
    case EBADR:
    case Error::FS_BAD_FILE_HANDLE:
      smartfd->fd(-1);
    default:
      break;
  }
  SWC_FS_SEEK_FINISH(error, smartfd, tracker);
}

void Base::handle_rmdir(const Event::Ptr& ev, const std::string& name) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_RMDIR, &ptr, &remain))
    return;

  SWC_FS_RMDIR_FINISH(error, name, tracker);
}

void Base::handle_rename(const Event::Ptr& ev,
                         const std::string& from, const std::string& to) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_RENAME, &ptr, &remain))
    return;

  SWC_FS_RENAME_FINISH(error, from, to, tracker);
}

void Base::handle_readdir(const Event::Ptr& ev, const std::string& name,
                          FS::DirentList& results) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_READDIR, &ptr, &remain))
    return;

  if(!error) {
    try {
      Params::ReaddirRsp params;
      params.decode(&ptr, &remain);
      results = std::move(params.listing);

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      error = e.code();
    }
  }
  SWC_FS_READDIR_FINISH(error, name, results.size(), tracker);
}

void Base::handle_remove(const Event::Ptr& ev, const std::string& name) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_REMOVE, &ptr, &remain))
    return;

  SWC_FS_REMOVE_FINISH(error, name, tracker);
}

void Base::handle_read(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd,
                       size_t& amount) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_READ, &ptr, &remain))
    return;

  switch(error) {
    case Error::OK:
    case Error::FS_EOF: {
      try {
        Params::ReadRsp params;
        params.decode(&ptr, &remain);
        amount = ev->data_ext.size;
        smartfd->pos(params.offset + amount);
      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        error = e.code();
      }
      break;
    }
    case EBADR:
    case Error::FS_BAD_FILE_HANDLE:
      smartfd->fd(-1);
    default:
      break;
  }
  SWC_FS_READ_FINISH(error, smartfd, amount, tracker);
}

void Base::handle_read_all(const Event::Ptr& ev, const std::string& name) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_READ_ALL, &ptr, &remain))
    return;

  SWC_FS_READALL_FINISH(error, name, ev->data_ext.size, tracker);
}

void Base::handle_pread(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd,
                        size_t& amount) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_PREAD, &ptr, &remain))
    return;

  switch(error) {
    case Error::OK:
    case Error::FS_EOF: {
      try {
        Params::ReadRsp params;
        params.decode(&ptr, &remain);
        amount = ev->data_ext.size;
        smartfd->pos(params.offset + amount);
      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        error = e.code();
      }
      break;
    }
    case EBADR:
    case Error::FS_BAD_FILE_HANDLE:
      smartfd->fd(-1);
    default:
      break;
  }
  SWC_FS_PREAD_FINISH(error, smartfd, amount, tracker);
}

void Base::handle_combi_pread(const Event::Ptr& ev,
                              const FS::SmartFd::Ptr& smartfd) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_COMBI_PREAD, &ptr, &remain))
    return;

  SWC_FS_COMBI_PREAD_FINISH(error, smartfd, ev->data_ext.size, tracker);
}

void Base::handle_open(const FS::FileSystem::Ptr& fs, const Event::Ptr& ev,
                       FS::SmartFd::Ptr& smartfd) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_OPEN, &ptr, &remain))
    return;

  if(!error) {
    try {
      Params::OpenRsp params;
      params.decode(&ptr, &remain);
      smartfd->fd(params.fd);
      smartfd->pos(0);
      fs->fd_open_incr();

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      error = e.code();
    }
  }
  SWC_FS_OPEN_FINISH(error, smartfd, fs->fds_open(), tracker);
}

void Base::handle_mkdirs(const Event::Ptr& ev, const std::string& name) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_MKDIRS, &ptr, &remain))
    return;

  SWC_FS_MKDIRS_FINISH(error, name, tracker);
}

void Base::handle_length(const Event::Ptr& ev, const std::string& name,
                         size_t& length) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_LENGTH, &ptr, &remain))
    return;

  if(!error) {
    try {
      Params::LengthRsp params;
      params.decode(&ptr, &remain);
      length = params.length;

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      error = e.code();
    }
  }
  SWC_FS_LENGTH_FINISH(error, name, length, tracker);
}

void Base::handle_flush(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_FLUSH, &ptr, &remain))
    return;

  switch(error) {
    case EBADR:
    case Error::FS_BAD_FILE_HANDLE:
      smartfd->fd(-1);
    default:
      break;
  }
  SWC_FS_FLUSH_FINISH(error, smartfd, tracker);
}

void Base::handle_exists(const Event::Ptr& ev, const std::string& name,
                         bool& state) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_EXISTS, &ptr, &remain))
    return;

  if(!error) {
    try {
      Params::ExistsRsp params;
      params.decode(&ptr, &remain);
      state = params.exists;

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      error = e.code();
    }
  }
  SWC_FS_EXISTS_FINISH(error, name, state, tracker);
}

void Base::handle_create(const FS::FileSystem::Ptr& fs, const Event::Ptr& ev,
                         FS::SmartFd::Ptr& smartfd) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_CREATE, &ptr, &remain))
    return;

  if(!error) {
    try {
      Params::OpenRsp params;
      params.decode(&ptr, &remain);
      smartfd->fd(params.fd);
      smartfd->pos(0);
      fs->fd_open_incr();

    } catch(...) {
      const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
      error = e.code();
    }
  }
  SWC_FS_CREATE_FINISH(error, smartfd, fs->fds_open(), tracker);
}

void Base::handle_close(const FS::FileSystem::Ptr& fs, const Event::Ptr& ev,
                        FS::SmartFd::Ptr& smartfd) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_CLOSE, &ptr, &remain))
    return;

  switch(error) {
    case Error::OK:
    case EACCES:
    case ENOENT:
    case EBADR:
    case EBADF:
    case Error::FS_BAD_FILE_HANDLE:
      if(smartfd->invalidate() != -1)
        fs->fd_open_decr();
      break;
    default:
      break;
  }
  SWC_FS_CLOSE_FINISH(error, smartfd, tracker);
}

void Base::handle_append(const Event::Ptr& ev, FS::SmartFd::Ptr& smartfd,
                         size_t& amount) {
  const uint8_t *ptr;
  size_t remain;

  if(!is_rsp(ev, FUNCTION_APPEND, &ptr, &remain))
    return;

  switch(error) {
    case Error::OK: {
      try {
        Params::AppendRsp params;
        params.decode(&ptr, &remain);
        amount = params.amount;
        smartfd->pos(params.offset + amount);
      } catch(...) {
        const Error::Exception& e = SWC_CURRENT_EXCEPTION("");
        error = e.code();
      }
      break;
    }
    case EBADR:
    case Error::FS_BAD_FILE_HANDLE:
      smartfd->fd(-1);
      break;
    default:
      break;
  }
  SWC_FS_APPEND_FINISH(error, smartfd, amount, tracker);
}

}}}}}
