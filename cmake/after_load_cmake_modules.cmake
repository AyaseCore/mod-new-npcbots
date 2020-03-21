AC_ADD_SCRIPT_LOADER("Npcbot" "${CMAKE_MOD_NEW_NPCBOTS_DIR}/src/loader.h")

CU_ADD_HOOK(BEFORE_GAME_LIBRARY "${CMAKE_MOD_NEW_NPCBOTS_DIR}/cmake/before_game_lib.cmake")
CU_ADD_HOOK(BEFORE_SCRIPTS_LIBRARY "${CMAKE_MOD_NEW_NPCBOTS_DIR}/cmake/before_scripts_lib.cmake")

AC_ADD_CONFIG_FILE("${CMAKE_MOD_NEW_NPCBOTS_DIR}/conf/newnpcbots.conf.dist")

message("机器人模块启用.")