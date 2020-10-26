/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
*/

#ifndef swcdb_core_NotMovableSharedPtr_h
#define swcdb_core_NotMovableSharedPtr_h

#include <memory>

namespace SWC { namespace Core {


template<class T>
class NotMovableSharedPtr : public std::shared_ptr<T> {
  public:

  NotMovableSharedPtr<T>() { }

  NotMovableSharedPtr<T>(T* other) 
                    : std::shared_ptr<T>(other) {
  }

  NotMovableSharedPtr<T>(const std::shared_ptr<T>& other) 
                    : std::shared_ptr<T>(other) {
  }

  NotMovableSharedPtr<T>(const std::shared_ptr<T>&&) = delete;
  
  NotMovableSharedPtr<T>& operator=(const std::shared_ptr<T>&) = delete;
};


}}

#endif // swcdb_core_NotMovableSharedPtr_h
