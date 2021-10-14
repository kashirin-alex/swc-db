/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */

#include "swcdb/fs/SmartFd.h"
#include "swcdb/core/String.h"


namespace SWC { namespace FS {


SmartFd::~SmartFd() noexcept { }

std::string SmartFd::to_string() const {
  return format("Fd('%s' flags=%u fd=%d pos=" SWC_FMT_LU ")",
    m_filepath.c_str(), m_flags.load(), m_fd.load(), m_pos.load());
}

void SmartFd::print(std::ostream& out) const {
  out << "Fd('" << m_filepath
      << "' flags=" << m_flags.load()
      << " fd=" << m_fd.load()
      << " pos=" << m_pos.load()
      << ')';
}


}}

