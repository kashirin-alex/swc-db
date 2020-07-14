#
# Copyright Since 2019 SWC-DB© [author: Kashirin Alex kashirin.alex@gmail.com]
# License details at <https://github.com/kashirin-alex/swc-db/#license>



set(java_home)
if(JAVA_INSTALL_PATH)
  set(java_home ${JAVA_INSTALL_PATH})
else()
  set(java_home $ENV{JAVA_HOME})
endif()

SET_DEPS(
  NAME "JAVA"
  LIB_PATHS ${java_home}/jre/lib/amd64
            ${java_home}/jre/lib/amd64/server  
            ${java_home}/lib/server
            ${java_home}/lib
  INC_PATHS ${java_home}/include
  # STATIC libjvm.a libjava.a libverify.a # libjawt.a 
  SHARED    jvm java verify # jawt(requires Xrender, Xtst, Xi)
  INCLUDE   jni.h
)



exec_program(env ARGS java --version OUTPUT_VARIABLE JAVARE_VERSION
             RETURN_VALUE Java_RETURN)
if (Java_RETURN STREQUAL "0")
  set(JAVARE_FOUND TRUE)
  string(REPLACE "\n" ";" JAVARE_VERSION ${JAVARE_VERSION})
  list(GET JAVARE_VERSION 0 JAVARE_VERSION)

  message(STATUS "Found Java Runtime Enviroment:")
  message("       ${JAVARE_VERSION}")

else()
  set(JAVARE_VERSION )
  set(JAVARE_FOUND FALSE)
  message(STATUS "Not Found Java Runtime Enviroment")
endif()
   

exec_program(env ARGS javac --version OUTPUT_VARIABLE JAVAC_VERSION
             RETURN_VALUE Java_RETURN)
if (Java_RETURN STREQUAL "0")
  set(JAVAC_FOUND TRUE)
  string(REPLACE "\n" ";" JAVAC_VERSION ${JAVAC_VERSION})
  list(GET JAVAC_VERSION 0 JAVAC_VERSION)

  message(STATUS "Found Java Compiler:")
  message("       ${JAVAC_VERSION}")

else()
  set(JAVAC_VERSION )
  set(JAVAC_FOUND FALSE)
  message(STATUS "Not Found Java Compiler")
endif()
   

exec_program(env ARGS mvn --version OUTPUT_VARIABLE MAVEN_VERSION
             RETURN_VALUE Java_RETURN)
if(MAVEN_VERSION MATCHES "^Apache Maven") 
  set(MAVEN_FOUND TRUE)
  string(REPLACE "\n" ";" MAVEN_VERSION ${MAVEN_VERSION})
  list(GET MAVEN_VERSION 0 MAVEN_VERSION)

  message(STATUS "Found Maven:")
  message("       ${MAVEN_VERSION}")

else()
  set(MAVEN_VERSION )
  set(MAVEN_FOUND FALSE)
  message(STATUS "Not Found Maven")
endif()
