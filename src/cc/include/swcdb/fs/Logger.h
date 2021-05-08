/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#ifndef swcdb_fs_Logger_h
#define swcdb_fs_Logger_h


// used-locally

#define _SWC_FS_ERR(_error, _cmd, _tracker, _code) \
  SWC_LOG_OUT( \
    ((_error) ? LOG_ERROR : LOG_DEBUG), \
    SWC_LOG_OSTREAM << _cmd; \
    if(_error) Error::print(SWC_LOG_OSTREAM << ' ', _error); \
    _code; \
  ); _tracker.stop(_error);


#define _SWC_FS_FD_ERR(_error, _cmd, _smartfd, _tracker, _code) \
  _SWC_FS_ERR(_error, _cmd, _tracker, \
    _code; \
    _smartfd->print(SWC_LOG_OSTREAM << ' '); \
  );



// paths based

#define SWC_FS_EXISTS_START(_path) \
  SWC_LOG_OUT(LOG_DEBUG, \
    SWC_LOG_OSTREAM << "exists path='" << _path << '\''; \
  );

#define SWC_FS_EXISTS_FINISH(_error, _path, _state, _tracker) \
  _SWC_FS_ERR(_error, "exists", _tracker, \
    SWC_LOG_OSTREAM << " state=" << _state << " path='" << _path << '\''; \
  );



#define SWC_FS_REMOVE_START(_path) \
  SWC_LOG_OUT(LOG_DEBUG, \
    SWC_LOG_OSTREAM << "remove path='" << _path << '\''; \
  );

#define SWC_FS_REMOVE_FINISH(_error, _path, _tracker) \
  _SWC_FS_ERR(_error, "remove", _tracker, \
    SWC_LOG_OSTREAM << " path='" << _path << '\''; \
  );



#define SWC_FS_LENGTH_START(_path) \
  SWC_LOG_OUT(LOG_DEBUG, \
    SWC_LOG_OSTREAM << "length path='" << _path << '\''; \
  );

#define SWC_FS_LENGTH_FINISH(_error, _path, _len, _tracker) \
  _SWC_FS_ERR(_error, "length", _tracker, \
    SWC_LOG_OSTREAM << " len=" << _len << " path='" << _path << '\''; \
  );



#define SWC_FS_MKDIRS_START(_path) \
  SWC_LOG_OUT(LOG_DEBUG, \
    SWC_LOG_OSTREAM << "mkdirs path='" << _path << '\''; \
  );

#define SWC_FS_MKDIRS_FINISH(_error, _path, _tracker) \
  _SWC_FS_ERR(_error, "mkdirs", _tracker, \
    SWC_LOG_OSTREAM << " path='" << _path << '\''; \
  );



#define SWC_FS_READDIR_START(_path) \
  SWC_LOG_OUT(LOG_DEBUG, \
    SWC_LOG_OSTREAM << "readdir path='" << _path << '\''; \
  );

#define SWC_FS_READDIR_FINISH(_error, _path, _sz, _tracker) \
  _SWC_FS_ERR(_error, "readdir", _tracker, \
    SWC_LOG_OSTREAM << " sz=" << _sz << " path='" << _path << '\''; \
  );



#define SWC_FS_RMDIR_START(_path) \
  SWC_LOG_OUT(LOG_DEBUG, \
    SWC_LOG_OSTREAM << "rmdir path='" << _path << '\''; \
  );

#define SWC_FS_RMDIR_FINISH(_error, _path, _tracker) \
  _SWC_FS_ERR(_error, "rmdir", _tracker, \
    SWC_LOG_OSTREAM << " path='" << _path << '\''; \
  );



#define SWC_FS_RENAME_START(_from, _to) \
  SWC_LOG_OUT(LOG_DEBUG, \
    SWC_LOG_OSTREAM << "rename '" << _from << "' to '" << _to << '\''; \
  );

#define SWC_FS_RENAME_FINISH(_error, _from, _to, _tracker) \
  _SWC_FS_ERR(_error, "rename", _tracker, \
    SWC_LOG_OSTREAM << " '" << _from << "' to '" << _to << '\''; \
  );



#define SWC_FS_READALL_START(_name) \
  SWC_LOG_OUT(LOG_DEBUG, \
    SWC_LOG_OSTREAM << "readall file='" << _name << '\''; \
  );

#define SWC_FS_READALL_FINISH(_error, _name, _amount, _tracker) \
  _SWC_FS_ERR(_error, "readall", _tracker, \
    SWC_LOG_OSTREAM << " file='" << _name << "' amt=" << _amount; \
  );



// SmartFd based


#define SWC_FS_CREATE_START(_smartfd, _bufsz, _replication, _blksz) \
  SWC_LOG_OUT(LOG_DEBUG, \
    _smartfd->print( \
      SWC_LOG_OSTREAM << "create bufsz=" << _bufsz \
                      << " replicas=" << int(_replication) \
                      << " blksz=" << _blksz << ' '); \
  );

#define SWC_FS_CREATE_FINISH(_error, _smartfd, _open_fds, _tracker) \
  _SWC_FS_FD_ERR(_error, "create", _smartfd, _tracker, \
    SWC_LOG_OSTREAM << " open-fds=" << _open_fds; \
  );



#define SWC_FS_OPEN_START(_smartfd, _bufsz) \
  SWC_LOG_OUT(LOG_DEBUG, \
    _smartfd->print(SWC_LOG_OSTREAM << "open bufsz=" << _bufsz << ' '); \
  );

#define SWC_FS_OPEN_FINISH(_error, _smartfd, _open_fds, _tracker) \
  _SWC_FS_FD_ERR(_error, "open", _smartfd, _tracker, \
    SWC_LOG_OSTREAM << " open-fds=" << _open_fds; \
  );



#define SWC_FS_WRITE_START(_smartfd, _replication, _blksz, _amount) \
  SWC_LOG_OUT(LOG_DEBUG, \
    _smartfd->print( \
      SWC_LOG_OSTREAM << "write replicas=" << int(_replication) \
                      << " blksz=" << _blksz \
                      << " amt=" << _amount << ' '); \
  );

#define SWC_FS_WRITE_FINISH(_error, _smartfd, _tracker) \
  _SWC_FS_ERR(_error, "write", _tracker, \
    _smartfd->print(SWC_LOG_OSTREAM << ' '); \
  );



#define SWC_FS_READ_START(_smartfd, _amount) \
  SWC_LOG_OUT(LOG_DEBUG, \
    _smartfd->print(SWC_LOG_OSTREAM << "read amt=" << _amount << ' '); \
  );

#define SWC_FS_READ_FINISH(_error, _smartfd, _amount, _tracker) \
  _SWC_FS_FD_ERR( \
    _error && _error == Error::FS_EOF ? Error::OK : _error, \
    "read", _smartfd, _tracker, \
    SWC_LOG_OSTREAM << " amt=" << _amount; \
    if(_error == Error::FS_EOF) SWC_LOG_OSTREAM << "EOF"; \
  );



#define SWC_FS_PREAD_START(_smartfd, _offset, _amount) \
  SWC_LOG_OUT(LOG_DEBUG, \
    _smartfd->print(SWC_LOG_OSTREAM \
      << "pread offset=" << _offset << " amt=" << _amount << ' '); \
  );

#define SWC_FS_PREAD_FINISH(_error, _smartfd, _amount, _tracker) \
  _SWC_FS_FD_ERR( \
    _error && _error == Error::FS_EOF ? Error::OK : _error, \
    "pread", _smartfd, _tracker, \
    SWC_LOG_OSTREAM << " amt=" << _amount; \
    if(_error == Error::FS_EOF) SWC_LOG_OSTREAM << "EOF"; \
  );



#define SWC_FS_COMBI_PREAD_START(_smartfd, _offset, _amount) \
  SWC_LOG_OUT(LOG_DEBUG, \
    _smartfd->print(SWC_LOG_OSTREAM \
      << "combi-pread offset=" << _offset << " amt=" << _amount << ' '); \
  );

#define SWC_FS_COMBI_PREAD_FINISH(_error, _smartfd, _amount, _tracker) \
  _SWC_FS_FD_ERR( \
    _error && _error == Error::FS_EOF ? Error::OK : _error, \
    "combi-pread", _smartfd, _tracker, \
    SWC_LOG_OSTREAM << " amt=" << _amount; \
    if(_error == Error::FS_EOF) SWC_LOG_OSTREAM << "EOF"; \
  );



#define SWC_FS_APPEND_START(_smartfd, _amount, _flags) \
  SWC_LOG_OUT(LOG_DEBUG, \
    _smartfd->print(SWC_LOG_OSTREAM \
      << "append amt=" << _amount << " flags=" << int(_flags) << ' '); \
  );

#define SWC_FS_APPEND_FINISH(_error, _smartfd, _amount, _tracker) \
  _SWC_FS_FD_ERR(_error, "append", _smartfd, _tracker, \
    SWC_LOG_OSTREAM << " amt=" << _amount; \
  );



#define SWC_FS_SEEK_START(_smartfd, _offset) \
  SWC_LOG_OUT(LOG_DEBUG, \
    _smartfd->print(SWC_LOG_OSTREAM << "seek offset=" << _offset << ' '); \
  );

#define SWC_FS_SEEK_FINISH(_error, _smartfd, _tracker) \
  _SWC_FS_FD_ERR(_error, "seek", _smartfd, _tracker, );




#define SWC_FS_FLUSH_START(_smartfd) \
  SWC_LOG_OUT(LOG_DEBUG, \
    _smartfd->print(SWC_LOG_OSTREAM << "flush "); \
  );

#define SWC_FS_FLUSH_FINISH(_error, _smartfd, _tracker) \
  _SWC_FS_FD_ERR(_error, "flush", _smartfd, _tracker, );




#define SWC_FS_SYNC_START(_smartfd) \
  SWC_LOG_OUT(LOG_DEBUG, \
    _smartfd->print(SWC_LOG_OSTREAM << "sync "); \
  );

#define SWC_FS_SYNC_FINISH(_error, _smartfd, _tracker) \
  _SWC_FS_FD_ERR(_error, "sync", _smartfd, _tracker, );




#define SWC_FS_CLOSE_START(_smartfd) \
  SWC_LOG_OUT(LOG_DEBUG, \
    _smartfd->print(SWC_LOG_OSTREAM << "close "); \
  );

#define SWC_FS_CLOSE_FINISH(_error, _smartfd, _tracker) \
  _SWC_FS_FD_ERR(_error, "close", _smartfd, _tracker, );




#endif // swcdb_fs_Logger_h
