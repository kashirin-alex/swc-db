/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/utils/cli/Shell_Fs.h"
#include "swcdb/core/Comparators.h"
#include "swcdb/fs/Interface.h"
#include <iomanip>


namespace SWC { namespace Utils { namespace shell {

Fs::Fs()
  : Interface("\033[32mSWC-DB(\033[36mfs-" +
              Env::Config::settings()->get_str("swc.fs") +
              "\033[32m)\033[33m> \033[00m",
              "/tmp/.swc-cli-fs-" +
              Env::Config::settings()->get_str("swc.fs") + "-history") {

  Env::FsInterface::init(
    Env::Config::settings(),
    FS::fs_type(Env::Config::settings()->get_str("swc.fs"))
  );

  options.push_back(
    new Option(
      "list",
      {"list directory contents",
       "list 'path';"},
      [ptr=this](const std::string& cmd){ return ptr->ls(cmd); },
      new re2::RE2("(?i)^(ls|list)")
    )
  );
}

Fs::~Fs() {
  Env::FsInterface::interface()->stop();
  #if defined(SWC_ENABLE_SANITIZER)
    std::this_thread::sleep_for(std::chrono::seconds(2));
    Env::FsInterface::reset();
  #endif
}

bool Fs::ls(const std::string& cmd) {

  std::string path;
  RE2::FullMatch(cmd, "(?i)^(ls|list)\\s+(.*)$", nullptr, &path);
  if(path.front() == '"' || path.front() == '\'')
    path = path.substr(1, path.size()-2);

  SWC_PRINT << "Listing path='" << path << "':" << SWC_PRINT_CLOSE;
  FS::DirentList entries;
  Env::FsInterface::fs()->readdir(err, path, entries);
  if(err)
    return error("Problem reading '"+path+"'");

  size_t lname = 0;
  Core::Vector<FS::Dirent*> dirs;
  Core::Vector<FS::Dirent*> files;
  dirs.reserve(entries.size());
  files.reserve(entries.size());
  for(auto& entry : entries) {
    if(entry.name.size() > lname)
      lname = entry.name.size();

    auto& tmp = entry.is_dir ? dirs : files;
    for(auto it = tmp.cbegin(); ; ++it) {
      if(it == tmp.cend() ||
         Condition::lt_volume(
           reinterpret_cast<const uint8_t*>((*it)->name.c_str()),
           (*it)->name.size(),
           reinterpret_cast<const uint8_t*>(entry.name.c_str()),
           entry.name.size() ) ) {
        tmp.insert(it, &entry);
        break;
      }
    }
  }

  time_t t_secs;
  char modified[20];

  SWC_PRINT << "Directories=" << dirs.size() << ":\n";
  for(auto& entry : dirs) {
    t_secs = entry->last_modification_time;
    std::strftime(modified, 20, "%Y/%m/%d %H:%M:%S", std::gmtime(&t_secs));

    SWC_LOG_OSTREAM << std::left << "  "
      << std::left << std::setw(lname + 2)
      << format("'%s'", entry->name.c_str())
      << std::left << "  modified=" << modified
      << '\n';
  }

  SWC_LOG_OSTREAM << "Files=" << files.size() << ":\n";
  for(auto& entry : files) {
    t_secs = entry->last_modification_time;
    std::strftime(modified, 20, "%Y/%m/%d %H:%M:%S", std::gmtime(&t_secs));

    SWC_LOG_OSTREAM << std::left << "  "
      << std::left << std::setw(lname + 2)
      << format("'%s'", entry->name.c_str())
      << std::left << "  modified=" << modified
      << "  size=" << entry->length
      << '\n';
  }

  SWC_LOG_OSTREAM << SWC_PRINT_CLOSE;
  return true;
}



}}} // namespace Utils::shell
