#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>




SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror") # -Wextra
SET (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wformat-security")


SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wpedantic") # -Weffc++
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat-security -Wformat-nonliteral")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wcast-align -Wcast-qual -Wnon-virtual-dtor")

SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wzero-as-null-pointer-constant -Wno-error=zero-as-null-pointer-constant")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wold-style-cast -Wno-error=old-style-cast")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnull-dereference -Wno-error=null-dereference")
SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wdeprecated-copy-dtor -Wno-error=deprecated-copy-dtor")
# SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++ -Wno-error=non-virtual-dtor")


if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wformat-signedness")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnoexcept -Wno-error=noexcept")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-override -Wno-error=suggest-override")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wuseless-cast -Wno-error=useless-cast")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-null-sentinel -Wno-error=strict-null-sentinel")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicated-cond -Wno-error=duplicated-cond")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wduplicated-branches -Wno-error=duplicated-branches")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wlogical-op -Wno-error=logical-op")

  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wshadow=compatible-local -Wshadow=local -Wshadow=global -Wshadow")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=shadow=compatible-local -Wno-error=shadow=local -Wno-error=shadow=global -Wno-error=shadow")

  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstack-usage=16384 -Wno-error=stack-usage=16384")

  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wtrampolines")

  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-attribute=const -Wno-error=suggest-attribute=const")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-attribute=pure -Wno-error=suggest-attribute=pure")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-attribute=malloc -Wno-error=suggest-attribute=malloc")
  SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-attribute=cold -Wno-error=suggest-attribute=cold")
  # SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-attribute=noreturn -Wno-error=suggest-attribute=noreturn")

  # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Walloc-zero -Wno-error=alloc-zero")
  # SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-final-types -Wno-error=suggest-final-types")

  if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 10.0.0)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Warith-conversion")
  endif()

  if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 11.0.0)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wtsan")
  endif()

  if (NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 12.0.0)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wbidi-chars=any")
  endif()


endif()
