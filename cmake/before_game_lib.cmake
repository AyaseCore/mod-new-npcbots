CollectSourceFiles(
  ${CMAKE_MOD_NEW_NPCBOTS_DIR}/src/npcbots/
  PRIVATE_SOURCES)
  
file(GLOB_RECURSE sources ${CMAKE_MOD_NEW_NPCBOTS_DIR}/src/npcbots/*)
source_group("AI\\Npcbots" FILES ${sources})

add_definitions(-DNPCBOT)

CollectIncludeDirectories(
  ${CMAKE_MOD_NEW_NPCBOTS_DIR}/src/npcbots/
  PUBLIC_INCLUDES)

