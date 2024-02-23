file(
 GLOB_RECURSE ASSIGNMENT3_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE ASSIGNMENT3_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this assignment3's asset folder to bin when it is built
add_custom_target(copyAssetsA3 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENT3_INC} DESTINATION include/assignment3)
add_executable(assignment3 ${ASSIGNMENT3_SRC} ${ASSIGNMENT3_INC})
target_link_libraries(assignment3 PUBLIC core IMGUI assimp)
target_include_directories(assignment3 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignment3 is built
add_dependencies(assignment3 copyAssetsA3)