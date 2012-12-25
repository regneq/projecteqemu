CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(ucs_sources
    chatchannel.cpp
    clientlist.cpp
    database.cpp
    ucs.cpp
    ucsconfig.cpp
    worldserver.cpp
)

SET(ucs_headers
    chatchannel.h
    clientlist.h
    database.h
    ucsconfig.h
    worldserver.h
)

ADD_EXECUTABLE(UCS ${ucs_sources} ${ucs_headers})

ADD_DEFINITIONS(-DCHAT)

TARGET_LINK_LIBRARIES(UCS Common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE})

IF(MSVC)
	ADD_DEFINITIONS(-D_CONSOLE)
    SET_TARGET_PROPERTIES(UCS PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
    TARGET_LINK_LIBRARIES(UCS "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
    TARGET_LINK_LIBRARIES(UCS "WS2_32")
ENDIF(MINGW)

IF(UNIX)
    TARGET_LINK_LIBRARIES(UCS "dl")
    TARGET_LINK_LIBRARIES(UCS "z")
    TARGET_LINK_LIBRARIES(UCS "m")
    TARGET_LINK_LIBRARIES(UCS "rt")
    TARGET_LINK_LIBRARIES(UCS "pthread")
    TARGET_LINK_LIBRARIES(UCS "ssl")
    TARGET_LINK_LIBRARIES(UCS "crypto")
    ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ../Bin)
