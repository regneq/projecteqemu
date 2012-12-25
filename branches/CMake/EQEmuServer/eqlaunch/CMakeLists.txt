CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(eqlaunch_sources
    eqlaunch.cpp
    worldserver.cpp
    ZoneLaunch.cpp
)

SET(eqlaunch_headers
    worldserver.h
    ZoneLaunch.h
)

ADD_EXECUTABLE(EQLaunch ${eqlaunch_sources} ${eqlaunch_headers})

TARGET_LINK_LIBRARIES(EQLaunch Common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE})

IF(MSVC)
	ADD_DEFINITIONS(-D_CONSOLE)
    SET_TARGET_PROPERTIES(EQLaunch PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
    TARGET_LINK_LIBRARIES(EQLaunch "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
    TARGET_LINK_LIBRARIES(EQLaunch "WS2_32")
ENDIF(MINGW)

IF(UNIX)
    TARGET_LINK_LIBRARIES(EQLaunch "dl")
    TARGET_LINK_LIBRARIES(EQLaunch "z")
    TARGET_LINK_LIBRARIES(EQLaunch "m")
    TARGET_LINK_LIBRARIES(EQLaunch "rt")
    TARGET_LINK_LIBRARIES(EQLaunch "pthread")
    TARGET_LINK_LIBRARIES(EQLaunch "ssl")
    TARGET_LINK_LIBRARIES(EQLaunch "crypto")
    ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ../Bin)
