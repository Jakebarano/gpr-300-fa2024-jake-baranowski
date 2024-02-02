file(
 GLOB_RECURSE ASSIGNMENT1_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE ASSIGNMENT1_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this assignment1's asset folder to bin when it is built
add_custom_target(copyAssetsA1 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENT1_INC} DESTINATION include/assignment1)
add_executable(assignment1 ${ASSIGNMENT1_SRC} ${ASSIGNMENT1_INC})
target_link_libraries(assignment1 PUBLIC core IMGUI assimp)
target_include_directories(assignment1 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignment1 is built
add_dependencies(assignment1 copyAssetsA1)