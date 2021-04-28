#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>

####
# File Content:
#
#  Build Configuration
#    options, case-sensitive:
#     - BUILD_LINKING =         STATIC or SHARED, a major executable target (eg. server), default SHARED
#     - UTILS_LINKING =         STATIC or SHARED, executable utility target, default SHARED
#     - LIBS_STATIC_LINKING =   STATIC, a static option is evaluated to a bundled static-lib, default SHARED
#     - LIBS_SHARED_LINKING =   STATIC or SHARED, a static option is evaluated to a bundled shared-lib, default SHARED
#     - BUILD_LINKING_CORE =    STATIC or SHARED, whether to use CORE_LIBS_STATIC_FLAGS, useable with cases such -s -static-libgcc -static-libstdc++, default SHARED
#     - LIBS_LINKING_CHECKING = STATIC or SHARED or DUAL, considerations for find package variable
#     - WITHOUT_TESTS =         ON, skip tests, default OFF
#     - TEST_LINKING =          STATIC or SHARED or DUAL, tests are built on linking type, default SHARED
#     - INSTALL_TARGETS =       Install only these targets, default All(OFF)
#    REQUIRED_SHARED and REQUIRED_STATIC are set depending on options variations. in-case a library of type is missing, it is a FATAL not found error.
#
#  Functions:
#     * SET_DEPS
#     * GET_TARGET_LINKS
#     * GET_ARCHIVE_LINKS
#     * ADD_LIB_TARGET
#     * ADD_UTIL_TARGET
#     * ADD_EXEC_TARGET
#     * ADD_TEST_TARGET
#     * ADD_TEST_EXEC
#



SET(REQUIRED_SHARED OFF)
SET(REQUIRED_STATIC OFF)

##### BUILD_CONFIGS

message(" ------------------------ BUILD CONFIGURATIONS ----------------------- ")

message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "Version: ${VERSION_STRING}")
message(STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}")



# -------------- EXECUTABLES --------------------------
#
#
if(NOT BUILD_LINKING OR (NOT BUILD_LINKING STREQUAL "STATIC"))
  set(BUILD_LINKING "SHARED")
endif ()

if(BUILD_LINKING       STREQUAL "STATIC")
  SET(REQUIRED_STATIC ON)
elseif(BUILD_LINKING   STREQUAL "SHARED")
  SET(REQUIRED_SHARED ON)
endif ()

message(STATUS "Excutables Linking: ${BUILD_LINKING}")



# -------------- EXECUTABLE UTILITIES --------------------------
#
#
if(NOT UTILS_LINKING OR (NOT UTILS_LINKING STREQUAL "STATIC"))
  set(UTILS_LINKING "SHARED")
endif ()

if(UTILS_LINKING       STREQUAL "STATIC")
  SET(REQUIRED_STATIC ON)
elseif(UTILS_LINKING   STREQUAL "SHARED")
  SET(REQUIRED_SHARED ON)
endif ()

message(STATUS "Excutables Utilities Linking: ${UTILS_LINKING}")



# -------------- LIBRARIES --------------------------
#
#
message(STATUS "Shared and Static Libraries Linkings:")


# ----- LIBS_STATIC_LINKING
if(NOT LIBS_STATIC_LINKING OR (NOT LIBS_STATIC_LINKING STREQUAL "STATIC"))
  set(LIBS_STATIC_LINKING "NONE")
endif ()

if(LIBS_STATIC_LINKING       STREQUAL "STATIC")
  SET(REQUIRED_STATIC ON)
endif ()

message("       Static libraries Linking: ${LIBS_STATIC_LINKING}")


# ----- LIBS_SHARED_LINKING
if(NOT LIBS_SHARED_LINKING OR (NOT LIBS_SHARED_LINKING STREQUAL "STATIC"))
  set(LIBS_SHARED_LINKING "SHARED")
endif ()

if(LIBS_SHARED_LINKING       STREQUAL "STATIC")
  SET(REQUIRED_STATIC ON)
elseif(LIBS_SHARED_LINKING   STREQUAL "SHARED")
  SET(REQUIRED_SHARED ON)
endif ()

message("       Shared libraries Linking: ${LIBS_SHARED_LINKING}")


# ----- BUILD_LINKING_CORE
if(NOT BUILD_LINKING_CORE OR (NOT BUILD_LINKING_CORE STREQUAL "STATIC"))
  set(BUILD_LINKING_CORE "SHARED")
endif ()

if(BUILD_LINKING_CORE       STREQUAL "STATIC")
  SET(REQUIRED_STATIC ON)
elseif(BUILD_LINKING_CORE   STREQUAL "SHARED")
  SET(REQUIRED_SHARED ON)
endif ()

message("       Linking of Core-Libraries: ${BUILD_LINKING_CORE}")


# ----- LIBRARIES CHECKINGS
SET(LIBS_LINKING_CHECKING_STATIC OFF)
SET(LIBS_LINKING_CHECKING_SHARED OFF)
if(LIBS_LINKING_CHECKING       STREQUAL "STATIC")
  SET(LIBS_LINKING_CHECKING_STATIC ON)
  SET(REQUIRED_STATIC ON)
elseif(LIBS_LINKING_CHECKING   STREQUAL "SHARED")
  SET(LIBS_LINKING_CHECKING_SHARED ON)
  SET(REQUIRED_SHARED ON)
elseif(LIBS_LINKING_CHECKING   STREQUAL "DUAL")
  SET(LIBS_LINKING_CHECKING_STATIC ON)
  SET(REQUIRED_STATIC ON)
  SET(LIBS_LINKING_CHECKING_SHARED ON)
  SET(REQUIRED_SHARED ON)
else()
  set(LIBS_LINKING_CHECKING OFF)
endif ()

message("       Libraries Checkings: ${LIBS_LINKING_CHECKING}")


# -------------- TESTS --------------------------
#
#
if( NOT WITHOUT_TESTS )
  enable_testing()

  if(NOT TEST_LINKING OR (NOT TEST_LINKING STREQUAL "STATIC" AND NOT TEST_LINKING STREQUAL "DUAL"))
    set(TEST_LINKING "SHARED")
  endif ()

  if(TEST_LINKING       STREQUAL "STATIC")
    SET(REQUIRED_STATIC ON)
  elseif(TEST_LINKING   STREQUAL "SHARED")
    SET(REQUIRED_SHARED ON)
  elseif(TEST_LINKING   STREQUAL "DUAL")
    SET(REQUIRED_STATIC ON)
    SET(REQUIRED_SHARED ON)
  endif ()
  message(STATUS "Tests Linking: ${TEST_LINKING}")

else()
  message(STATUS "Tests: WITHOUT_TESTS")
endif ()



message("")
message(STATUS "Static libs-lookup set to: ${REQUIRED_STATIC}")
message(STATUS "Shared libs-lookup set to: ${REQUIRED_SHARED}")

if(REQUIRED_SHARED)

  LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${INSTALL_DIR}/lib" isSystemDir)
  IF("${isSystemDir}" STREQUAL "-1")
    SET(CMAKE_INSTALL_RPATH "${INSTALL_DIR}/lib")
    SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  ENDIF("${isSystemDir}" STREQUAL "-1")
endif ()


message("")
message(STATUS "Build Type and Flags:")
message("       C FLAGS: ${CMAKE_CFLAGS}")
message("       CXX FLAGS: ${CMAKE_CXX_FLAGS}")
message("       BUILD TYPE: ${CMAKE_BUILD_TYPE}")
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
message("       BUILD TYPE FLAGS: ${CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}}")

message(" ------------------------ BUILD CONFIGURATIONS ----------------------- ")
message("")
message("")

# END BUILD_CONFIGS
# -------------------------------







##### SET_DEPS
# -------------------------------
  # SET_DEPS(
  #   NAME        CAP.NAME
  #   REQUIRED    BOOL
  #   LIB_PATHS   SEARCH_PATHS
  #   INC_PATHS   SEARCH_PATHS
  #   STATIC      STATIC_LIBS
  #   SHARED      SHARED_LIBS
  #   INCLUDE     HEADER FILES
  #   INSTALL     COPY & INSTALL
  # )

# Sets the following vars on parent-scope:
#   - CAP.NAME_FOUND
#   - CAP.NAME_LIBRARIES_STATIC
#   - CAP.NAME_LIBRARIES_SHARED
#   - CAP.NAME_INCLUDE_PATHS

# -------------------------------

function(SET_DEPS)
  cmake_parse_arguments(OPT "" "NAME;REQUIRED" "LIB_PATHS;INC_PATHS;STATIC;SHARED;INCLUDE;INSTALL" ${ARGN})


  # explicit lookup
  foreach(path ${OPT_INC_PATHS} ${LOOKUP_INCLUDE_PATHS})
    foreach(inc_h ${OPT_INCLUDE})
      if(EXISTS ${path}/${inc_h} AND NOT INCLUDE_DIR${inc_h})
        set(INCLUDE_DIR${inc_h} ${path})
      endif ()
    endforeach()

    set(INCLUDE_DIRS)
    foreach(inc_h ${OPT_INCLUDE})
      if(INCLUDE_DIR${inc_h})
        set(INCLUDE_DIRS ${INCLUDE_DIRS} ${INCLUDE_DIR${inc_h}})
      else()
        set(INCLUDE_DIRS)
        break()
      endif ()
    endforeach()

    if(INCLUDE_DIRS)
      list(REMOVE_DUPLICATES INCLUDE_DIRS)
      break()
    endif ()
  endforeach()


  FIND_LIBS(
    OUTPUT LIBRARIES
    PATHS  ${OPT_LIB_PATHS}
    STATIC ${OPT_STATIC}
    SHARED ${OPT_SHARED}
  )

  if ((NOT OPT_INCLUDE OR INCLUDE_DIRS) AND
      (NOT OPT_STATIC OR NOT REQUIRED_STATIC OR LIBRARIES_STATIC) AND
      (NOT OPT_SHARED OR NOT REQUIRED_SHARED OR LIBRARIES_SHARED))
    set("${OPT_NAME}_FOUND" TRUE)


    message(STATUS "Found ${OPT_NAME}:")
    if(LIBRARIES_STATIC AND REQUIRED_STATIC)
      list(REMOVE_DUPLICATES LIBRARIES_STATIC)
      set("${OPT_NAME}_LIBRARIES_STATIC" ${LIBRARIES_STATIC})
      message("       Static Libs: ${LIBRARIES_STATIC}")
    endif ()

    if(LIBRARIES_SHARED AND REQUIRED_SHARED)
      list(REMOVE_DUPLICATES LIBRARIES_SHARED)
      set("${OPT_NAME}_LIBRARIES_SHARED" ${LIBRARIES_SHARED})
      message("       Shared Libs: ${LIBRARIES_SHARED}")
    endif ()

    if(NOT UTILS_NO_INSTALL_DEP_LIBS AND OPT_INSTALL)
      if(LIBRARIES_SHARED)
        INSTALL_LIBS(DEST lib LIBS ${LIBRARIES_SHARED})
      endif()
      if(LIBRARIES_STATIC)
        INSTALL_LIBS(DEST lib LIBS ${LIBRARIES_STATIC} ARCHIVE TRUE)
      endif()
    endif()

    if(INCLUDE_DIRS)
      message("       Include path: ${INCLUDE_DIRS}")
      include_directories(${INCLUDE_DIRS})  # per target ?
    endif ()

  else ()

    set("${OPT_NAME}_FOUND" FALSE)
    set("${OPT_NAME}_LIBRARIES_STATIC")
    set("${OPT_NAME}_LIBRARIES_SHARED")
    set("${OPT_NAME}_INCLUDE_DIR")
    message(STATUS "Not Found ${OPT_NAME}: ${OPT_SHARED} ${OPT_STATIC} ${OPT_INCLUDE}")
    if (OPT_REQUIRED AND (OPT_INCLUDE OR OPT_STATIC OR OPT_SHARED))
      message(FATAL_ERROR "       Could NOT find ${OPT_NAME}")
    endif ()

  endif ()

  set("${OPT_NAME}_FOUND"  ${${OPT_NAME}_FOUND} PARENT_SCOPE)
  set("${OPT_NAME}_INCLUDE_PATHS" ${INCLUDE_DIRS} PARENT_SCOPE)
  set("${OPT_NAME}_LIBRARIES_STATIC" ${${OPT_NAME}_LIBRARIES_STATIC} PARENT_SCOPE)
  set("${OPT_NAME}_LIBRARIES_SHARED" ${${OPT_NAME}_LIBRARIES_SHARED} PARENT_SCOPE)

endfunction()
# END SET_DEPS
# -------------------------------



##### FIND_LIBS
# -------------------------------
  # FIND_LIBS(
  #   OUTPUT VAR_NAME
  #   PATHS  SEARCH_PATHS
  #   STATIC STATIC_LIBS
  #   SHARED SHARED_LIBS
  # )
# -------------------------------

function(FIND_LIBS)
  cmake_parse_arguments(OPT "" "OUTPUT" "PATHS;STATIC;SHARED" ${ARGN})


  # --- STATIC LIBS
  if(REQUIRED_STATIC AND OPT_STATIC)
  set("${OPT_OUTPUT}_STATIC" "")
  foreach(lib ${OPT_STATIC})
    # message(STATUS "looking for: ${lib}")
    find_library(
      FOUND_${lib}
      NAMES ${lib}
      PATHS ${OPT_PATHS} ${LOOKUP_LIB_PATHS}
    )
    if(FOUND_${lib})
      set("${OPT_OUTPUT}_STATIC" ${${OPT_OUTPUT}_STATIC} ${FOUND_${lib}})
    endif()
  endforeach()

  set("${OPT_OUTPUT}_STATIC" ${${OPT_OUTPUT}_STATIC} PARENT_SCOPE)
  # message(STATUS "FOUND_LIB: ${OPT_OUTPUT} ${${OPT_OUTPUT}}")

  endif ()

  # --- SHARED LIBS
  if(REQUIRED_SHARED AND OPT_SHARED)
  set("${OPT_OUTPUT}_SHARED" "")
  foreach(lib ${OPT_SHARED})
    # message(STATUS "looking for: ${lib}")
    find_library(
      FOUND_${lib}
      NAMES ${lib}
      PATHS ${OPT_PATHS} ${LOOKUP_LIB_PATHS}
    )
    if(FOUND_${lib})
      set(real_path)
      foreach(fpath ${FOUND_${lib}})
        get_filename_component(tmp ${fpath} REALPATH)
        set(real_path ${real_path} ${tmp})
      endforeach()
      set("${OPT_OUTPUT}_SHARED" ${${OPT_OUTPUT}_SHARED} ${real_path})
    endif()
  endforeach()

  set("${OPT_OUTPUT}_SHARED" ${${OPT_OUTPUT}_SHARED} PARENT_SCOPE)
  # message(STATUS "FOUND_LIB: ${OPT_OUTPUT} ${${OPT_OUTPUT}}")

  endif ()

endfunction()
# END FIND_LIBS
# -------------------------------


##### INSTALL_LIBS
# -------------------------------
  # INSTALL_LIBS(
  #   DEST VAR_NAME
  #   LIBS LIBRARIES
  #   ARCHIVE
  # )
# -------------------------------

function(INSTALL_LIBS)
  cmake_parse_arguments(OPT "" "DEST" "LIBS;ARCHIVE" ${ARGN})
  set(msg )
  if (OPT_ARCHIVE)
    set(msg "static")
  else()
    set(msg "shared")
  endif()
  message("       Copying & Installing ${msg}:")

  foreach(fpath ${OPT_LIBS})
    set(soname )
    if(NOT OPT_ARCHIVE)
      exec_program(bash ARGS ${CMAKE_CURRENT_LIST_DIR}/soname.sh ${fpath}
                  OUTPUT_VARIABLE SONAME_OUT RETURN_VALUE SONAME_RETURN)
      if (SONAME_RETURN STREQUAL "0")
        set(soname ${SONAME_OUT})
      endif()
    endif()

    if (NOT soname)
      get_filename_component(soname ${fpath} NAME)
    endif()
    get_filename_component(dir ${fpath} DIRECTORY)

    message("         ${dir}/${soname}")
    configure_file(${dir}/${soname} "${OPT_DEST}/${soname}" COPYONLY)
    install(FILES "${CMAKE_BINARY_DIR}/${OPT_DEST}/${soname}" DESTINATION ${OPT_DEST})
  endforeach()

endfunction()
# END INSTALL_LIBS
# -------------------------------



##### GET_TARGET_LINKS
# -------------------------------
  # GET_TARGET_LINKS(
  #   FOR           target type EXEC/LIB, optional
  #   TARGETS       targets to check for links
  #   STATIC        target's specific static links
  #   SHARED        target's specific shared links
  #   WITH_MALLOC   add malloc libs
  # )
# result:
#     SHARED_TARGETS, STATIC_TARGETS, SHARED_LINKING and STATIC_LINKING set to parent-scope
# -------------------------------

function(GET_TARGET_LINKS)
  cmake_parse_arguments(OPT "" "FOR" "TARGETS;STATIC;SHARED;WITH_MALLOC" ${ARGN})

  set(STATIC_LINKING ${OPT_STATIC})
  set(SHARED_LINKING ${OPT_SHARED})
  set(STATIC_TARGETS )
  set(SHARED_TARGETS )

  foreach(TARGET ${OPT_TARGETS})
    # set(SHARED_TARGETS ${SHARED_TARGETS} ${TARGET}-shared)
    GET_ARCHIVE_LINKS(NAME ${TARGET}
                      STATIC_TARGETS ${STATIC_TARGETS} STATIC_LINKING ${STATIC_LINKING}
                      SHARED_TARGETS ${SHARED_TARGETS} SHARED_LINKING ${SHARED_LINKING})
  endforeach()
  if(BUILD_LINKING_CORE STREQUAL "STATIC") # AND OPT_FOR STREQUAL "EXEC")
    set(STATIC_LINKING ${STATIC_LINKING} ${CORE_LIBS_STATIC})
  endif ()

  if(OPT_WITH_MALLOC)
    set(STATIC_LINKING ${STATIC_LINKING} ${MALLOC_LIBRARIES_STATIC})
  endif()
  if(STATIC_LINKING)
    list(REMOVE_DUPLICATES STATIC_LINKING)
    string(REPLACE ";" " " STATIC_LINKING "-Wl,--whole-archive ${STATIC_LINKING} -Wl,--no-whole-archive")
  endif()

  if(BUILD_LINKING_CORE STREQUAL "STATIC")
    set(STATIC_LINKING ${STATIC_LINKING} ${CORE_LIBS_STATIC_FLAGS})
  endif ()

  set(STATIC_LINKING ${STATIC_LINKING} ${CORE_LIBS})
  set("STATIC_LINKING" ${STATIC_LINKING} PARENT_SCOPE)

  if(STATIC_TARGETS)
    list(REMOVE_DUPLICATES STATIC_TARGETS)
  endif()
  set("STATIC_TARGETS" ${STATIC_TARGETS} PARENT_SCOPE)


  if(SHARED_TARGETS)
    list(REMOVE_DUPLICATES SHARED_TARGETS)
  endif()
  set("SHARED_TARGETS" ${SHARED_TARGETS} PARENT_SCOPE)

  set(SHARED_LINKING ${SHARED_LINKING} ${CORE_LIBS_SHARED} ${CORE_LIBS})
  if(OPT_WITH_MALLOC)
    set(SHARED_LINKING ${SHARED_LINKING} ${MALLOC_LIBRARIES_SHARED})
  endif()
  if(SHARED_LINKING)
    list(REMOVE_DUPLICATES SHARED_LINKING)
  endif()
  set("SHARED_LINKING" ${SHARED_LINKING} PARENT_SCOPE)

endfunction()
# END GET_TARGET_LINKS
# -------------------------------




##### GET_ARCHIVE_LINKS
# -------------------------------
  # GET_ARCHIVE_LINKS(
  #   NAME                TARGET FOR GETTING IT'S LINKINGS
  #   STATIC_TARGETS      aggregated targets
  #   STATIC_LINKING      aggregated static links
  #   SHARED_TARGETS      aggregated targets
  #   SHARED_LINKING      aggregated static links
  # )
# result:
#     STATIC_TARGETS, STATIC_LINKING and SHARED_TARGETS, SHARED_LINKING set to parent-scope
# -------------------------------

function(GET_ARCHIVE_LINKS)
  cmake_parse_arguments(OPT "" "NAME" "STATIC_TARGETS;STATIC_LINKING;SHARED_TARGETS;SHARED_LINKING" ${ARGN})

  # message("GET_ARCHIVE_LINKS: ${OPT_NAME}")

  set(STATIC_TARGETS ${OPT_STATIC_TARGETS} ${OPT_NAME}-archive)
  get_property(links GLOBAL PROPERTY ${OPT_NAME}-ARCHIVE_STATIC)
  set(STATIC_LINKING ${OPT_STATIC_LINKING} ${links})

  set(SHARED_TARGETS ${OPT_SHARED_TARGETS} ${OPT_NAME}-shared)
  get_property(links GLOBAL PROPERTY ${OPT_NAME}-ARCHIVE_SHARED)
  set(SHARED_LINKING ${OPT_SHARED_LINKING} ${links})

  get_property(targets GLOBAL PROPERTY ${OPT_NAME}-ARCHIVE_TARGETS)

  foreach(INNER_TARGET ${targets})
    GET_ARCHIVE_LINKS(NAME ${INNER_TARGET}
                      STATIC_TARGETS ${STATIC_TARGETS} STATIC_LINKING ${STATIC_LINKING}
                      SHARED_TARGETS ${SHARED_TARGETS} SHARED_LINKING ${SHARED_LINKING})
  endforeach()

  if(STATIC_LINKING)
    list(REMOVE_DUPLICATES STATIC_LINKING)
  endif()
  set("STATIC_LINKING" ${STATIC_LINKING} PARENT_SCOPE)

  if(STATIC_TARGETS)
    list(REMOVE_DUPLICATES STATIC_TARGETS)
  endif()
  set("STATIC_TARGETS" ${STATIC_TARGETS} PARENT_SCOPE)

  if(SHARED_LINKING)
    list(REMOVE_DUPLICATES SHARED_LINKING)
  endif()
  set("SHARED_LINKING" ${SHARED_LINKING} PARENT_SCOPE)

  if(SHARED_TARGETS)
    list(REMOVE_DUPLICATES SHARED_TARGETS)
  endif()
  set("SHARED_TARGETS" ${SHARED_TARGETS} PARENT_SCOPE)

endfunction()
# END GET_ARCHIVE_LINKS
# -------------------------------




##### ADD_LIB_TARGET
# -------------------------------
  # ADD_LIB_TARGET(
  #   NAME          libName
  #   SRCS          sourceToCompile
  #   TARGETS       targets-dependant
  #   STATIC        dependenciesOfTheTarget - static
  #   SHARED        dependenciesOfTheTarget - shared
  #   FLAGS         compiler flags
  #   ONLY_STATIC_SHARED TRUE/FALSE - optional
  #   ONLY_SHARED TRUE/FALSE - optional
  #   SOVERSION     Library SO VERSION
  # )
# -------------------------------

function(ADD_LIB_TARGET)
  cmake_parse_arguments(OPT "ONLY_STATIC_SHARED" "NAME" "SRCS;TARGETS;STATIC;SHARED;FLAGS;FOR;ONLY_SHARED;SOVERSION" ${ARGN})

  set(STATIC_LINKING ${OPT_STATIC})
  set(STATIC_TARGETS )
  set(SHARED_LINKING ${OPT_SHARED})
  set(SHARED_TARGETS )
  GET_TARGET_LINKS(FOR ${OPT_FOR} TARGETS ${OPT_TARGETS} STATIC ${STATIC_LINKING} SHARED ${SHARED_LINKING})



  # OBJECT
  add_library(obj_${OPT_NAME} OBJECT ${OPT_SRCS})
  set_property(TARGET obj_${OPT_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
  target_compile_options(obj_${OPT_NAME} PRIVATE ${OPT_FLAGS})

  # ARCHIVE - intermidiate form
  add_library(${OPT_NAME}-archive STATIC $<TARGET_OBJECTS:obj_${OPT_NAME}>)
  target_link_libraries(${OPT_NAME}-archive LINK_PRIVATE ${STATIC_TARGETS})
  target_link_options(${OPT_NAME}-archive PRIVATE ${OPT_FLAGS})

  set_property(GLOBAL PROPERTY ${OPT_NAME}-ARCHIVE_SHARED   "${OPT_SHARED}")
  set_property(GLOBAL PROPERTY ${OPT_NAME}-ARCHIVE_STATIC   "${OPT_STATIC}")
  set_property(GLOBAL PROPERTY ${OPT_NAME}-ARCHIVE_TARGETS  "${OPT_TARGETS}")


  # STATIC LIBS
  add_library(${OPT_NAME}-static STATIC $<TARGET_OBJECTS:obj_${OPT_NAME}>)
  SET_TARGET_PROPERTIES(${OPT_NAME}-static PROPERTIES OUTPUT_NAME ${OPT_NAME} CLEAN_DIRECT_OUTPUT 1)
  target_compile_options(${OPT_NAME}-static PRIVATE ${OPT_FLAGS})
  target_link_options(${OPT_NAME}-static PRIVATE ${OPT_FLAGS})

  # SHARED LIBS
  add_library(${OPT_NAME}-shared SHARED $<TARGET_OBJECTS:obj_${OPT_NAME}>)
  SET_TARGET_PROPERTIES(${OPT_NAME}-shared PROPERTIES OUTPUT_NAME ${OPT_NAME} CLEAN_DIRECT_OUTPUT 1)
  target_compile_options(${OPT_NAME}-shared PRIVATE ${OPT_FLAGS})
  target_link_options(${OPT_NAME}-shared PRIVATE ${OPT_FLAGS})

  if(OPT_SOVERSION)
    # set_target_properties(${OPT_NAME}-static PROPERTIES SOVERSION ${OPT_SOVERSION})
    set_target_properties(${OPT_NAME}-shared PROPERTIES SOVERSION ${OPT_SOVERSION})
  endif()

  # SHARED AND STATIC LIBS - WITH STATIC LINKING

  if(NOT OPT_ONLY_SHARED AND (
    OPT_ONLY_STATIC_SHARED OR LIBS_STATIC_LINKING STREQUAL "STATIC" OR LIBS_SHARED_LINKING STREQUAL "STATIC"))

    if(LIBS_STATIC_LINKING STREQUAL "STATIC")
      target_link_libraries(${OPT_NAME}-static LINK_PRIVATE ${STATIC_LINKING} ${STATIC_TARGETS})
    endif ()
    if(OPT_ONLY_STATIC_SHARED OR LIBS_SHARED_LINKING STREQUAL "STATIC")
      target_link_libraries(${OPT_NAME}-shared LINK_PRIVATE ${STATIC_LINKING} ${STATIC_TARGETS})
    endif ()

  endif ()


  # SHARED LIBS - WITH SHARED LINKING
  if(OPT_ONLY_SHARED OR (NOT OPT_ONLY_STATIC_SHARED AND LIBS_SHARED_LINKING STREQUAL "SHARED"))
    target_link_libraries(${OPT_NAME}-shared LINK_PRIVATE ${SHARED_LINKING} ${SHARED_TARGETS})
  endif ()

  if(NOT INSTALL_TARGETS OR ${OPT_NAME} IN_LIST INSTALL_TARGETS)
    install(TARGETS ${OPT_NAME}-static ARCHIVE DESTINATION lib)
    install(TARGETS ${OPT_NAME}-shared LIBRARY DESTINATION lib)
  endif ()

  # message(STATUS ${OPT_NAME} ": STATIC_TARGETS:${STATIC_TARGETS} STATIC_LINKING:${STATIC_LINKING} "
  #                           ", SHARED_TARGETS:${SHARED_TARGETS} SHARED_LINKING:${SHARED_LINKING} ")
endfunction()
# END ADD_LIB_TARGET
# -------------------------------




##### ADD_UTIL_TARGET
# -------------------------------
  # ADD_UTIL_TARGET(
  #   util_name
  #   SRCS        sourcesToCompile
  #   TARGETS     targets
  #   STATIC      static linking
  #   SHARED      shared linking
  #   FLAGS       compiler flags
  # )
# -------------------------------

function(ADD_UTIL_TARGET)
  cmake_parse_arguments(OPT "" "NAME" "SRCS;TARGETS;STATIC;SHARED;FLAGS" ${ARGN})


  set(STATIC_LINKING ${OPT_STATIC})
  set(STATIC_TARGETS )
  set(SHARED_LINKING ${OPT_SHARED})
  set(SHARED_TARGETS )
  GET_TARGET_LINKS(TARGETS ${OPT_TARGETS} STATIC ${STATIC_LINKING} SHARED ${SHARED_LINKING} WITH_MALLOC TRUE)

  if(UTILS_LINKING       STREQUAL "STATIC")
    set(TARGETS_LINKED  ${STATIC_TARGETS})
    set(LINKED_LIBS     ${STATIC_LINKING})

  elseif(UTILS_LINKING  STREQUAL "SHARED")
    set(TARGETS_LINKED  ${SHARED_TARGETS})
    set(LINKED_LIBS     ${SHARED_LINKING})
  endif()

  add_executable(${OPT_NAME} ${OPT_SRCS})
  target_compile_options(${OPT_NAME} PRIVATE ${OPT_FLAGS} ${MALLOC_FLAGS})
  target_link_libraries(${OPT_NAME} ${LINKED_LIBS} ${TARGETS_LINKED})
  target_link_options(${OPT_NAME} PRIVATE ${OPT_FLAGS})

  if(NOT INSTALL_TARGETS OR ${OPT_NAME} IN_LIST INSTALL_TARGETS)
    install(TARGETS ${OPT_NAME} RUNTIME DESTINATION bin)
  endif()

  # message(STATUS ${OPT_NAME} ": STATIC_TARGETS:${STATIC_TARGETS} STATIC_LINKING:${STATIC_LINKING} "
  #                            ", SHARED_TARGETS:${SHARED_TARGETS} SHARED_LINKING:${SHARED_LINKING} ")
endfunction()
# END ADD_UTIL_TARGET
# -------------------------------




##### ADD_EXEC_TARGET
# -------------------------------
  # ADD_EXEC_TARGET(
  #   NAME       exec_name
  #   SRCS       sourcesToCompile
  #   TARGETS    targets
  #   STATIC     static linking
  #   SHARED     shared linking
  #   FLAGS      Flags for compile options
  #   ONLY_DYN   link only shared
  # )
# -------------------------------

function(ADD_EXEC_TARGET)
  cmake_parse_arguments(OPT "" "NAME" "SRCS;TARGETS;STATIC;SHARED;FLAGS;ONLY_DYN" ${ARGN})


  set(STATIC_LINKING ${OPT_STATIC})
  set(STATIC_TARGETS )
  set(SHARED_LINKING ${OPT_SHARED})
  set(SHARED_TARGETS )
  GET_TARGET_LINKS(TARGETS ${OPT_TARGETS} STATIC ${STATIC_LINKING} SHARED ${SHARED_LINKING} WITH_MALLOC TRUE)

  if(NOT OPT_ONLY_DYN AND BUILD_LINKING  STREQUAL "STATIC")
    set(TARGETS_LINKED  ${STATIC_TARGETS})
    set(LINKED_LIBS     ${STATIC_LINKING})
  elseif(OPT_ONLY_DYN OR BUILD_LINKING   STREQUAL "SHARED")
    set(TARGETS_LINKED  ${SHARED_TARGETS})
    set(LINKED_LIBS     ${SHARED_LINKING})
  endif()

  add_executable(${OPT_NAME} ${OPT_SRCS})
  target_compile_options(${OPT_NAME} PRIVATE ${OPT_FLAGS} ${MALLOC_FLAGS})
  target_link_libraries(${OPT_NAME} ${LINKED_LIBS} ${TARGETS_LINKED})
  target_link_options(${OPT_NAME} PRIVATE ${OPT_FLAGS})

  if(NOT INSTALL_TARGETS OR ${OPT_NAME} IN_LIST INSTALL_TARGETS)
    install(TARGETS ${OPT_NAME} RUNTIME DESTINATION bin)
  endif()
endfunction()
# END ADD_EXEC_TARGET
# -------------------------------




##### ADD_TEST_TARGET
# -------------------------------
  # ADD_TEST_TARGET(
  #   NAME          test-name
  #   SRCS          sourceToCompile
  #   TARGETS       targets
  #   STATIC        static linking
  #   SHARED        shared linking
  #   FLAGS         compiler flags
  #   ENV           environ
  #   EXEC_DEPS     test's dependency on files/targets
  #   PRE_CMD_TYPE  Pre-command for each built-type
  #   PRE_CMD       Pre-command
  #   POST_CMD      Post-command
  #   ARGS          test arguments
  # )
# -------------------------------

function(ADD_TEST_TARGET)
  cmake_parse_arguments(OPT "" "NAME;" "SRCS;TARGETS;STATIC;SHARED;FLAGS;EXEC_OPTS;ENV;EXEC_DEPS;PRE_CMD_TYPE;PRE_CMD;POST_CMD;ARGS" ${ARGN})


  if(OPT_PRE_CMD)
    add_test(Test-Env-Pre-${OPT_NAME} ${OPT_PRE_CMD})
  endif ()

  set(STATIC_LINKING ${OPT_STATIC})
  set(STATIC_TARGETS )
  set(SHARED_LINKING ${OPT_SHARED})
  set(SHARED_TARGETS )
  GET_TARGET_LINKS(TARGETS ${OPT_TARGETS} STATIC ${STATIC_LINKING} SHARED ${SHARED_LINKING})

  set(STATIC_LINKING  ${MALLOC_LIBRARIES_STATIC} ${STATIC_LINKING})
  set(SHARED_LINKING  ${MALLOC_LIBRARIES_SHARED} ${SHARED_LINKING})

  if(NOT TEST_LINKING OR TEST_LINKING STREQUAL "DUAL" OR TEST_LINKING STREQUAL "STATIC")
    if(OPT_PRE_CMD_TYPE)
      add_test(Test-Static-Env-Pre-${OPT_NAME} ${OPT_PRE_CMD_TYPE})
    endif ()

    add_executable(test-${OPT_NAME}-static ${OPT_SRCS} ${OPT_EXEC_DEPS})
    target_compile_options(test-${OPT_NAME}-static PRIVATE ${OPT_FLAGS} ${MALLOC_FLAGS})
    target_link_libraries(test-${OPT_NAME}-static ${STATIC_LINKING} ${STATIC_TARGETS})
    target_link_options(test-${OPT_NAME}-static PRIVATE ${OPT_FLAGS})

    if(OPT_EXEC_OPTS)
      foreach(exec_opt ${OPT_EXEC_OPTS})
        if(OPT_ENV)
          add_test(${OPT_NAME}-wStatic-${exec_opt} env ${OPT_ENV}test-${OPT_NAME}-static ${exec_opt} ${OPT_ARGS})
        else ()
          add_test(${OPT_NAME}-wStatic-${exec_opt} test-${OPT_NAME}-static ${exec_opt} ${OPT_ARGS})
        endif ()
      endforeach()
    else()
      if(OPT_ENV)
        add_test(${OPT_NAME}-wStatic env ${OPT_ENV}test-${OPT_NAME}-static ${OPT_ARGS})
      else ()
        add_test(${OPT_NAME}-wStatic test-${OPT_NAME}-static ${OPT_ARGS})
      endif ()
    endif ()

  endif ()

  if(NOT TEST_LINKING OR TEST_LINKING STREQUAL "DUAL" OR TEST_LINKING STREQUAL "SHARED")
    if(OPT_PRE_CMD_TYPE)
      add_test(Test-Shared-Env-Pre-${OPT_NAME} ${OPT_PRE_CMD_TYPE})
    endif ()

    add_executable(test-${OPT_NAME}-shared ${OPT_SRCS} ${OPT_EXEC_DEPS})
    target_compile_options(test-${OPT_NAME}-shared PRIVATE ${OPT_FLAGS} ${MALLOC_FLAGS})
    target_link_libraries(test-${OPT_NAME}-shared ${SHARED_LINKING} ${SHARED_TARGETS})
    target_link_options(test-${OPT_NAME}-shared PRIVATE ${OPT_FLAGS})

    if(OPT_EXEC_OPTS)
      foreach(exec_opt ${OPT_EXEC_OPTS})
        if(OPT_ENV)
          add_test(${OPT_NAME}-wShared-${exec_opt} env ${OPT_ENV}test-${OPT_NAME}-shared ${exec_opt} ${OPT_ARGS})
        else ()
          add_test(${OPT_NAME}-wShared-${exec_opt} test-${OPT_NAME}-shared ${exec_opt} ${OPT_ARGS})
        endif ()
      endforeach()
    else()
      if(OPT_ENV)
        add_test(${OPT_NAME}-wShared env ${OPT_ENV}test-${OPT_NAME}-shared ${OPT_ARGS})
      else ()
        add_test(${OPT_NAME}-wShared test-${OPT_NAME}-shared ${OPT_ARGS})
      endif ()
    endif ()

  endif ()


  if(OPT_POST_CMD)
    add_test(Test-Env-Post-${OPT_NAME} ${OPT_POST_CMD})
  endif ()

endfunction()
# END ADD_TEST_TARGET
# -------------------------------




##### ADD_TEST_EXEC
# -------------------------------
  # ADD_TEST_EXEC(
  #   util_name
  #   SRCS       sourcesToCompile
  #   TARGETS    targets
  #   STATIC     static linking
  #   SHARED     shared linking
  #   FLAGS      compiler flags
  #   ONLY_DYN  link only shared
  # )
# -------------------------------

function(ADD_TEST_EXEC)
cmake_parse_arguments(OPT "" "NAME" "SRCS;TARGETS;STATIC;SHARED;FLAGS;ONLY_DYN" ${ARGN})


set(STATIC_LINKING ${OPT_STATIC})
set(STATIC_TARGETS )
set(SHARED_LINKING ${OPT_SHARED})
set(SHARED_TARGETS )
GET_TARGET_LINKS(TARGETS ${OPT_TARGETS} STATIC ${STATIC_LINKING} SHARED ${SHARED_LINKING} WITH_MALLOC TRUE)


if(NOT OPT_ONLY_DYN AND UTILS_LINKING       STREQUAL "STATIC")
  set(TARGETS_LINKED  ${STATIC_TARGETS})
  set(LINKED_LIBS     ${STATIC_LINKING})

elseif(OPT_ONLY_DYN OR UTILS_LINKING  STREQUAL "SHARED")
  set(TARGETS_LINKED  ${SHARED_TARGETS})
  set(LINKED_LIBS     ${SHARED_LINKING})
endif()


add_executable(${OPT_NAME} ${OPT_SRCS})
target_compile_options(${OPT_NAME} PRIVATE ${OPT_FLAGS} ${MALLOC_FLAGS})
target_link_libraries(${OPT_NAME} ${TARGETS_LINKED} ${LINKED_LIBS})
target_link_options(${OPT_NAME} PRIVATE ${OPT_FLAGS})

endfunction()
# END ADD_TEST_EXEC
# -------------------------------

