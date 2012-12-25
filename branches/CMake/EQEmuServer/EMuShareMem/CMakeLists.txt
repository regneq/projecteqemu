CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(sharedmem_src
    DLLMain.cpp
    Doors.cpp
    Items.cpp
    Loot.cpp
    MMF.cpp
    MMFMutex.cpp
    NPCFactionLists.cpp
    Opcodes.cpp
    SkillCaps.cpp
    Spells.cpp
)

SET(sharedmem_headers
    Doors.h
    Items.h
    Loot.h
    MMF.h
    MMFMutex.h
    NPCFactionLists.h
    Opcodes.h
    SkillCaps.h
    Spells.h
)

ADD_LIBRARY(EMuShareMem SHARED ${sharedmem_src} ${sharedmem_headers})
TARGET_LINK_LIBRARIES(EMuShareMem Common)

IF(UNIX)
    TARGET_LINK_LIBRARIES(World "dl")
    TARGET_LINK_LIBRARIES(World "z")
    TARGET_LINK_LIBRARIES(World "m")
    TARGET_LINK_LIBRARIES(World "rt")
    TARGET_LINK_LIBRARIES(World "pthread")
    TARGET_LINK_LIBRARIES(World "ssl")
    TARGET_LINK_LIBRARIES(World "crypto")
    ADD_DEFINITIONS(-fPIC)
ENDIF(UNIX)

SET(LIBRARY_OUTPUT_PATH ../Bin)
