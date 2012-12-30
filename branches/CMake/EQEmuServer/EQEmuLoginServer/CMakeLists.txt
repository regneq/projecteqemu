CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(eqlogin_sources
    Client.cpp
    ClientManager.cpp
    Config.cpp
    DatabaseMySQL.cpp
    DatabasePostgreSQL.cpp
    ErrorLog.cpp
    Main.cpp
    ServerManager.cpp
    WorldServer.cpp
)

IF(MSVC OR MINGW)
    SET(eqlogin_sources ${eqlogin_sources} Encryption.cpp)
ENDIF(MSVC OR MINGW)

SET(eqlogin_headers
    Client.h
    ClientManager.h
    Config.h
    Database.h
    DatabaseMySQL.h
    DatabasePostgreSQL.h
    Encryption.h
    EQCryptoAPI.h
    ErrorLog.h
    LoginServer.h
    LoginStructures.h
    Options.h
    ServerManager.h
    WorldServer.h
)

ADD_EXECUTABLE(LoginServer ${eqlogin_sources} ${eqlogin_headers})

TARGET_LINK_LIBRARIES(LoginServer Common debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE})

IF(MSVC)
	ADD_DEFINITIONS(-D_CONSOLE)
    SET_TARGET_PROPERTIES(LoginServer PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
    TARGET_LINK_LIBRARIES(LoginServer "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
    TARGET_LINK_LIBRARIES(LoginServer "WS2_32")
ENDIF(MINGW)

IF(UNIX)
    SET(EQEMU_UNIX_ENC_LIBRARY_LOC "${CMAKE_SOURCE_DIR}/EQEmuLoginServer" CACHE PATH "Location of EQEmuAuthCrypto and cryptopp")
    LINK_DIRECTORIES(${EQEMU_UNIX_ENC_LIBRARY_LOC})
    TARGET_LINK_LIBRARIES(LoginServer "dl")
    TARGET_LINK_LIBRARIES(LoginServer "z")
    TARGET_LINK_LIBRARIES(LoginServer "m")
    TARGET_LINK_LIBRARIES(LoginServer "rt")
    TARGET_LINK_LIBRARIES(LoginServer "pthread")
    TARGET_LINK_LIBRARIES(LoginServer "ssl")
    TARGET_LINK_LIBRARIES(LoginServer "crypto")
    TARGET_LINK_LIBRARIES(LoginServer "EQEmuAuthCrypto")
    TARGET_LINK_LIBRARIES(LoginServer "cryptopp")
    ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(EXECUTABLE_OUTPUT_PATH ../Bin)
