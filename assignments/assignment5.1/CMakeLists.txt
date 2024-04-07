file(
 GLOB_RECURSE ASSIGNMENT5.1_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE ASSIGNMENT5.1_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this assignment5.1's asset folder to bin when it is built
add_custom_target(copyAssetsA5.1 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENT5.1_INC} DESTINATION include/assignment5.1)
add_executable(assignment5.1 ${ASSIGNMENT5.1_SRC} ${ASSIGNMENT5.1_INC})
target_link_libraries(assignment5.1 PUBLIC core IMGUI assimp)
target_include_directories(assignment5.1 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignment5.1 is built
add_dependencies(assignment5.1 copyAssetsA5.1)