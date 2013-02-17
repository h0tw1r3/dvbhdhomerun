PROJECT(userhdhomerun)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND cmake_policy)
   CMAKE_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(LIBHDHOMERUN_PATH /usr/lib/libhdhomerun)

SET(CMAKE_BUILD_TYPE Release)
#SET(CMAKE_CXX_FLAGS_PROFILE "-pg")

SET(userhdhomerun_HDRS
  conf_inifile.h
  hdhomerun_control.h
  hdhomerun_controller.h
  hdhomerun_tuner.h
  log_file.h
  thread_pthread.h
)

SET(userhdhomerun_SRCS
  conf_inifile.cpp
  hdhomerun_control.cpp
  hdhomerun_controller.cpp
  hdhomerun_tuner.cpp
  log_file.cpp
  thread_pthread.cpp
)

INCLUDE(CheckStructHasMember)
CHECK_STRUCT_HAS_MEMBER("struct hdhomerun_discover_device_t" tuner_count ${LIBHDHOMERUN_PATH}/hdhomerun.h HAVE_HDHOMERUN_TUNER_COUNT)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_BINARY_DIR} 
	${LIBHDHOMERUN_PATH}
)

LINK_DIRECTORIES(
	${LIBHDHOMERUN_PATH}
)

ADD_EXECUTABLE(userhdhomerun main.cpp ${userhdhomerun_SRCS})
TARGET_LINK_LIBRARIES(userhdhomerun
	hdhomerun
	pthread
)

ADD_CUSTOM_TARGET(cppcheck
  COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} \; cppcheck --enable=all main.cpp ${userhdhomerun_SRCS}
)

INSTALL(TARGETS userhdhomerun
  RUNTIME DESTINATION bin
)
