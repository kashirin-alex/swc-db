/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Ceph/FileSystem.h"

extern "C" {
#include <dirent.h>
}


namespace SWC { namespace FS {


Configurables* apply_ceph(Configurables* config) {
  config->settings->file_desc.add_options()
    ("swc.fs.ceph.path.root", Config::str(""),
      "Ceph FileSystem's base root path")
    ("swc.fs.ceph.cfg.dyn", Config::strs(),
      "Dyn-config file")

    ("swc.fs.ceph.client.id", Config::str(),
      "This ceph Client Id(name)")

    ("swc.fs.ceph.perm.group", Config::i32(),
      "CephFs Permission Group")
    ("swc.fs.ceph.perm.user", Config::i32(),
      "CephFs Permission User")

    ("swc.fs.ceph.stripe.unit", Config::i32(),
      "CephFs default stripe_unit")
    ("swc.fs.ceph.stripe.count", Config::i32(),
      "CephFs default stripe_count")
    ("swc.fs.ceph.object.size", Config::i32(),
      "CephFs default object_size")
    ("swc.fs.ceph.replication", Config::i32(),
      "CephFs default replication")

    ("swc.fs.ceph.configuration.file", Config::str(),
      "The ceph configuration file 'ceph.conf'")
    ("swc.fs.ceph.mon.addr", Config::str(),
      "The ceph config value for 'mon_addr'")

    ("swc.fs.ceph.metrics.enabled", Config::boo(true),
     "Enable or Disable Metrics Tracking")

    ("swc.fs.ceph.fds.max", Config::g_i32(256),
      "Max Open Fds for opt. without closing")
  ;

  config->settings->parse_file(
    config->settings->get_str("swc.fs.ceph.cfg", ""),
    "swc.fs.ceph.cfg.dyn"
  );

  config->path_root = config->settings->get_str(
    "swc.fs.ceph.path.root");
  config->cfg_fds_max = config->settings
    ->get<Config::Property::V_GINT32>("swc.fs.ceph.fds.max");
  config->stats_enabled = config->settings->get_bool(
    "swc.fs.ceph.metrics.enabled");
  return config;
}




FileSystemCeph::FileSystemCeph(Configurables* config)
        : FileSystem(apply_ceph(config)),
          m_filesystem(nullptr), m_perm(nullptr) {
  setup_connection();
}

void FileSystemCeph::setup_connection() {

  uint32_t tries=0;
  while(m_run && !initialize()) {
    SWC_LOGF(LOG_ERROR,
      "FS-Ceph, unable to initialize connection to Ceph, try=%u",
      ++tries);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  // status check
  int err;
  const char* cwd = ceph_getcwd(m_filesystem);
  if(!Condition::str_eq(path_root, cwd)) {
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
                            client_id.c_str(), err, Error::get_text(err));
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
                            conf_file.c_str(), err, Error::get_text(err));
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
                              client_id.c_str(), err, Error::get_text(err));
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
                        path_root.c_str(), err, Error::get_text(err));
    stop();
    m_run.store(true);
    return false;
  }

  return bool(m_filesystem);
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

Type FileSystemCeph::get_type() const noexcept {
  return Type::CEPH;
}

std::string FileSystemCeph::to_string() const {
  return format(
    "(type=CEPH path_root=%s path_data=%s)",
    path_root.c_str(),
    path_data.c_str()
  );
}


bool FileSystemCeph::exists(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::EXISTS_SYNC);
  SWC_FS_EXISTS_START(name);
  std::string abspath;
  get_abspath(name, abspath);

  errno = 0;
	struct ceph_statx stx;
  err = ceph_statx(m_filesystem, abspath.c_str(), &stx,
                   CEPH_STATX_CTIME, AT_SYMLINK_NOFOLLOW);
  if(err)
    err = -err;
  else if(errno)
    err = errno;
  bool state = !err;
  if(err == ENOENT)
    err = Error::OK;
  SWC_FS_EXISTS_FINISH(err, abspath, state, tracker);
  return state;
}

void FileSystemCeph::remove(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::REMOVE_SYNC);
  SWC_FS_REMOVE_START(name);
  std::string abspath;
  get_abspath(name, abspath);
  errno = 0;

  err = ceph_unlink(m_filesystem, abspath.c_str());
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
  if(err == ENOENT)
    err = Error::OK;
  SWC_FS_REMOVE_FINISH(err, abspath, tracker);
}

size_t FileSystemCeph::length(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::LENGTH_SYNC);
  SWC_FS_LENGTH_START(name);
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
  size_t len = err ? 0 : stx.stx_size;
  SWC_FS_LENGTH_FINISH(err, abspath, len, tracker);
  return len;
}

void FileSystemCeph::mkdirs(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::MKDIRS_SYNC);
  SWC_FS_MKDIRS_START(name);
  std::string abspath;
  get_abspath(name, abspath);

  errno = 0;
  err = ceph_mkdirs(m_filesystem, abspath.c_str(), 644);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
  SWC_FS_MKDIRS_FINISH(err, abspath, tracker);
}

void FileSystemCeph::readdir(int& err, const std::string& name,
                             DirentList& results) {
  auto tracker = statistics.tracker(Statistics::READDIR_SYNC);
  SWC_FS_READDIR_START(name);
  std::string abspath;
  get_abspath(name, abspath);

  errno = 0;
	struct ceph_dir_result* dirp = nullptr;
	err = ceph_opendir(m_filesystem, abspath.c_str(), &dirp);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
  if(err)
    goto _finish;

  struct dirent de;
	struct ceph_statx stx;

  while((err = ceph_readdirplus_r(
              m_filesystem, dirp, &de,
              &stx, CEPH_STATX_INO, AT_NO_ATTR_SYNC, nullptr)) == 1) {
    if(de.d_name[0] == '.' || !de.d_name[0])
      continue;

    auto& entry = results.emplace_back();
    entry.name.reserve(abspath.length() + 1 + strlen(de.d_name));
    entry.name.append(abspath);
    entry.name.append("/");
    entry.name.append(de.d_name);
    entry.is_dir = S_ISDIR(stx.stx_mode) || de.d_type == DT_DIR;

    entry.length = stx.stx_size;
    entry.last_modification_time = stx.stx_mtime.tv_sec;
  }
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;

  _finish:
    if(dirp)
      ceph_closedir(m_filesystem, dirp);
    SWC_FS_READDIR_FINISH(err, abspath, results.size(), tracker);
}

void FileSystemCeph::rmdir(int& err, const std::string& name) {
  auto tracker = statistics.tracker(Statistics::RMDIR_SYNC);
  SWC_FS_RMDIR_START(name);
  std::string abspath;
  get_abspath(name, abspath);

  errno = 0;
	struct ceph_dir_result* dirp = nullptr;
	err = ceph_opendir(m_filesystem, abspath.c_str(), &dirp);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
  if(err)
    goto _finish;

  struct dirent de;
	struct ceph_statx stx;

  while((err = ceph_readdirplus_r(
              m_filesystem, dirp, &de,
              &stx, CEPH_STATX_INO, AT_NO_ATTR_SYNC, nullptr)) == 1) {
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
    err = errno;
  if(err == ENOENT)
    err = Error::OK;

  _finish:
    if(dirp)
      ceph_closedir(m_filesystem, dirp);
    SWC_FS_RMDIR_FINISH(err, abspath, tracker);
}

void FileSystemCeph::rename(int& err, const std::string& from,
                            const std::string& to) {
  auto tracker = statistics.tracker(Statistics::RENAME_SYNC);
  SWC_FS_RENAME_START(from, to);
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
  SWC_FS_RENAME_FINISH(err, abspath_from, abspath_to, tracker);
}


void FileSystemCeph::create(int& err, SmartFd::Ptr& smartfd,
                            int32_t bufsz, uint8_t replication,
                            int64_t objsz) {
  auto tracker = statistics.tracker(Statistics::CREATE_SYNC);

  int oflags = O_WRONLY | O_CREAT;
  if(!(smartfd->flags() & OpenFlags::OPEN_FLAG_OVERWRITE))
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
  SWC_FS_CREATE_START(smartfd, bufsz, replication, objsz);
  std::string abspath;
  get_abspath(smartfd->filepath(), abspath);

  errno = 0;
  int tmperr = ceph_open_layout(m_filesystem, abspath.c_str(), oflags, 644,
 		                     0, 0, objsz, nullptr);
  if(tmperr < 0) {
    tmperr = -tmperr;
  } else if(tmperr > 0) {
    smartfd->fd(tmperr);
    fd_open_incr();
    err = tmperr = Error::OK;
  } else if(errno) {
    tmperr = errno;
  }
  if(tmperr) {
    smartfd->fd(-1);
    if(tmperr == EACCES || tmperr == ENOENT)
      err = Error::FS_PATH_NOT_FOUND;
    else if (tmperr == EPERM)
      err = Error::FS_PERMISSION_DENIED;
    else
      err = tmperr;
  }
  SWC_FS_CREATE_FINISH(tmperr, smartfd, fds_open(), tracker);
}

void FileSystemCeph::open(int& err, SmartFd::Ptr& smartfd, int32_t bufsz) {
  auto tracker = statistics.tracker(Statistics::OPEN_SYNC);
  SWC_FS_OPEN_START(smartfd, bufsz);
  std::string abspath;
  get_abspath(smartfd->filepath(), abspath);

  int oflags = O_RDONLY;
  errno = 0;
  int tmperr = ceph_open(m_filesystem, abspath.c_str(), oflags, 0);
  if(tmperr < 0) {
    tmperr = -tmperr;
  } else if(tmperr > 0) {
    smartfd->fd(tmperr);
    fd_open_incr();
    err = tmperr = Error::OK;
  } else if(errno) {
    tmperr = errno;
  }

  if(tmperr) {
    smartfd->fd(-1);
    if(tmperr == EACCES || tmperr == ENOENT)
      err = Error::FS_PATH_NOT_FOUND;
    else if (tmperr == EPERM)
      err = Error::FS_PERMISSION_DENIED;
    else
      err = tmperr;
  }
  SWC_FS_OPEN_FINISH(tmperr, smartfd, fds_open(), tracker);
}

size_t FileSystemCeph::read(int& err, SmartFd::Ptr& smartfd,
                            void *dst, size_t amount) {
  auto tracker = statistics.tracker(Statistics::READ_SYNC);
  SWC_FS_READ_START(smartfd, amount);

  size_t ret;
  errno = 0;
  ssize_t nread = ceph_read(m_filesystem, smartfd->fd(),
                            static_cast<char*>(dst), amount, smartfd->pos());
  if(nread < 0)
    err = -nread;
  else if(errno)
    err = errno;
  else
    err = Error::OK;

  if(err) {
    ret = 0;
    nread = 0;
  } else {
    if((ret = nread) != amount)
      err = Error::FS_EOF;
    smartfd->forward(nread);
  }
  SWC_FS_READ_FINISH(err, smartfd, ret, tracker);
  return ret;
}


size_t FileSystemCeph::pread(int& err, SmartFd::Ptr& smartfd,
                             uint64_t offset, void *dst, size_t amount) {
  auto tracker = statistics.tracker(Statistics::PREAD_SYNC);
  SWC_FS_PREAD_START(smartfd, offset, amount);

  size_t ret;
  errno = 0;
  ssize_t nread = ceph_read(m_filesystem, smartfd->fd(),
                            static_cast<char*>(dst), amount, offset);
  if(nread < 0)
    err = -nread;
  else if(errno)
    err = errno;
  else
    err = Error::OK;

  if(err) {
    ret = 0;
    nread = 0;
  } else if((ret = nread) != amount) {
    err = Error::FS_EOF;
  }
  SWC_FS_PREAD_FINISH(err, smartfd, ret, tracker);
  return ret;
}

size_t FileSystemCeph::append(int& err, SmartFd::Ptr& smartfd,
                                StaticBuffer& buffer, Flags flags) {
  auto tracker = statistics.tracker(Statistics::APPEND_SYNC);
  SWC_FS_APPEND_START(smartfd, buffer.size, flags);

  ssize_t nwritten = 0;
  errno = 0;

  nwritten = ceph_write(m_filesystem, smartfd->fd(),
                        reinterpret_cast<const char*>(buffer.base),
                        buffer.size, smartfd->pos());
  if(nwritten < 0)
    err = -nwritten;
  else if(errno)
    err = errno;
  else if(nwritten != ssize_t(buffer.size))
    err = ECANCELED;
  else
    err = Error::OK;

  if(!err) {
    if (flags == Flags::FLUSH)
      flush(err, smartfd);
    else if (flags == Flags::SYNC)
      sync(err, smartfd);
  }
  if(err) {
    nwritten = 0;
  } else {
    smartfd->forward(nwritten);
  }
  SWC_FS_APPEND_FINISH(err, smartfd, nwritten, tracker);
  return nwritten;
}

void FileSystemCeph::seek(int& err, SmartFd::Ptr& smartfd, size_t offset) {
  auto tracker = statistics.tracker(Statistics::SEEK_SYNC);
  SWC_FS_SEEK_START(smartfd, offset);

  errno = 0;
  err = ceph_lseek(m_filesystem, smartfd->fd(), offset, SEEK_SET);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
  if(!err) {
    smartfd->pos(offset);
  }
  SWC_FS_SEEK_FINISH(err, smartfd, tracker);
}

void FileSystemCeph::flush(int& err, SmartFd::Ptr& smartfd) {
  auto tracker = statistics.tracker(Statistics::FLUSH_SYNC);
  SWC_FS_FLUSH_START(smartfd);

  errno = 0;
  err = ceph_fsync(m_filesystem, smartfd->fd(), true);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
  SWC_FS_FLUSH_FINISH(err, smartfd, tracker);
}

void FileSystemCeph::sync(int& err, SmartFd::Ptr& smartfd) {
  auto tracker = statistics.tracker(Statistics::SYNC_SYNC);
  SWC_FS_SYNC_START(smartfd);

  errno = 0;
  err = ceph_fsync(m_filesystem, smartfd->fd(), false);
  if(err < 0)
    err = -err;
  else if(errno)
    err = errno;
  SWC_FS_SYNC_FINISH(err, smartfd, tracker);
}

void FileSystemCeph::close(int& err, SmartFd::Ptr& smartfd) {
  auto tracker = statistics.tracker(Statistics::CLOSE_SYNC);
  SWC_FS_CLOSE_START(smartfd);

  int32_t fd = smartfd->invalidate();
  if(fd != -1) {
    errno = 0;
    err = ceph_close(m_filesystem, fd);
    if(err < 0)
      err = -err;
    else if(errno)
      err = errno;
    fd_open_decr();
  } else {
    err = EBADR;
  }
  SWC_FS_CLOSE_FINISH(err, smartfd, tracker);
}






}} // namespace SWC



extern "C" {
SWC::FS::FileSystem* fs_make_new_ceph(SWC::FS::Configurables* config) {
  return static_cast<SWC::FS::FileSystem*>(
    new SWC::FS::FileSystemCeph(config));
}
}
