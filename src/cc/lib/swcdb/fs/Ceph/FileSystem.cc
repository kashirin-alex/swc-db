/*
 * Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/fs/Ceph/FileSystem.h"

extern "C" {
#include <dirent.h>
}

namespace SWC{ namespace FS {


Config apply_ceph() {
  Env::Config::settings()->file_desc.add_options()
    ("swc.fs.ceph.path.root", str(""), 
      "Ceph FileSystem's base root path")
    ("swc.fs.ceph.cfg.dyn", strs(), 
      "Dyn-config file")

    ("swc.fs.ceph.client.id", str(), 
      "This ceph Client Id(name)")

    ("swc.fs.ceph.perm.group", i32(), 
      "CephFs Permission Group")
    ("swc.fs.ceph.perm.user", i32(), 
      "CephFs Permission User")
      
    ("swc.fs.ceph.stripe.unit", i32(), 
      "CephFs default stripe_unit")
    ("swc.fs.ceph.stripe.count", i32(), 
      "CephFs default stripe_count")
    ("swc.fs.ceph.object.size", i32(), 
      "CephFs default object_size")
    ("swc.fs.ceph.replication", i32(), 
      "CephFs default replication")

    ("swc.fs.ceph.configuration.file", str(), 
      "The ceph configuration file 'ceph.conf'")
    ("swc.fs.ceph.mon.addr", str(), 
      "The ceph config value for 'mon_addr'")

    ("swc.fs.ceph.fds.max", g_i32(256), 
      "Max Open Fds for opt. without closing")
  ;

  Env::Config::settings()->parse_file(
    Env::Config::settings()->get_str("swc.fs.ceph.cfg", ""),
    "swc.fs.ceph.cfg.dyn"
  );
  
  Config config;
  config.path_root = Env::Config::settings()->get_str(
    "swc.fs.ceph.path.root");
  config.cfg_fds_max = Env::Config::settings()->get<Property::V_GINT32>(
    "swc.fs.ceph.fds.max");
  return config;
}




FileSystemCeph::FileSystemCeph() 
        : FileSystem(apply_ceph()),
          m_filesystem(nullptr), m_perm(nullptr),
          m_run(true) { 
  setup_connection();
}

void FileSystemCeph::setup_connection() {
  auto settings = Env::Config::settings();

  uint32_t tries=0; 
  while(m_run.load() && !initialize()) {
    SWC_LOGF(LOG_ERROR, 
      "FS-Ceph, unable to initialize connection to Ceph, try=%d",
      ++tries);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  
  // status check
  int err;
  const char* cwd = ceph_getcwd(m_filesystem);
  if(path_root.compare(cwd) != 0) {
      SWC_LOGF(LOG_WARN, 
        "FS-Ceph, ceph_cwd='%s' != path_root='%s' changing",
        cwd, path_root.c_str());
    err = ceph_chdir(m_filesystem, path_root.c_str());
    SWC_ASSERT(!err);
  }
 
  if(settings->has("swc.fs.ceph.stripe.unit"))
    ceph_set_default_file_stripe_unit(
      m_filesystem, settings->get_i32("swc.fs.ceph.stripe.unit"));
 
  if(settings->has("swc.fs.ceph.stripe.count"))
    ceph_set_default_file_stripe_count(
      m_filesystem, settings->get_i32("swc.fs.ceph.stripe.count"));

  if(settings->has("swc.fs.ceph.object.size"))
    ceph_set_default_object_size(
      m_filesystem, settings->get_i32("swc.fs.ceph.object.size"));

  if(settings->has("swc.fs.ceph.replication"))
    ceph_set_default_file_replication(
      m_filesystem, settings->get_i32("swc.fs.ceph.replication"));
  
  /*
    ceph_cfg_min_obj_sz = ceph_conf_get(
      m_filesystem, "object_size", char *buf, size_t len)
    int ceph_conf_set(m_filesystem, const char *option, const char *value);
    int ceph_conf_get(m_filesystem, const char *option, char *buf, size_t len);

    int ceph_statfs(m_filesystem, const char *path, struct statvfs *stbuf);
  */

  SWC_LOGF(LOG_DEBUG, "FS-Ceph, working Dir='%s'", cwd);
}

bool FileSystemCeph::initialize() {
  auto settings = Env::Config::settings();
  
  int err;
  errno = 0;

  /*
  if(settings->has("swc.fs.ceph.user") || 
     settings->has("swc.fs.ceph.group"))
    m_perm = ceph_userperm_new(c uid, gid_t gid, int ngids, gid_t *gidlist);
  */

  std::string client_id;
  if(settings->has("swc.fs.ceph.client.id"))
    client_id = settings->get_str("swc.fs.ceph.client.id");
  
	err = ceph_create(
    &m_filesystem, client_id.empty() ? nullptr : client_id.c_str());
	if(err < 0 || errno) {
    err = err ? -err : errno;
    SWC_LOGF(LOG_ERROR, "ceph_create('%s') failed - %d(%s)", 
                            client_id.c_str(), err, strerror(err));
    return false;
	}

  bool ceph_conf;
  if((ceph_conf = settings->has("swc.fs.ceph.configuration.file"))) {
    const std::string conf_file = settings->get_str(
      "swc.fs.ceph.configuration.file");
    SWC_LOGF(LOG_INFO, "Reading ceph.configuration.file('%s')", 
              conf_file.c_str());
    err = ceph_conf_read_file(m_filesystem, conf_file.c_str());
    if(err < 0 || errno) {
      err = err ? -err : errno;
      SWC_LOGF(LOG_ERROR, "ceph_conf_read_file('%s') failed - %d(%s)", 
                            conf_file.c_str(), err, strerror(err));
      stop();
      m_run.store(true);
      return false;
    }
  }

  if(!ceph_conf && settings->has("swc.fs.ceph.mon.addr")) {
    const std::string mon_host = settings->get_str("swc.fs.ceph.mon.addr");
	  err = ceph_conf_set(m_filesystem, "mon_host", mon_host.c_str());
	  if(err < 0 || errno) {
      err = err ? -err : errno;
      SWC_LOGF(LOG_ERROR, "ceph_create('%s') failed - %d(%s)", 
                              client_id.c_str(), err, strerror(err));
      stop();
      m_run.store(true);
      return false;
	  }
  }

  std::string mon_host;
  mon_host.resize(4096);
  int sz = ceph_conf_get(m_filesystem, "mon_host", mon_host.data(), 4096);
  mon_host = std::string(mon_host.data(), sz);
  SWC_LOGF(LOG_INFO, "Connecting to ceph-mon_host('%s') sz=%d", 
            mon_host.c_str(), sz);

  //int ceph_mount_perms_set(m_filesystem, m_perm);
  //int ceph_select_filesystem(m_filesystem, const char *fs_name);
  
  err = ceph_mount(m_filesystem, path_root.c_str());
  if(err < 0 || errno) {
    err = err ? -err : errno;
    SWC_LOGF(LOG_ERROR, "ceph_mount('%s') failed - %d(%s)", 
                        path_root.c_str(), err, strerror(err));
    stop();
    m_run.store(true);
    return false;
  }

  return m_filesystem != nullptr;
}

FileSystemCeph::~FileSystemCeph() { }

void FileSystemCeph::stop() {
  m_run.store(false);
  
  if(m_perm) {
    ceph_userperm_destroy(m_perm);
  }
  
  if(m_filesystem) {
    if(ceph_is_mounted(m_filesystem)) {
      ceph_sync_fs(m_filesystem);
      ceph_unmount(m_filesystem);
    }
    ceph_release(m_filesystem);
  }

  FileSystem::stop();
}

Types::Fs FileSystemCeph::get_type() {
  return Types::Fs::CEPH;
};

std::string FileSystemCeph::to_string() {
  return format(
    "(type=CEPH path_root=%s path_data=%s)", 
    path_root.c_str(),
    path_data.c_str()
  );
}


bool FileSystemCeph::exists(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);

  errno = 0;
	struct ceph_statx stx;
  err = ceph_statx(m_filesystem, abspath.c_str(), &stx, 
                   CEPH_STATX_CTIME, AT_SYMLINK_NOFOLLOW);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno == ENOENT ? Error::OK : errno;
  SWC_LOGF(LOG_DEBUG, "exists state='%d' err='%d' path='%s'", 
            (int)!err, err, abspath.c_str());
  return !err;
}
  
void FileSystemCeph::remove(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);
  errno = 0;  

  err = ceph_unlink(m_filesystem, abspath.c_str());
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno == ENOENT ? Error::OK : errno;

  if(err) {
    SWC_LOGF(LOG_ERROR, "remove('%s') failed - %s",
              abspath.c_str(), strerror(err));
    return;
  }
  SWC_LOGF(LOG_DEBUG, "remove('%s')", abspath.c_str());
}

size_t FileSystemCeph::length(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);

	struct ceph_statx stx;
  errno = 0;
  err = ceph_statx(m_filesystem, abspath.c_str(), &stx, 
                   CEPH_STATX_SIZE, AT_SYMLINK_NOFOLLOW);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
  if(err) {
    SWC_LOGF(LOG_ERROR, "length('%s') failed - %s", 
              abspath.c_str(), strerror(err));
    return 0;
  }

  SWC_LOGF(LOG_DEBUG, "length len='%lu' path='%s'", stx.stx_size, abspath.c_str());
  return stx.stx_size;
}

void FileSystemCeph::mkdirs(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);
  SWC_LOGF(LOG_DEBUG, "mkdirs path='%s'", abspath.c_str());
  
  errno = 0;
  err = ceph_mkdirs(m_filesystem, abspath.c_str(), 644);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
}

void FileSystemCeph::readdir(int& err, const std::string& name, 
                             DirentList& results) {
  std::string abspath;
  get_abspath(name, abspath);
  SWC_LOGF(LOG_DEBUG, "Readdir dir='%s'", abspath.c_str());

  errno = 0;
	struct ceph_dir_result *dirp;
	err = ceph_opendir(m_filesystem, abspath.c_str(), &dirp);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
  if(err) {
    SWC_LOGF(LOG_ERROR, "readdir('%s') failed - %s", 
              abspath.c_str(), strerror(err)); 
    return;
  }

  struct dirent de;
	struct ceph_statx stx;

  while((err = ceph_readdirplus_r(
              m_filesystem, dirp, &de, 
              &stx, CEPH_STATX_INO, AT_NO_ATTR_SYNC, NULL)) == 1) {
    if(de.d_name[0] == '.' || !de.d_name[0])
      continue;

    auto& entry = results.emplace_back();
    entry.name.append(abspath);
    entry.name.append("/");
    entry.name.append(de.d_name);
    entry.is_dir = S_ISDIR(stx.stx_mode) || de.d_type == DT_DIR;
    
    entry.length = (uint64_t)stx.stx_size;
    entry.last_modification_time = stx.stx_mtime.tv_sec;
  }

  ceph_closedir(m_filesystem, dirp);
  
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
}

void FileSystemCeph::rmdir(int& err, const std::string& name) {
  std::string abspath;
  get_abspath(name, abspath);

  errno = 0;
	struct ceph_dir_result *dirp;
	err = ceph_opendir(m_filesystem, abspath.c_str(), &dirp);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
  if(err) {
    SWC_LOGF(LOG_ERROR, "readdir('%s') failed - %s", 
              abspath.c_str(), strerror(err)); 
    return;
  }

  struct dirent de;
	struct ceph_statx stx;

  while((err = ceph_readdirplus_r(
              m_filesystem, dirp, &de, 
              &stx, CEPH_STATX_INO, AT_NO_ATTR_SYNC, NULL)) == 1) {
    if(de.d_name[0] == '.' || !de.d_name[0])
      continue;

    if(S_ISDIR(stx.stx_mode) || de.d_type == DT_DIR) {
      rmdir(err, name + "/" + de.d_name);
      if(err)
        break;
    }
  }

  errno = 0;
  err = ceph_rmdir(m_filesystem, abspath.c_str()); // recursive
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno == ENOENT ? Error::OK : errno;

  ceph_closedir(m_filesystem, dirp);
  
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;

  SWC_LOGF(LOG_DEBUG, "rmdir('%s')", abspath.c_str());
}

void FileSystemCeph::rename(int& err, const std::string& from, 
                            const std::string& to)  {
  std::string abspath_from;
  get_abspath(from, abspath_from);
  std::string abspath_to;
  get_abspath(to, abspath_to);

  errno = 0;
  err = ceph_rename(m_filesystem, abspath_from.c_str(), abspath_to.c_str());
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
  if(err) {
    SWC_LOGF(LOG_ERROR, "rename('%s' to '%s') failed - %s", 
              abspath_from.c_str(), abspath_to.c_str(), strerror(err));
    return;
  }
  SWC_LOGF(LOG_DEBUG, "rename('%s' to '%s')", 
            abspath_from.c_str(), abspath_to.c_str());
}


void FileSystemCeph::create(int& err, SmartFd::Ptr& smartfd, 
                            int32_t bufsz, uint8_t replication, 
                            int64_t objsz) {
  std::string abspath;
  get_abspath(smartfd->filepath(), abspath);
  SWC_LOGF(LOG_DEBUG, "create %s bufsz=%d replication=%d objsz=%ld",
            smartfd->to_string().c_str(), bufsz, replication, objsz);

  int oflags = O_WRONLY | O_CREAT;
  if((smartfd->flags() & OpenFlags::OPEN_FLAG_OVERWRITE) == 0)
    oflags |= O_APPEND;
  else
    oflags |= O_TRUNC;

  if (bufsz <= -1)
    bufsz = 0;

  if (objsz <= -1 || objsz <= ceph_cfg_min_obj_sz) {
    objsz = 0;
  } else {
    objsz /= 512;
    objsz *= 512;
  }

  errno = 0;
  err = ceph_open_layout(m_filesystem, abspath.c_str(), oflags, 644,
 		                     0, 0, objsz, nullptr);
  if(err < 0) {
    err = -err;
  } else if(err > 0) {
    smartfd->fd(err);
    err = Error::OK;
  } else if(errno) {
    err = errno;
  }

  if (err) {
    smartfd->fd(-1);
    SWC_LOGF(LOG_ERROR, "create failed: %d(%s) objsz=%ld, %s ", 
              err, strerror(err), objsz, smartfd->to_string().c_str());
                
    if(err == EACCES || err == ENOENT)
      err = Error::FS_PATH_NOT_FOUND;
    else if (err == EPERM)
      err = Error::FS_PERMISSION_DENIED;
    return;
  }
  fd_open_incr();
  SWC_LOGF(LOG_DEBUG, "created %s bufsz=%d replication=%d objsz=%ld",
            smartfd->to_string().c_str(), bufsz, replication, objsz);
}

void FileSystemCeph::open(int& err, SmartFd::Ptr& smartfd, int32_t bufsz) {
  std::string abspath;
  get_abspath(smartfd->filepath(), abspath);
  SWC_LOGF(LOG_DEBUG, "open %s bufsz=%d",
            smartfd->to_string().c_str(), bufsz);

  int oflags = O_RDONLY;
  errno = 0;
  err = ceph_open(m_filesystem, abspath.c_str(), oflags, 0);
  if(err < 0) {
    err = -err;
  } else if(err > 0) {
    smartfd->fd(err);
    err = Error::OK;
  } else if(errno) {
    err = errno;
  }

  if (err) {
    smartfd->fd(-1);
    SWC_LOGF(LOG_ERROR, "open failed: %d(%s), %s", 
              err, strerror(err), smartfd->to_string().c_str());
                
    if(err == EACCES || err == ENOENT)
      err = Error::FS_PATH_NOT_FOUND;
    else if (err == EPERM)
      err = Error::FS_PERMISSION_DENIED;
    return;
  }
  fd_open_incr();
  SWC_LOGF(LOG_DEBUG, "opened %s", smartfd->to_string().c_str());
}
  
size_t FileSystemCeph::read(int& err, SmartFd::Ptr& smartfd, 
                            void *dst, size_t amount) {
  SWC_LOGF(LOG_DEBUG, "read %s amount=%lu", 
            smartfd->to_string().c_str(), amount);

  size_t ret;
  errno = 0;
  ssize_t nread = ceph_read(m_filesystem, smartfd->fd(), (char*)dst, amount, 
                            smartfd->pos());
  if(nread < 0)
    err = -nread;
  else if(errno)
    err = errno;

  if(err) {
    ret = 0;
    nread = 0;
    SWC_LOGF(LOG_ERROR, "read failed: %d(%s), %s", 
              err, strerror(err), smartfd->to_string().c_str());
  } else {
    if((ret = nread) != amount)
      err = Error::FS_EOF;
    smartfd->pos(smartfd->pos() + nread);
    SWC_LOGF(LOG_DEBUG, "read(ed) %s amount=%lu eof=%d", 
              smartfd->to_string().c_str(), ret, err == Error::FS_EOF);
  }
  return ret;
}

  
size_t FileSystemCeph::pread(int& err, SmartFd::Ptr& smartfd, 
                             uint64_t offset, void *dst, size_t amount) {
  SWC_LOGF(LOG_DEBUG, "pread %s offset=%lu amount=%lu", 
            smartfd->to_string().c_str(), offset, amount);

  size_t ret;
  errno = 0;
  ssize_t nread = ceph_read(m_filesystem, smartfd->fd(), (char*)dst, amount, 
                            offset);
  if(nread < 0)
    err = -nread;
  else if(errno)
    err = errno;

  if(err) {
    ret = 0;
    nread = 0;
    SWC_LOGF(LOG_ERROR, "pread failed: %d(%s), %s", 
              err, strerror(err), smartfd->to_string().c_str());
  } else {
    if((ret = nread) != amount)
      err = Error::FS_EOF;
    smartfd->pos(offset + nread);
    SWC_LOGF(LOG_DEBUG, "pread(ed) %s amount=%lu eof=%d", 
              smartfd->to_string().c_str(), ret, err == Error::FS_EOF);
  }
  return ret;
}

size_t FileSystemCeph::append(int& err, SmartFd::Ptr& smartfd, 
                                StaticBuffer& buffer, Flags flags) {

  SWC_LOGF(LOG_DEBUG, "append %s amount=%u flags=%d", 
            smartfd->to_string().c_str(), buffer.size, flags);
    
  ssize_t nwritten = 0;
  errno = 0;

  nwritten = ceph_write(m_filesystem, smartfd->fd(), 
                        (const char*)buffer.base, buffer.size,
	                      smartfd->pos());
  if(nwritten < 0)
    err = -nwritten;
  else if(errno)
    err = errno;   

  if (err) {
    SWC_LOGF(LOG_ERROR, "write failed: %d(%s), %s", 
              err, strerror(err), smartfd->to_string().c_str());
    return 0;
  }
  smartfd->pos(smartfd->pos() + nwritten);

  if (flags == Flags::FLUSH)
    flush(err, smartfd);
  else if (flags == Flags::SYNC) 
    sync(err, smartfd);

  SWC_LOGF(LOG_DEBUG, "appended %s amount=%ld", 
            smartfd->to_string().c_str(), nwritten);
  return nwritten;
}

void FileSystemCeph::seek(int& err, SmartFd::Ptr& smartfd, size_t offset) {
  SWC_LOGF(LOG_DEBUG, "seek %s offset=%lu", 
            smartfd->to_string().c_str(), offset);
    
  errno = 0;
  err = ceph_lseek(m_filesystem, smartfd->fd(), offset, SEEK_SET);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;

  if(err) {
    SWC_LOGF(LOG_ERROR, "seek failed - %d(%s) %s", 
               err, strerror(err), smartfd->to_string().c_str());
    return;
  }
  smartfd->pos(offset);
}

void FileSystemCeph::flush(int& err, SmartFd::Ptr& smartfd) {
  SWC_LOGF(LOG_DEBUG, "flush %s", smartfd->to_string().c_str());

  errno = 0;
  err = ceph_fsync(m_filesystem, smartfd->fd(), true);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
  
  if (err)
    SWC_LOGF(LOG_ERROR, "flush failed: %d(%s), %s", 
              err, strerror(err), smartfd->to_string().c_str());
}

void FileSystemCeph::sync(int& err, SmartFd::Ptr& smartfd) {
  SWC_LOGF(LOG_DEBUG, "sync %s", smartfd->to_string().c_str());
  
  errno = 0;
  err = ceph_fsync(m_filesystem, smartfd->fd(), false);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
  
  if (err)
    SWC_LOGF(LOG_ERROR, "sync failed: %d(%s), %s", 
              err, strerror(err), smartfd->to_string().c_str());
}

void FileSystemCeph::close(int& err, SmartFd::Ptr& smartfd) {
    
  SWC_LOGF(LOG_DEBUG, "close %s", smartfd->to_string().c_str());

  if(smartfd->valid()) {
    errno = 0;
    err = ceph_close(m_filesystem, smartfd->fd());
    if(err < 0)
      err = -err;
    else if(errno)
      err = errno;

    if(err)
      SWC_LOGF(LOG_ERROR, "close, failed: %d(%s), %s", 
                 err, strerror(err), smartfd->to_string().c_str());
    fd_open_decr();
  } else {
    err = EBADR;
  }
  smartfd->fd(-1);
  smartfd->pos(0);
}






}} // namespace SWC



extern "C" { 
SWC::FS::FileSystem* fs_make_new_ceph() {
  return (SWC::FS::FileSystem*)(new SWC::FS::FileSystemCeph());
};
void fs_apply_cfg_ceph(SWC::Env::Config::Ptr env) {
  SWC::Env::Config::set(env);
};
}
