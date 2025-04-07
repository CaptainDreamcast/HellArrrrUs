cmake_minimum_required(VERSION 3.25) 
project(WelcomeHell CXX)               
set(CMAKE_CXX_STANDARD 17)            
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

include("${CMAKE_SOURCE_DIR}/../../addons/prism/cmake/prism.cmake" REQUIRED)

# Add source/header files to project
file(GLOB_RECURSE SOURCES ../*.cpp SOURCES ../*.h SOURCES ../*.rc)
list(FILTER SOURCES EXCLUDE REGEX ".*web.*")
list(FILTER SOURCES EXCLUDE REGEX ".*/build/.*")

add_link_options(/NODEFAULTLIB:libcmt.lib)
add_link_options(/IGNORE:4099,4286,4098)
add_link_options(/OPT:REF)
add_link_options(/OPT:ICF)

# Define exe
add_executable(WelcomeHell ${SOURCES})

# Link with libraries
target_link_libraries(WelcomeHell prism)
target_link_libraries(WelcomeHell SDL2)
target_link_libraries(WelcomeHell SDL2main)
target_link_libraries(WelcomeHell SDL2_image)
target_link_libraries(WelcomeHell SDL2_mixer)
target_link_libraries(WelcomeHell SDL2_ttf)
target_link_libraries(WelcomeHell enet)
target_link_libraries(WelcomeHell freetype)
target_link_libraries(WelcomeHell glew)
target_link_libraries(WelcomeHell curl)
target_link_libraries(WelcomeHell libjpeg)
target_link_libraries(WelcomeHell libpng)
target_link_libraries(WelcomeHell libwebp)
target_link_libraries(WelcomeHell zstd)
target_link_libraries(WelcomeHell zlib)
target_link_libraries(WelcomeHell crypt32)
target_link_libraries(WelcomeHell wldap32)
target_link_libraries(WelcomeHell ws2_32)
target_link_libraries(WelcomeHell winmm)
target_link_libraries(WelcomeHell OpenGL32)
target_link_libraries(WelcomeHell version)

# Define preprocessor definitions
target_compile_definitions(WelcomeHell PUBLIC UNICODE)
target_compile_definitions(WelcomeHell PUBLIC _UNICODE)

set_property(TARGET WelcomeHell PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
target_compile_options(WelcomeHell PRIVATE /Gy)

set_target_properties(WelcomeHell PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../assets)
set_target_properties(WelcomeHell PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/../assets)

# Copy over DLLs
file(GLOB DLLS "${CMAKE_SOURCE_DIR}/../../addons/prism/windows/vs17/DLL/*.dll")
foreach(DLL ${DLLS})
  add_custom_command(TARGET WelcomeHell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${DLL}
    $<TARGET_FILE_DIR:WelcomeHell>)
endforeach()

# Optimize for release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  target_compile_options(WelcomeHell PRIVATE /O2)
endif()
