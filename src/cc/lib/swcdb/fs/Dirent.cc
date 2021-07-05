/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Dirent.h"


namespace SWC { namespace FS {



std::string Dirent::to_string() const {
  std::string s("Dirent(");
  s.append("name=");
  s.append(name);
  s.append(" modified=");
  s.append(std::to_string(last_modification_time));
  s.append(" is_dir=");
  s.append(std::to_string(is_dir));
  if(!is_dir) {
    s.append(" length=");
    s.append(std::to_string(length));
  }
  s.append(")\n");
  return s;
}



}}
