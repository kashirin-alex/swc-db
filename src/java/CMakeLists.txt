#
# SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
# License details at <https://github.com/kashirin-alex/swc-db/#license>




if(NOT SWC_WITHOUT_JAVA AND THRIFT_JAVA_GEN_PATH AND MAVEN_FOUND)
  execute_process(COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/mvn  ${CMAKE_CURRENT_BINARY_DIR}/)

	set(MODULES_PLACE_HOLDER "")
	set(MODULES_PLACE_HOLDER "${MODULES_PLACE_HOLDER} \n <module>thrift</module>")
	set(MODULES_PLACE_HOLDER "${MODULES_PLACE_HOLDER} \n <module>jdbc</module>")

	if(JAVARE_FOUND)
		set(MODULES_PLACE_HOLDER "${MODULES_PLACE_HOLDER} \n <module>thrift-tests</module>")
    set_property(GLOBAL PROPERTY THRIFT_JAVA_TEST
			${CMAKE_CURRENT_BINARY_DIR}/mvn/swcdb/thrift-tests/target/thrift-tests-${VERSION_STRING}-bundled.jar
		)
	endif()

	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/mvn/swcdb/pom.xml.in
		${CMAKE_CURRENT_BINARY_DIR}/mvn/swcdb/pom.xml @ONLY)
	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/mvn/swcdb/thrift/pom.xml.in
		${CMAKE_CURRENT_BINARY_DIR}/mvn/swcdb/thrift/pom.xml @ONLY)
	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/mvn/swcdb/thrift/pom_sonatype.xml
		${CMAKE_CURRENT_BINARY_DIR}/mvn/swcdb/thrift/pom_sonatype.xml @ONLY)
	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/mvn/swcdb/thrift-tests/pom.xml.in
		${CMAKE_CURRENT_BINARY_DIR}/mvn/swcdb/thrift-tests/pom.xml @ONLY)

	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/mvn/swcdb/jdbc/pom.xml.in
		${CMAKE_CURRENT_BINARY_DIR}/mvn/swcdb/jdbc/pom.xml @ONLY)
	configure_file(
		${CMAKE_CURRENT_SOURCE_DIR}/mvn/swcdb/jdbc/thrift/pom.xml.in
		${CMAKE_CURRENT_BINARY_DIR}/mvn/swcdb/jdbc/thrift/pom.xml @ONLY)


	add_custom_target(MVN_build ALL
					          mvn -f mvn/swcdb/pom.xml -Dmaven.test.skip=true package
										license:add-third-party
										)


	install(
    FILES			 	${CMAKE_CURRENT_BINARY_DIR}/mvn/swcdb/thrift/target/thrift-${SWCDB_VERSION}-v${THRIFT_VERSION}-bundled.jar
    DESTINATION lib/java/	
		RENAME 			swcdb-thrift-${SWCDB_VERSION}-v${THRIFT_VERSION}-bundled.jar
	)
	install(
    FILES			 	${CMAKE_CURRENT_BINARY_DIR}/mvn/swcdb/thrift/target/classes/THIRD-PARTY.txt
    DESTINATION share/swcdb/	
		RENAME 			THIRD-PARTY_swcdb-thrift-${SWCDB_VERSION}-v${THRIFT_VERSION}-bundled.jar.txt
	)

	install(
    FILES			 	${CMAKE_CURRENT_BINARY_DIR}/mvn/swcdb/jdbc/thrift/target/jdbc.thrift-${SWCDB_VERSION}-v${THRIFT_VERSION}-bundled.jar
    DESTINATION lib/java/	
		RENAME 			swcdb-jdbc-thrift-${SWCDB_VERSION}-v${THRIFT_VERSION}-bundled.jar
	)
	install(
    FILES			 	${CMAKE_CURRENT_BINARY_DIR}/mvn/swcdb/jdbc/thrift/target/classes/THIRD-PARTY.txt
    DESTINATION share/swcdb/	
		RENAME 			THIRD-PARTY_swcdb-jdbc-thrift-${SWCDB_VERSION}-v${THRIFT_VERSION}-bundled.jar.txt
	)

endif()

