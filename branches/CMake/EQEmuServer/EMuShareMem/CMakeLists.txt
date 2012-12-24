CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(sharedmem_src
    DLLMain.cpp
    Doors.cpp
    Items.cpp
    Loot.cpp
    MMF.cpp
    MMFMutex.cpp
    NPCFactionLists.cpp
    NPCTypes.cpp
    Opcodes.cpp
    Skillcaps.cpp
    Spells.cpp
)

SET(sharedmem_headers
    Doors.h
    Items.h
    Loot.h
    MMF.h
    MMFMutex.h
    NPCFactionLists.h
    NPCTypes.h
    Opcodes.h
    Skillcaps.h
    Spells.h
)

ADD_LIBRARY(EMuShareMem SHARED ${sharedmem_src} ${sharedmem_headers})
TARGET_LINK_LIBRARIES(EMuShareMem Common)

SET(LIBRARY_OUTPUT_PATH ../Bin)
