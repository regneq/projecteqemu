CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(zone_sources
    AA.cpp
    aggro.cpp
    attack.cpp
    beacon.cpp
    bonuses.cpp
    bot.cpp
    botspellsai.cpp
    client.cpp
    client_logs.cpp
    client_mods.cpp
    client_packet.cpp
    client_process.cpp
    command.cpp
    doors.cpp
    effects.cpp
    embparser.cpp
    embperl.cpp
    embxs.cpp
    entity.cpp
    exp.cpp
    faction.cpp
    fearpath.cpp
    forage.cpp
    groups.cpp
    guild.cpp
    guild_mgr.cpp
    hate_list.cpp
    horse.cpp
    inventory.cpp
    loottables.cpp
    Map.cpp
    mob.cpp
    MobAI.cpp
    net.cpp
    npc.cpp
    NpcAI.cpp
    Object.cpp
    parser.cpp
    pathing.cpp
    perl_client.cpp
    perl_doors.cpp
    perl_entity.cpp
    perl_groups.cpp
    perl_hateentry.cpp
    perl_mob.cpp
    perl_npc.cpp
    perl_object.cpp
    perl_perlpacket.cpp
    perl_PlayerCorpse.cpp
    perl_questitem.cpp
    perl_raids.cpp
    perlpacket.cpp
    perlparser.cpp
    petitions.cpp
    pets.cpp
    PlayerCorpse.cpp
    QGlobals.cpp
    questmgr.cpp
    QuestParserCollection.cpp
    raids.cpp
    spawn2.cpp
    spawn2.h
    spawngroup.cpp
    spdat.cpp
    special_attacks.cpp
    spell_effects.cpp
    spells.cpp
    tasks.cpp
    titles.cpp
    tradeskills.cpp
    trading.cpp
    trap.cpp
    tribute.cpp
    updatemgr.cpp
    watermap.cpp
    waypoints.cpp
    worldserver.cpp
    zone.cpp
    zone_logsys.cpp
    zone_profile.cpp
    ZoneConfig.cpp
    zonedb.cpp
    zonedbasync.cpp
    zoning.cpp
)

SET(zone_headers
    AA.h
    basic_functions.h
    beacon.h
    blah.h
    bot.h
    botStructs.h
    client.h
    client_logs.h
    client_packet.h
    command.h
    doors.h
    embparser.h
    embperl.h
    embxs.h
    entity.h
    errmsg.h
    event_codes.h
    faction.h
    features.h
    forage.h
    groups.h
    guild_mgr.h
    hate_list.h
    horse.h
    loottable.h
    map.h
    masterentity.h
    maxskill.h
    message.h
    mob.h
    net.h
    npc.h
    NpcAI.h
    object.h
    parser.h
    pathing.h
    perlpacket.h
    perlparser.h
    petitions.h
    pets.h
    PlayerCorpse.h
    QGlobals.h
    QuestInterface.h
    questmgr.h
    QuestParserCollection.h
    raid.h
    raids.h
    skills.h
    spawn2.cpp
    spawn2.h
    spawngroup.h
    spdat.h
    StringIDs.h
    tasks.h
    titles.h
    trap.h
    updatemgr.h
    watermap.h
    worldserver.h
    zone.h
    zone_profile.h
    ZoneConfig.h
    zonedb.h
    zonedbasync.h
    zonedump.h
)

OPTION(ENABLE_BOTS "Enable Bots" OFF)

ADD_EXECUTABLE(Zone ${zone_sources} ${zone_headers})

ADD_DEFINITIONS(-DZONE)
IF(ENABLE_BOTS)
    ADD_DEFINITIONS(-DBOTS)
ENDIF(ENABLE_BOTS)

IF(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${perl_ccopts} ${perl_ldopts} ${mysql_ccopts} ${mysql_ldopts}")
ENDIF(UNIX)

TARGET_LINK_LIBRARIES(Zone Common ${PERL_LIBRARY} debug ${MySQL_LIBRARY_DEBUG} optimized ${MySQL_LIBRARY_RELEASE})

IF(MSVC)
	ADD_DEFINITIONS(-D_CONSOLE)
    SET_TARGET_PROPERTIES(Zone PROPERTIES LINK_FLAGS_RELEASE "/OPT:REF /OPT:ICF")
    TARGET_LINK_LIBRARIES(Zone "Ws2_32.lib")
ENDIF(MSVC)

IF(MINGW)
    TARGET_LINK_LIBRARIES(Client "WS2_32")
ENDIF(MINGW)

SET(EXECUTABLE_OUTPUT_PATH ../Bin)
