/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/core/comm/IoContext.h"
#include "swcdb/core/comm/Settings.h"
#include "swcdb/fs/Settings.h"
#include "swcdb/fs/Interface.h"


using namespace SWC;

void init_app_options(Config::Settings* settings) {
  Config::init_comm_options(settings);
  Config::init_fs_options(settings);
}



void run(size_t thread_id){

    int err = Error::OK;
    bool exists = Env::FsInterface::fs()->exists(err, "nonexisting/"+std::to_string(thread_id));
    if(exists || err != Error::OK){
     std::cerr << "ERROR(nonexisting file) exists=" << exists << " err=" << err << "\n";
     exit(1);
    }
    err = Error::OK;
    Env::FsInterface::fs()->mkdirs(err, std::to_string(thread_id));
    if(err != Error::OK){
     std::cerr << "ERROR mkdirs err=" << err << "\n";
     exit(1);
    }
    err = Error::OK;
    Env::FsInterface::fs()->mkdirs(err, std::to_string(thread_id));
    if(err != Error::OK){
     std::cerr << "ERROR mkdirs err=" << err << "\n";
     exit(1);
    }

    err = Error::OK;
    Env::FsInterface::fs()->rename(err, std::to_string(thread_id), std::to_string(thread_id)+"new");
    if(err != Error::OK){
     std::cerr << "ERROR(rename-dir) err=" << err << "\n";
     exit(1);
    }
    if(!Env::FsInterface::fs()->exists(err, std::to_string(thread_id)+"new") || err != Error::OK){
     std::cerr << "ERROR(rename-dir) exists err=" << err << "\n";
     exit(1);
    }
    err = Error::OK;
    Env::FsInterface::fs()->rename(err, std::to_string(thread_id)+"new", std::to_string(thread_id));
    if(err != Error::OK){
     std::cerr << "ERROR(rename-dir) err=" << err << "\n";
     exit(1);
    }
    if(!Env::FsInterface::fs()->exists(err, std::to_string(thread_id)) || err != Error::OK){
     std::cerr << "ERROR(rename-dir) exists err=" << err << "\n";
     exit(1);
    }

    err = Error::OK;
    FS::DirentList listing;
    Env::FsInterface::fs()->readdir(err, "", listing);
    if(err != Error::OK){
     std::cerr << "ERROR readir err=(" << err << ")\n";
     exit(1);
    }
    bool found = false;
    std::cout << "Dir List, sz=" << listing.size() <<  ":\n";
    for(auto& dirent : listing){
      std::cout << " " << dirent.to_string();
      if(Condition::str_eq(dirent.name, std::to_string(thread_id))) {
        found = true;
        break;
      }
    }
    if(!found){
     std::cerr << "ERROR readir missing expected dir=" << std::to_string(thread_id) << "\n";
     exit(1);
    }


    err = Error::OK;
    exists = Env::FsInterface::fs()->exists(err, std::to_string(thread_id));
    if(!exists || err != Error::OK){
     std::cerr << "ERROR(existing file) exists=" << exists << " err=" << err << "\n";
     exit(1);
    }

    err = Error::OK;
    Env::FsInterface::fs()->rmdir(err, std::to_string(thread_id));
    if(err != Error::OK){
     std::cerr << "ERROR(rmdir) err=" << err << "\n";
     exit(1);
    }
    err = Error::OK;
    exists = Env::FsInterface::fs()->exists(err, std::to_string(thread_id));
    if(exists || err != Error::OK){
     std::cerr << "ERROR(rmdir failed) exists=" << exists << " err=" << err << "\n";
     exit(1);
    }
    err = Error::OK;
    Env::FsInterface::fs()->remove(err, std::to_string(thread_id));
    if(err != Error::OK && err != 2){
     std::cerr << "ERROR(remove) non-existing err=" << err << "\n";
     exit(1);
    }

    err = Error::OK;
    FS::SmartFd::Ptr smartfd = FS::SmartFd::make_ptr(
      "testfile_"+std::to_string(thread_id),
      FS::OpenFlags::OPEN_FLAG_OVERWRITE);
    Env::FsInterface::fs()->create(err, smartfd, -1, 0, -1);
    if(err != Error::OK || !smartfd->valid()) {
     std::cerr << "ERROR(create) err=" << err << " " << smartfd->to_string() <<"\n";
     exit(1);
    }
    err = Error::OK;
    Env::FsInterface::fs()->close(err, smartfd);
    if(err != Error::OK){
     std::cerr << "ERROR(close) err=" << err << "\n";
     exit(1);
    }
    err = Error::OK;
    exists = Env::FsInterface::fs()->exists(err, smartfd->filepath());
    if(!exists || err != Error::OK){
     std::cerr << "ERROR(create failed) exists=" << exists << " err=" << err << "\n";
     exit(1);
    }

    err = Error::OK;
    Env::FsInterface::fs()->remove(err, smartfd->filepath());
    if(err != Error::OK){
     std::cerr << "ERROR(remove) created-file err=" << err << "\n";
     exit(1);
    }
    err = Error::OK;
    exists = Env::FsInterface::fs()->exists(err, smartfd->filepath());
    if(exists || err != Error::OK){
     std::cerr << "ERROR(remove failed) created-file  exists=" << exists << " err=" << err << "\n";
     exit(1);
    }


    // create >> append >> flush >> sync >> close >> exists >> length
    // >> open >> read >> seek >> read(suff) >> seek(for EOF) >> read(EOF) >> pread >> pread(EOF) >> close >> remove

    // create >>
    Env::FsInterface::fs()->create(err, smartfd, -1, 0, -1);
    if(err != Error::OK || !smartfd->valid()) {
     std::cerr << "ERROR(create) err=" << err << " " << smartfd->to_string() <<"\n";
     exit(1);
    }

    // append >>
    int r=1;
    size_t num_blocks = 16/r;
    size_t block_sz = 67108864*r;
    size_t file_sz = 0;
    std::string data_start("Abc1234");
    std::string data_end("567890");
    size_t file_blk = block_sz-data_start.length()-data_end.length();
    size_t written = 0;
    for(size_t i=0;i<num_blocks;++i){
      file_sz += block_sz;
      std::string data = data_start;
      data.reserve(file_blk + data_end.size());
      for(size_t _n=0; _n < file_blk; ++_n)
        data.append("+");
      data.append(data_end);
      StaticBuffer buffer(
        reinterpret_cast<uint8_t*>(data.data()), data.length(), false);
      size_t amount = Env::FsInterface::fs()->append(err, smartfd, buffer, FS::Flags::FLUSH);
      written += amount;
      if(err != Error::OK || amount!=data.length() || smartfd->pos() != written) {
        std::cerr << "ERROR(append) err=" << err << " amount=" << amount << " " << smartfd->to_string() <<"\n";
        exit(1);
      }
      if(i == 7) {
        err = Error::OK;
        Env::FsInterface::fs()->sync(err, smartfd);
        if(err != Error::OK){
          std::cerr << "ERROR(sync,append) err=" << err << "\n";
          exit(1);
        }
      }
      std::cout << " progress=" << thread_id << " written=" << written << "\n";
    }


    // flush >>
    err = Error::OK;
    Env::FsInterface::fs()->flush(err, smartfd);
    if(err != Error::OK){
     std::cerr << "ERROR(flush,create) err=" << err << "\n";
     exit(1);
    }
    // sync >>
    err = Error::OK;
    Env::FsInterface::fs()->sync(err, smartfd);
    if(err != Error::OK){
     std::cerr << "ERROR(sync,create) err=" << err << "\n";
     exit(1);
    }

    // close >>
    err = Error::OK;
    Env::FsInterface::fs()->close(err, smartfd);
    if(err != Error::OK){
     std::cerr << "ERROR(close,create) err=" << err << "\n";
     exit(1);
    }

    // exists >>
    err = Error::OK;
    exists = Env::FsInterface::fs()->exists(err, smartfd->filepath());
    if(!exists || err != Error::OK){
     std::cerr << "ERROR(create failed) exists=" << exists << " err=" << err << "\n";
     exit(1);
    }

    // length >>
    err = Error::OK;
    size_t len = Env::FsInterface::fs()->length(err, smartfd->filepath());
    if(err != Error::OK || len != file_sz) {
     std::cerr << "ERROR(length) len=" << len << " expected-len=" << file_sz
               << " err=" << err << " " << smartfd->to_string() <<"\n";
     exit(1);
    }

    // open >>
    err = Error::OK;
    smartfd->flags(0);
    Env::FsInterface::fs()->open(err, smartfd, -1);
    if(err != Error::OK || !smartfd->valid()) {
     std::cerr << "ERROR(open) err=" << err << " " << smartfd->to_string() <<"\n";
     exit(1);
    }

    // read >>
    err = Error::OK;
    uint8_t buf[7];
    if (Env::FsInterface::fs()->read(err, smartfd, buf,  data_start.length()) != data_start.length()
        || err != Error::OK
        || !Condition::str_eq(
              reinterpret_cast<char*>(buf), data_start.c_str(), data_start.length())) {
     std::cerr << "ERROR(read) err=" << err
               << " buf=" << std::string(reinterpret_cast<char*> (buf), data_start.length())
                << " " << smartfd->to_string() <<"\n";
     exit(1);
    }
    std::cout << "read-data='"
              << std::string(reinterpret_cast<char*>(buf), data_start.length()) << "'\n";

    // seek >>
    err = Error::OK;
    size_t seek_offset = len-data_end.length();
    Env::FsInterface::fs()->seek(err, smartfd, seek_offset);
    if (err != Error::OK || smartfd->pos() != seek_offset) {
     std::cerr << "ERROR(seek) err=" << err << " to=" << seek_offset << " " << smartfd->to_string() <<"\n";
     exit(1);
    }

    // read(suff) >>
    err = Error::OK;
    uint8_t bufsuf[6];
    if (Env::FsInterface::fs()->read(err, smartfd, bufsuf,  data_end.length()) != data_end.length()
        || err != Error::OK
        || !Condition::str_eq(reinterpret_cast<char*>(bufsuf), data_end.c_str(), data_end.length())) {
     std::cerr << "ERROR(read(suff)) err=" << err << " buf=" << bufsuf << " " << smartfd->to_string() <<"\n";
     exit(1);
    }
    std::cout << "read(suff)-data='"
              << std::string(reinterpret_cast<char*> (bufsuf), data_end.length()) << "'\n";

    // seek(for EOF) >>
    err = Error::OK;
    seek_offset = len-data_end.length()+1;
    Env::FsInterface::fs()->seek(err, smartfd, seek_offset);
    if (err != Error::OK || smartfd->pos() != seek_offset) {
     std::cerr << "ERROR(seek) err=" << err << " to=" << seek_offset << " " << smartfd->to_string() <<"\n";
     exit(1);
    }

    // read(with EOF) >>
    err = Error::OK;
    uint8_t bufeof[6];
    if (Env::FsInterface::fs()->read(err, smartfd, bufeof, data_end.length()) != data_end.length()-1
        || err != Error::FS_EOF
        || !Condition::str_eq(reinterpret_cast<char*>(bufeof), data_end.data()+1, data_end.length()-1 )) {
     std::cerr << "ERROR(read(with EOF)) err=" << err << " buf=" << bufeof << " " << smartfd->to_string() <<"\n";
     exit(1);
    }
    std::cout << "read(with EOF)-data='"
              << std::string(reinterpret_cast<char*> (bufeof), data_end.length()-1) << "'\n";


    // pread >>
    size_t pread_offset = 0;
    for(size_t i=0;i<num_blocks;++i){
      err = Error::OK;
      uint8_t buf_start[7];
      if (Env::FsInterface::fs()->pread(err, smartfd, pread_offset, buf_start, data_start.length())
          != data_start.length()
          || err != Error::OK
          || smartfd->pos() != pread_offset+data_start.length()
          || !Condition::str_eq(reinterpret_cast<char*>(buf_start), data_start.c_str(), data_start.length())) {
        std::cerr << "ERROR(pread) err=" << err << " buf=" << buf_start << " " << smartfd->to_string() <<"\n";
        exit(1);
      }
      pread_offset+=block_sz;
      std::cout << "pread thread=" << thread_id << " blk=" << i
                << " start_data='"
                << std::string(reinterpret_cast<char*>(buf_start), data_start.length()) << "'\n";
    }

    // pread(with EOF) >>
    err = Error::OK;
    uint8_t bufpeof[6];
    if (Env::FsInterface::fs()->pread(err, smartfd,
                                      len-data_end.length()+1,
                                      bufpeof, data_end.length()) != data_end.length()-1
        || err != Error::FS_EOF
        || !Condition::str_eq(reinterpret_cast<char*>(bufpeof), data_end.data()+1, data_end.length()-1 )) {
     std::cerr << "ERROR(pread(with EOF)) err=" << err << " buf=" << bufpeof << " " << smartfd->to_string() <<"\n";
     exit(1);
    }
    std::cout << "pread(with EOF)-data='"
              << std::string(reinterpret_cast<char*>(bufpeof), data_end.length()-1) << "'\n";



    // close >>
    err = Error::OK;
    Env::FsInterface::fs()->close(err, smartfd);
    if(err != Error::OK){
     std::cerr << "ERROR(close,open) err=" << err << "\n";
     exit(1);
    }

    err = Error::OK;
    Env::FsInterface::fs()->rename(err, smartfd->filepath(), smartfd->filepath()+"new");
    if(err != Error::OK){
     std::cerr << "ERROR(rename) err=" << err << "\n";
     exit(1);
    }
    if(!Env::FsInterface::fs()->exists(err, smartfd->filepath()+"new") || err != Error::OK){
     std::cerr << "ERROR(rename) exists err=" << err << "\n";
     exit(1);
    }

    err = Error::OK;
    Env::FsInterface::fs()->remove(err, smartfd->filepath());
    if(err != Error::OK){
     std::cerr << "ERROR(remove) written-file err=" << err << "\n";
     exit(1);
    }

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

int main(int argc, char** argv) {
  SWC::Env::Config::init(argc, argv, &init_app_options, nullptr);

  Env::FsInterface::init(
    Env::Config::settings(),
    FS::fs_type(Env::Config::settings()->get_str("swc.fs"))
  );

  for(size_t chk=1;chk<=2;++chk) {
    int err = Error::OK;
    // mkdirs data-root
    Env::FsInterface::fs()->mkdirs(err, "a/child/folder");
    if(err != Error::OK){
      std::cerr << "ERROR(mkdirs data-root) mkdirs err=" << err << "\n";
      exit(1);
    }
    // aswell, mkdir -pf
    Env::FsInterface::fs()->mkdirs(err, "a/child/folder");
    if(err != Error::OK) {
      std::cerr << "ERROR(mkdirs -p data-root) mkdirs err=" << err << "\n";
      exit(1);
    }

    std::cout << "--1--\n";
    std::vector<std::thread*> threads;
    for(size_t t=1;t<=2;++t)
      threads.push_back(new std::thread([t](){run(t);}));

    std::cout << "--2--\n";
    for(auto& t : threads) {
      t->join();
      delete t;
    }
    std::cout << "--3--\n";


    err = Error::OK;
    // remove data-root
    Env::FsInterface::fs()->rmdir(err, "");
    if(err != Error::OK){
      std::cerr << "ERROR(rmr data-root) rmdir err=" << err << "\n";
      exit(1);
    }

    std::cout << "--######################-- chk=" <<chk<< "\n";
    //std::this_thread::sleep_for(std::chrono::milliseconds(10000));
  }

  //for(size_t chk=1;chk<=10;++chk)
  //  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  //std::cout << " fs()->stop\n";
  Env::FsInterface::fs()->stop();

  //for(size_t chk=1;chk<=100;++chk)
  //  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  std::this_thread::sleep_for(std::chrono::milliseconds(100));
  std::cout << " ## OK! ##\n" << std::endl;
  exit(0);
}