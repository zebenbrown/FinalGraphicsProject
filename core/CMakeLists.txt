file(
 GLOB_RECURSE CORE_INC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.h *.hpp
)

file(
 GLOB_RECURSE CORE_SRC CONFIGURE_DEPENDS
 RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
 *.c *.cpp
)

set(CORE_INC_DIR
 ${CMAKE_CURRENT_SOURCE_DIR}
 CACHE PATH "CORE INCLUDE SOURCE PATH"
)

add_library(core STATIC ${CORE_SRC} ${CORE_INC}
        cobb/shader.hpp
        cobb/camera.cpp
        cobb/object.cpp
        cobb/line.cpp
        cobb/texture2d.cpp)

add_custom_target(copyAssetsLine ALL COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/cobb/assets/
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/)

find_package(OpenGL REQUIRED)

target_link_libraries(core PUBLIC IMGUI glm assimp)

install (TARGETS core DESTINATION lib)
install (FILES ${CORE_INC} DESTINATION include/core)

#Trigger asset copy when testing is built
add_dependencies(core copyAssetsLine)