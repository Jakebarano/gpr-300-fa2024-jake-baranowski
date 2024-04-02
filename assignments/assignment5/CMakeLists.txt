file(
 GLOB_RECURSE ASSIGNMENT5_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE ASSIGNMENT5_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)
#Copies this assignment5's asset folder to bin when it is built
add_custom_target(copyAssetsA5 ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
${CMAKE_CURRENT_SOURCE_DIR}/assets/
${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

install(FILES ${ASSIGNMENT5_INC} DESTINATION include/assignment5)
add_executable(assignment5 ${ASSIGNMENT5_SRC} ${ASSIGNMENT5_INC})
target_link_libraries(assignment5 PUBLIC core IMGUI assimp)
target_include_directories(assignment5 PUBLIC ${CORE_INC_DIR} ${stb_INCLUDE_DIR})

#Trigger asset copy when assignment5 is built
add_dependencies(assignment5 copyAssetsA5)