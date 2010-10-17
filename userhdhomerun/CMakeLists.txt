PROJECT(userhdhomerun)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
IF(COMMAND cmake_policy)
   CMAKE_policy(SET CMP0003 NEW)
ENDIF(COMMAND cmake_policy)

SET(LIBHDHOMERUN_PATH /usr/lib/libhdhomerun)

SET(QT_USE_QTXML 1)
FIND_PACKAGE(Qt4 COMPONENTS QtGui REQUIRED)
include(${QT_USE_FILE})

SET(CMAKE_BUILD_TYPE Debug)

SET(userhdhomerun_HDRS
  hdhomerun_control.h
  hdhomerun_controller.h
  hdhomerun_tuner.h
)

SET(userhdhomerun_SRCS
  hdhomerun_control.cpp
  hdhomerun_controller.cpp
  hdhomerun_tuner.cpp
)

INCLUDE_DIRECTORIES(
	${CMAKE_CURRENT_BINARY_DIR} 
	${QT_INCLUDES}
	${LIBHDHOMERUN_PATH}
)

LINK_DIRECTORIES(
	${LIBHDHOMERUN_PATH}
)

ADD_EXECUTABLE(userhdhomerun main.cpp ${userhdhomerun_SRCS})
TARGET_LINK_LIBRARIES(userhdhomerun
	${QT_LIBRARIES} 
	hdhomerun
)


INSTALL(TARGETS userhdhomerun
  RUNTIME DESTINATION bin
)
