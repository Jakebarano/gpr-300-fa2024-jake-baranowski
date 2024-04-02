file(
 GLOB_RECURSE ASSIGNMENT4_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE ASSIGNMENT4_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this assignment4's asset folder to bin when it is built
add_custom_target(copyAssetsA4 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENT4_INC} DESTINATION include/assignment4)
add_executable(assignment4 ${ASSIGNMENT4_SRC} ${ASSIGNMENT4_INC})
target_link_libraries(assignment4 PUBLIC core IMGUI assimp)
target_include_directories(assignment4 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignment4 is built
add_dependencies(assignment4 copyAssetsA4)