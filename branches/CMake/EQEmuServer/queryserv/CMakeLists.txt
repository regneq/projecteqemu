CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(qserv_sources
    database.cpp
    lfguild.cpp
    queryserv.cpp
    queryservconfig.cpp
    worldserver.cpp
)

SET(qserv_headers
    database.h
    lfguild.h
    queryservconfig.h
    worldserver.h
)

ADD_EXECUTABLE(QueryServ ${qserv_sources} ${qserv_headers})

ADD_DEFINITIONS(-DQSERV)

TARGET_LINK_LIBRARIES(QueryServ Common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE})

IF(MSVC)
	ADD_DEFINITIONS(-D_CONSOLE)
    SET_TARGET_PROPERTIES(QueryServ PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
    TARGET_LINK_LIBRARIES(QueryServ "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
    TARGET_LINK_LIBRARIES(QueryServ "WS2_32")
ENDIF(MINGW)

IF(UNIX)
    TARGET_LINK_LIBRARIES(QueryServ "dl")
    TARGET_LINK_LIBRARIES(QueryServ "z")
    TARGET_LINK_LIBRARIES(QueryServ "m")
    TARGET_LINK_LIBRARIES(QueryServ "rt")
    TARGET_LINK_LIBRARIES(QueryServ "pthread")
    TARGET_LINK_LIBRARIES(QueryServ "ssl")
    TARGET_LINK_LIBRARIES(QueryServ "crypto")
    ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ../Bin)
