/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
 * License details at <https://github.com/kashirin-alex/swc-db/#license>
 */


#include "swcdb/fs/Statistics.h"


namespace SWC { namespace FS {


void Statistics::Metric::add(bool err, uint64_t ns) noexcept {
  lock();
  if(UINT64_MAX - m_total > ns) {
    ++m_count;
    m_total += ns;
  }
  if(err)
    ++m_error;
  if(!m_min || m_min > ns)
    m_min = ns;
  if(m_max < ns)
    m_max = ns;
  unlock();
}

void Statistics::Metric::gather(Metric& m) noexcept {
  lock();
  m.m_error = m_error;
  m.m_count = m_count;
  m.m_total = m_total;
  m.m_min = m_min;
  m.m_max = m_max;
  m_error = 0;
  m_count = 0;
  m_max = m_min = m_total = 0;
  unlock();
}

void Statistics::Metric::reset() noexcept {
  lock();
  m_error = 0;
  m_count = 0;
  m_max = m_min = m_total = 0;
  unlock();
}



void Statistics::gather(Statistics& stats) noexcept {
  for(uint8_t i = 0; i < Command::MAX; ++i) {
    metrics[i].gather(stats.metrics[i]);
  }
}

void Statistics::reset() noexcept {
  for(auto& m : metrics) {
    m.reset();
  }
}

const char* Statistics::to_string(Command cmd) noexcept {
  switch(cmd) {
    case OPEN_SYNC:
      return "OPEN_SYNC";
    case OPEN_ASYNC:
      return "OPEN_ASYNC";
    case CREATE_SYNC:
      return "CREATE_SYNC";
    case CREATE_ASYNC:
      return "CREATE_ASYNC";
    case CLOSE_SYNC:
      return "CLOSE_SYNC";
    case CLOSE_ASYNC:
      return "CLOSE_ASYNC";
    case READ_SYNC:
      return "READ_SYNC";
    case READ_ASYNC:
      return "READ_ASYNC";
    case APPEND_SYNC:
      return "APPEND_SYNC";
    case APPEND_ASYNC:
      return "APPEND_ASYNC";
    case SEEK_SYNC:
      return "SEEK_SYNC";
    case SEEK_ASYNC:
      return "SEEK_ASYNC";
    case REMOVE_SYNC:
      return "REMOVE_SYNC";
    case REMOVE_ASYNC:
      return "REMOVE_ASYNC";
    case LENGTH_SYNC:
      return "LENGTH_SYNC";
    case LENGTH_ASYNC:
      return "LENGTH_ASYNC";
    case PREAD_SYNC:
      return "PREAD_SYNC";
    case PREAD_ASYNC:
      return "PREAD_ASYNC";
    case MKDIRS_SYNC:
      return "MKDIRS_SYNC";
    case MKDIRS_ASYNC:
      return "MKDIRS_ASYNC";
    case FLUSH_SYNC:
      return "FLUSH_SYNC";
    case FLUSH_ASYNC:
      return "FLUSH_ASYNC";
    case RMDIR_SYNC:
      return "RMDIR_SYNC";
    case RMDIR_ASYNC:
      return "RMDIR_ASYNC";
    case READDIR_SYNC:
      return "READDIR_SYNC";
    case READDIR_ASYNC:
      return "READDIR_ASYNC";
    case EXISTS_SYNC:
      return "EXISTS_SYNC";
    case EXISTS_ASYNC:
      return "EXISTS_ASYNC";
    case RENAME_SYNC:
      return "RENAME_SYNC";
    case RENAME_ASYNC:
      return "RENAME_ASYNC";
    case SYNC_SYNC:
      return "SYNC_SYNC";
    case SYNC_ASYNC:
      return "SYNC_ASYNC";
    case WRITE_SYNC:
      return "WRITE_SYNC";
    case WRITE_ASYNC:
      return "WRITE_ASYNC";
    case READ_ALL_SYNC:
      return "READ_ALL_SYNC";
    case READ_ALL_ASYNC:
      return "READ_ALL_ASYNC";
    case COMBI_PREAD_SYNC:
      return "COMBI_PREAD_SYNC";
    case COMBI_PREAD_ASYNC:
      return "COMBI_PREAD_ASYNC";
    default:
      return "UNKNOWN";
  }
}

}}
