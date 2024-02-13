file(
 GLOB_RECURSE ASSIGNMENT2_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE ASSIGNMENT2_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this assignment2's asset folder to bin when it is built
add_custom_target(copyAssetsA2 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENT2_INC} DESTINATION include/assignment2)
add_executable(assignment2 ${ASSIGNMENT2_SRC} ${ASSIGNMENT2_INC})
target_link_libraries(assignment2 PUBLIC core IMGUI assimp)
target_include_directories(assignment2 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignment2 is built
add_dependencies(assignment2 copyAssetsA2)