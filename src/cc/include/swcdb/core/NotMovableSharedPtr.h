/*
 * SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
*/

#ifndef swcdb_core_NotMovableSharedPtr_h
#define swcdb_core_NotMovableSharedPtr_h


namespace SWC { namespace Core {


template<class T>
class NotMovableSharedPtr : public std::shared_ptr<T> {
  public:

  constexpr SWC_CAN_INLINE
  NotMovableSharedPtr() noexcept { }

  SWC_CAN_INLINE
  NotMovableSharedPtr(T* other) noexcept : std::shared_ptr<T>(other) { }

  SWC_CAN_INLINE
  NotMovableSharedPtr(const std::shared_ptr<T>& other) noexcept
                    : std::shared_ptr<T>(other) {
  }

  NotMovableSharedPtr(const NotMovableSharedPtr&) noexcept = default;

  NotMovableSharedPtr(NotMovableSharedPtr&&) noexcept = delete;
  NotMovableSharedPtr<T>& operator=(NotMovableSharedPtr&&) noexcept = delete;
  NotMovableSharedPtr<T>& operator=(const NotMovableSharedPtr&) = delete;

  NotMovableSharedPtr(std::shared_ptr<T>&&) noexcept = delete;
  NotMovableSharedPtr<T>& operator=(std::shared_ptr<T>&&) noexcept = delete;

  ~NotMovableSharedPtr() noexcept { }
};


}}

#endif // swcdb_core_NotMovableSharedPtr_h
