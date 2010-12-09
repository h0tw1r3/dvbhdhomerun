PROJECT(userhdhomerun)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND cmake_policy)
   CMAKE_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(LIBHDHOMERUN_PATH /usr/lib/libhdhomerun)

SET(CMAKE_BUILD_TYPE Debug)

SET(userhdhomerun_HDRS
  conf_inifile.h
  hdhomerun_control.h
  hdhomerun_controller.h
  hdhomerun_tuner.h
  thread_pthread.h
)

SET(userhdhomerun_SRCS
  conf_inifile.cpp
  hdhomerun_control.cpp
  hdhomerun_controller.cpp
  hdhomerun_tuner.cpp
  thread_pthread.cpp
)

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
)


INSTALL(TARGETS userhdhomerun
  RUNTIME DESTINATION bin
)
