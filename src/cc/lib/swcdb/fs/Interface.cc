/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Interface.h"
#include <dlfcn.h>


namespace SWC { namespace FS {

namespace  { // local ns
void hold_delay() {
  std::this_thread::sleep_for(std::chrono::microseconds(10000));
}
}

Interface::Interface(const Config::Settings::Ptr& settings, Type typ)
                    : m_type(typ),
                      m_fs(use_filesystem(settings)),
                      loaded_dl() {
  SWC_LOGF(LOG_INFO, "INIT-%s", to_string().c_str());
}

FileSystem::Ptr
Interface::use_filesystem(const Config::Settings::Ptr& settings) {
  Configurables config(settings);

  std::string fs_name;
  switch(m_type) {

    case Type::LOCAL:{
      #if defined (BUILTIN_FS_LOCAL) || defined (BUILTIN_FS_ALL)
        return FileSystem::Ptr(new FileSystemLocal(&config));
      #endif
      fs_name.append("local");
      break;
    }

    case Type::BROKER:{
      #if defined (BUILTIN_FS_BROKER) || defined (BUILTIN_FS_ALL)
        return FileSystem::Ptr(new FileSystemBroker(&config));
      #endif
      fs_name.append("broker");
      break;
    }

    case Type::HADOOP: {
      #if defined (BUILTIN_FS_HADOOP) || defined (BUILTIN_FS_ALL)
        return FileSystem::Ptr(new FileSystemHadoop(&config));
      #endif
      fs_name.append("hadoop");
      break;
    }

    case Type::HADOOP_JVM: {
      #if defined (BUILTIN_FS_HADOOP_JVM) || defined (BUILTIN_FS_ALL)
        return FileSystem::Ptr(new FileSystemHadoopJVM(&config));
      #endif
      fs_name.append("hadoop_jvm");
      break;
    }

    case Type::CEPH:{
      #if defined (BUILTIN_FS_CEPH) || defined (BUILTIN_FS_ALL)
        return FileSystem::Ptr(new FileSystemCeph(&config));
      #endif
      fs_name.append("ceph");
      break;
    }

    case Type::CUSTOM: {
      fs_name.append("custom");
      break;
    }

    default:
      SWC_THROWF(Error::CONFIG_BAD_VALUE,
        "Not implemented FileSystem name=%s type=%d",
        fs_name.c_str(), int(m_type));
  }

  std::string cfg_lib("swc.fs.lib." + fs_name);
  std::string fs_lib;
  if(settings->has(cfg_lib.c_str())) {
    fs_lib = settings->get_str(cfg_lib.c_str());
  } else {
    fs_lib.reserve(settings->install_path.length() + 20 + fs_name.length());
    fs_lib.append(settings->install_path);
    fs_lib.append("/lib/libswcdb_fs_"); // (./lib/libswcdb_fs_local.so)
    fs_lib.append(fs_name);
    fs_lib.append(SWC_DSO_EXT);
  }

  const char* err = dlerror();
  void* handle = dlopen(fs_lib.c_str(), RTLD_NOW | RTLD_LAZY | RTLD_LOCAL);
  if (err || !handle)
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "Shared Lib %s, open fail: %s\n",
              fs_lib.c_str(), err);

  err = dlerror();
  std::string handler_name("fs_make_new_" + fs_name);
  void* f_new_ptr = dlsym(handle, handler_name.c_str());
  if (err || !f_new_ptr)
    SWC_THROWF(Error::CONFIG_BAD_VALUE,
              "Shared Lib %s, link(%s) fail: %s handle=%p\n",
              fs_lib.c_str(), handler_name.c_str(), err, handle);

  loaded_dl = {.lib=handle, .make=f_new_ptr};
  return FileSystem::Ptr(
    reinterpret_cast<fs_make_new_t*>(f_new_ptr)(&config));
}


Interface::~Interface() noexcept {
  m_fs = nullptr;
  if(loaded_dl.lib) {
    dlclose(loaded_dl.lib);
  }
}

Type Interface::get_type_underlying() const noexcept {
  return m_fs->get_type_underlying();
}

std::string Interface::to_string() const {
  return format("FS::Interface(type=%d, details=%s)",
                int(m_type), m_fs ? m_fs->to_string().c_str() : "NULL");
}

bool Interface::need_fds() const noexcept {
  return m_fs->need_fds();
}

void Interface::stop() {
  m_fs->stop();
}

void Interface::get_structured_ids(int& err, const std::string& base_path,
                                  IdEntries_t& entries,
                                  const std::string& base_id) {
  //path(base_path) .../111/222/333/444f >> IdEntries_t{(int64_t)111222333444}

  DirentList dirs;
  readdir(err, base_path, dirs);
  if(err)
    return;

  entries.reserve(entries.size() + dirs.size());
  for(auto& entry : dirs) {
    if(!entry.is_dir) continue;

    std::string id_name;
    id_name.reserve(base_id.length() + entry.name.length());
    id_name.append(base_id);
    if(entry.name.back() == ID_SPLIT_LAST) {
      id_name.append(entry.name.substr(0, entry.name.length()-1));
      try {
        entries.push_back(std::stoull(id_name));
      } catch(...){
        SWC_LOGF(LOG_ERROR, "Error converting id_name=%s to uint64",
                  id_name.c_str());
      }
      continue;
    }

    std::string new_base_path;
    new_base_path.reserve(base_path.length() + 1 + entry.name.length());
    new_base_path.append(base_path);
    new_base_path.append("/");
    new_base_path.append(entry.name);
    id_name.append(entry.name);
    get_structured_ids(err, new_base_path, entries, id_name);
    if(err)
      return;
  }
}

// default form to FS methods

void Interface::readdir(int& err, const std::string& base_path,
                        DirentList& dirs) {
  for(;;) {
    dirs.clear();
    m_fs->readdir(err = Error::OK, base_path, dirs);
    switch(err) {
      case Error::OK:
      case ENOENT:
      case Error::SERVER_SHUTTING_DOWN:
        return;
      default:
        hold_delay();
        SWC_LOGF(LOG_WARN, "readdir, retrying to err=%d(%s) dir(%s)",
                  err, Error::get_text(err), base_path.c_str());
    }
  }
}

bool Interface::exists(int& err, const std::string& name) {
  for(bool state;;) {
    state = m_fs->exists(err = Error::OK, name);
    switch(err) {
      case Error::OK:
      case Error::SERVER_SHUTTING_DOWN:
        return state;
      default:
        hold_delay();
        SWC_LOGF(LOG_WARN, "exists, retrying to err=%d(%s) path(%s)",
                  err, Error::get_text(err), name.c_str());
    }
  }
}

void Interface::exists(Callback::ExistsCb_t&& cb,
                       const std::string& name) {
  m_fs->exists([name, cb=std::move(cb), ptr=ptr()]
    (int err, bool state) mutable {
      if(!err || err == Error::SERVER_SHUTTING_DOWN)
        cb(err, state);
      else
        ptr->exists(std::move(cb), name);
    },
    name
  );
}

void Interface::mkdirs(int& err, const std::string& name) {
  for(;;) {
    m_fs->mkdirs(err = Error::OK, name);
    switch(err) {
      case Error::OK:
      case EEXIST:
      case Error::SERVER_SHUTTING_DOWN:
        return;
      default:
        hold_delay();
        SWC_LOGF(LOG_WARN, "mkdirs, retrying to err=%d(%s) dir(%s)",
                  err, Error::get_text(err), name.c_str());
    }
  }
}

void Interface::rmdir(int& err, const std::string& name) {
  for(;;) {
    m_fs->rmdir(err = Error::OK, name);
    switch(err) {
      case Error::OK:
      case ENOENT:
      case Error::SERVER_SHUTTING_DOWN:
        return;
      default:
        hold_delay();
        SWC_LOGF(LOG_WARN, "rmdir, retrying to err=%d(%s) dir(%s)",
                  err, Error::get_text(err), name.c_str());
    }
  }
}

void Interface::rmdir_incl_opt_subs(int& err, const std::string& name,
                                    const std::string& up_to) {
  rmdir(err, name);
  if(err)
    return;

  const char* p=name.data();
  std::string base_path;
  for(const char* c=p+name.length(); c>p; --c) {
    if(*c != '/')
      continue;
    base_path = std::string(p, c-p);
    if(Condition::str_eq(up_to, base_path))
      break;

    DirentList entrs;
    readdir(err, base_path, entrs);
    if(!err && !entrs.size()) {
      rmdir(err, base_path);
      if(!err)
        continue;
    }
    break;
  }
  if(err)
    err = Error::OK;
}

void Interface::remove(int& err, const std::string& name) {
  for(;;) {
    m_fs->remove(err = Error::OK, name);
    switch(err) {
      case Error::OK:
      case ENOENT:
      case Error::SERVER_SHUTTING_DOWN:
        return;
      default:
        hold_delay();
        SWC_LOGF(LOG_WARN, "remove, retrying to err=%d(%s) file(%s)",
                  err, Error::get_text(err), name.c_str());
    }
  }
}

void Interface::remove(Callback::RemoveCb_t&& cb,
                       const std::string& name) {
  m_fs->remove([name, cb=std::move(cb), ptr=ptr()]
    (int err) mutable {
      switch(err) {
        case Error::OK:
        case ENOENT:
        case Error::SERVER_SHUTTING_DOWN:
          return cb(err);
        default: {
          hold_delay();
          SWC_LOGF(LOG_WARN, "remove, retrying to err=%d(%s) file(%s)",
                   err, Error::get_text(err), name.c_str());
          ptr->remove(std::move(cb), name);
        }
      }
    },
    name
  );
}

void Interface::rename(int& err, const std::string& from,
                       const std::string& to) {
  for(;;) {
    m_fs->rename(err = Error::OK, from, to);
    switch(err) {
      case Error::OK:
      case Error::SERVER_SHUTTING_DOWN:
        return;
      case ENOENT: {
        int e_err;
        if(!exists(e_err, from) && !e_err && exists(e_err, to) && !e_err) {
          err = Error::OK;
          return;
        }
        [[fallthrough]];
      }
      case ENOTEMPTY: { // overwrite dir like rename of file
        int e_err;
        rmdir(e_err, to);
        [[fallthrough]];
      }
      default:
        hold_delay();
        SWC_LOGF(LOG_WARN, "rename, retrying to err=%d(%s) from(%s) to(%s)",
                  err, Error::get_text(err), from.c_str(), to.c_str());
    }
  }
}

size_t Interface::length(int& err, const std::string& name) {
  for(size_t length;;) {
    length = m_fs->length(err = Error::OK, name);
    switch(err) {
      case Error::OK:
      case Error::FS_PATH_NOT_FOUND:
      case ENOENT:
      //case Error::FS_PERMISSION_DENIED:
      case Error::SERVER_SHUTTING_DOWN:
        return length;
      default:
        hold_delay();
        SWC_LOGF(LOG_WARN, "length, retrying to err=%d(%s) file(%s)",
                  err, Error::get_text(err), name.c_str());
    }
  }
}


void Interface::write(int& err, SmartFd::Ptr smartfd,
                      uint8_t replication, StaticBuffer& buffer) {
  for(buffer.own=false;;) {
    m_fs->write(err = Error::OK, smartfd, replication, buffer);
    switch(err) {
      case Error::OK:
      case Error::FS_PATH_NOT_FOUND:
      //case Error::FS_PERMISSION_DENIED:
      case Error::SERVER_SHUTTING_DOWN: {
        buffer.own=true;
        return;
      }
      default:
        hold_delay();
        SWC_LOGF(LOG_WARN, "write, retrying to err=%d(%s) file(%s)",
                err, Error::get_text(err), smartfd->filepath().c_str());
    }
  }
}

void Interface::read(int& err, const std::string& name, StaticBuffer* dst) {
  for(;;) {
    m_fs->read(err = Error::OK, name, dst);
    switch(err) {
      case Error::OK:
      case Error::FS_EOF:
      case Error::FS_PATH_NOT_FOUND:
      //case Error::FS_PERMISSION_DENIED:
      case Error::SERVER_SHUTTING_DOWN:
        return;
      default:
        hold_delay();
        SWC_LOGF(LOG_WARN, "read-all, retrying to err=%d(%s) file(%s)",
                err, Error::get_text(err), name.c_str());
    }
  }
}

bool Interface::open(int& err, SmartFd::Ptr& smartfd) {
  m_fs->open(err = Error::OK, smartfd);
  switch(err) {
    case Error::OK:
    case Error::FS_PATH_NOT_FOUND:
    //case Error::FS_PERMISSION_DENIED:
    case Error::SERVER_SHUTTING_DOWN:
      return false;
    default:
      if(smartfd->valid()) {
        int tmperr = Error::OK;
        m_fs->close(tmperr, smartfd);
      }
    return true;
  }
}

bool Interface::create(int& err, SmartFd::Ptr& smartfd, uint8_t replication) {
  m_fs->create(err = Error::OK, smartfd, replication);
  switch(err) {
    case Error::OK:
    case Error::FS_PATH_NOT_FOUND:
    //case Error::FS_PERMISSION_DENIED:
    case Error::SERVER_SHUTTING_DOWN:
      return false;
    default:
      if(smartfd->valid()) {
        int tmperr = Error::OK;
        m_fs->close(tmperr, smartfd);
      }
    return true;
  }
}

void Interface::close(int& err, SmartFd::Ptr& smartfd) {
  while(smartfd->valid()) {
    m_fs->close(err = Error::OK, smartfd);
    switch(err) {
      case Error::OK:
      case EACCES:
      case ENOENT:
      case EBADR:
      case EBADF:
      case Error::FS_BAD_FILE_HANDLE:
      case Error::SERVER_SHUTTING_DOWN:
        return;
      default:
        hold_delay();
        SWC_LOGF(LOG_WARN, "close, retrying to err=%d(%s) file(%s)",
                 err, Error::get_text(err), smartfd->filepath().c_str());
    }
  }
}

void Interface::close(Callback::CloseCb_t&& cb,
                      SmartFd::Ptr& smartfd) {
  m_fs->close(
    [cb=std::move(cb), _smartfd=smartfd, ptr=ptr()] (int err) mutable {
      switch(err) {
        case Error::OK:
        case EACCES:
        case ENOENT:
        case EBADR:
        case EBADF:
        case Error::FS_BAD_FILE_HANDLE:
        case Error::SERVER_SHUTTING_DOWN:
          return cb(err);
        default: {
          if(!_smartfd->valid())
            return cb(EBADR);
          hold_delay();
          SWC_LOGF(LOG_WARN, "close, retrying to err=%d(%s) file(%s)",
                   err, Error::get_text(err), _smartfd->filepath().c_str());
          ptr->close(std::move(cb), _smartfd);
        }
      }
    },
    smartfd
  );
}


void set_structured_id(const std::string& number, std::string& s) {
  auto it = number.cbegin();
  for(size_t n = 0;
      n + ID_SPLIT_LEN < number.length();
      n += ID_SPLIT_LEN, it += ID_SPLIT_LEN) {
    s.reserve(s.length() + ID_SPLIT_LEN + 1);
    s.append(it, it + ID_SPLIT_LEN);
    s.append("/");
  }
  s.append(it, number.cend());
  s += ID_SPLIT_LAST;
}

} // namespace FS



namespace Env {

void FsInterface::init(const SWC::Config::Settings::Ptr& settings,
                       FS::Type typ) {
  m_env.reset(new FsInterface(settings, typ));
}

FsInterface::FsInterface(const SWC::Config::Settings::Ptr& settings,
                         FS::Type typ)
                        : m_interface(new FS::Interface(settings ,typ)) {
}

}


} // namespace SWC
