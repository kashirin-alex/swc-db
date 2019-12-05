/*
 * Copyright (C) 2019 SWC-DB (author: Kashirin Alex (kashirin.alex@gmail.com))
 */


#include "swcdb/fs/FileSystem.h"

#include <algorithm>


namespace SWC { namespace FS {


std::string normalize_pathname(std::string s) {
  if(*(--s.end()) != '/') 
    s.append("/"); 
  return s;
}

Types::Fs fs_type(std::string fs_name) {
  std::transform(fs_name.begin(), fs_name.end(), fs_name.begin(),
                 [](unsigned char c){ return std::tolower(c); });
    
#if !defined (FS_BROKER_APP)
  if(fs_name.compare("broker") == 0)
    return Types::Fs::BROKER;
#endif

  if(fs_name.compare("local") == 0)
    return Types::Fs::LOCAL;
  if(fs_name.compare("hadoop") == 0)
    return Types::Fs::HADOOP;
  if(fs_name.compare("hadoop_jvm") == 0)
    return Types::Fs::HADOOP_JVM;
  if(fs_name.compare("ceph") == 0)
    return Types::Fs::CEPH;
  if(fs_name.compare("custom") == 0)
    return Types::Fs::CUSTOM;
  else
    HT_THROWF(Error::CONFIG_BAD_VALUE, 
              "Unknown FileSystem name=%s", fs_name.c_str());
  return Types::Fs::NONE;
}

std::string type_to_string(Types::Fs typ) {
  if(typ == Types::Fs::BROKER)
    return "Broker";
  if(typ == Types::Fs::LOCAL)
    return "Local";
  if(typ == Types::Fs::HADOOP)
    return "Hadoop";
  if(typ == Types::Fs::HADOOP_JVM)
    return "HadoopJVM";
  if(typ == Types::Fs::CEPH)
    return "Ceph";
  if(typ == Types::Fs::CUSTOM)
    return "Custom";
  HT_THROWF(Error::CONFIG_BAD_VALUE, 
            "Unknown FileSystem type=%d", (int)typ);
}


FileSystem::FileSystem() { }
  
FileSystem::FileSystem(bool setting_applied) { }
  
FileSystem::FileSystem(std::string root, bool setting_applied)
    : path_root(normalize_pathname(root)),
      path_data(
        normalize_pathname(
          Env::Config::settings()->get<std::string>("swc.fs.path.data"))) { 
}

FileSystem::~FileSystem() { 
  std::cout << " ~FileSystem() \n"; 
}

void FileSystem::stop() {}

Types::Fs FileSystem::get_type() {
  return Types::Fs::NONE;
}
  
const std::string FileSystem::to_string() {
  return format(
    "(type=NONE path_root=%s path_data=%s)", 
    path_root.c_str(),
    path_data.c_str()
  );
}

const std::string FileSystem::get_abspath(const std::string &name) {
  std::string abspath;
  abspath.append(path_root);
  if(!name.empty()){
    abspath.append(path_data);
    abspath.append(name);
  } else {
    abspath.append(path_data.substr(0, path_data.length()-1));
  }
  return abspath;
}

/* Default functions of an async call, 
 * used if a FileSystem(typ) does not implement 
*/

void FileSystem::exists(Callback::ExistsCb_t cb, const std::string &name) {
  int err = Error::OK;
  bool state = exists(err, name);
  cb(err, state);
}

void FileSystem::remove(Callback::RemoveCb_t cb, const std::string &name) {
  int err = Error::OK;
  remove(err, name);
  cb(err);
}
 
void FileSystem::length(Callback::LengthCb_t cb, const std::string &name) {
  int err = Error::OK;
  size_t len = length(err, name);
  cb(err, len);
}
 
void FileSystem::mkdirs(Callback::MkdirsCb_t cb, const std::string &name) {
  int err = Error::OK;
  mkdirs(err, name);
  cb(err);
}

void FileSystem::readdir(Callback::ReaddirCb_t cb, const std::string &name) {
  int err = Error::OK;
  DirentList listing;
  readdir(err, name, listing);
  cb(err, listing);
}

void FileSystem::rmdir(Callback::RmdirCb_t cb, const std::string &name) {
  int err = Error::OK;
  rmdir(err, name);
  cb(err);
}
  
void FileSystem::rename(Callback::RmdirCb_t cb, const std::string &from, 
                                      const std::string &to) {
  int err = Error::OK;
  rename(err, from, to);
  cb(err);
}

void FileSystem::write(Callback::WriteCb_t cb, SmartFd::Ptr &smartfd,
                       int32_t replication, int64_t blksz, 
                       StaticBuffer &buffer) {
  int err = Error::OK;
  write(err, smartfd, replication, blksz, buffer);
  cb(err, smartfd);
}

void FileSystem::create(Callback::CreateCb_t cb, SmartFd::Ptr &smartfd,
                        int32_t bufsz, int32_t replication, int64_t blksz) {
  int err = Error::OK;
  create(err, smartfd, bufsz, replication, blksz);
  cb(err, smartfd);
}

void FileSystem::open(Callback::OpenCb_t cb, SmartFd::Ptr &smartfd, 
                      int32_t bufsz) {
  int err = Error::OK;
  open(err, smartfd, bufsz);
  cb(err, smartfd);
}

size_t FileSystem::read(int &err, SmartFd::Ptr &smartfd, 
                        StaticBuffer* dst, size_t amount) {
  if(!dst->size)
    dst->reallocate(amount);
  return read(err, smartfd, dst->base, amount);
}
   
void FileSystem::read(Callback::ReadCb_t cb, SmartFd::Ptr &smartfd, 
                      size_t amount) {
  int err = Error::OK;
  auto dst = std::make_shared<StaticBuffer>();
  read(err, smartfd, dst.get(), amount);
  cb(err, smartfd, dst);
}
  
size_t FileSystem::pread(int &err, SmartFd::Ptr &smartfd, 
                         uint64_t offset, StaticBuffer* dst, size_t amount) {
  if(!dst->size)
    dst->reallocate(amount);
  return pread(err, smartfd, offset, dst->base, amount);
}

void FileSystem::pread(Callback::PreadCb_t cb, SmartFd::Ptr &smartfd, 
                       uint64_t offset, size_t amount) {
  int err = Error::OK;
  auto dst = std::make_shared<StaticBuffer>();
  pread(err, smartfd, offset, dst.get(), amount);
  cb(err, smartfd, dst);
}
 
void FileSystem::append(Callback::AppendCb_t cb, SmartFd::Ptr &smartfd, 
                        StaticBuffer &buffer, Flags flags) {
  int err = Error::OK;
  size_t len = append(err, smartfd, buffer, flags);
  cb(err, smartfd, len);
}

void FileSystem::seek(Callback::CloseCb_t cb, SmartFd::Ptr &smartfd, 
                      size_t offset) {
  int err = Error::OK;
  seek(err, smartfd, offset);
  cb(err, smartfd);
}

void FileSystem::flush(Callback::FlushCb_t cb, SmartFd::Ptr &smartfd) {
  int err = Error::OK;
  flush(err, smartfd);
  cb(err, smartfd);
}

void FileSystem::sync(Callback::SyncCb_t cb, SmartFd::Ptr &smartfd) {
  int err = Error::OK;
  sync(err, smartfd);
  cb(err, smartfd);
}

void FileSystem::close(Callback::CloseCb_t cb, SmartFd::Ptr &smartfd) {
  int err = Error::OK;
  close(err, smartfd);
  cb(err, smartfd);
}


}}