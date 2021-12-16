cmake_minimum_required(VERSION 3.0)
project(Chess3D)

find_package(OpenGL REQUIRED)

# c++11, -g option is used to export debug symbols for gdb
if(${CMAKE_CXX_COMPILER_ID} MATCHES GNU OR
    ${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
  # Using C++11 on OSX requires using libc++ instead of libstd++.
  # libc++ is an implementation of the C++ standard library for OSX.
  if(APPLE)
    if(XCODE)
      set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
      set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
    else()
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")
    endif()
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
  endif()
endif()

# define library postfix so that different builds will produce
# distinguished libraries
set(CMAKE_RELEASE_POSTFIX "_r" CACHE string "Release postfix")
set(CMAKE_DEBUG_POSTFIX "_d" CACHE string "Debug postfix")
set(CMAKE_RELWITHDEBINFO_POSTFIX "_rd" CACHE string
  "Release with debug info postfix")
set(CMAKE_MINSIZEREL_POSTFIX "_mr" CACHE string
  "Minimum size release postfix")
mark_as_advanced(CMAKE_RELEASE_POSTFIX)
mark_as_advanced(CMAKE_DEBUG_POSTFIX)
mark_as_advanced(CMAKE_RELWITHDEBINFO_POSTFIX)
mark_as_advanced(CMAKE_MINSIZEREL_POSTFIX)

# for rdm (emacs)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# for grouping into folders
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(src/vendor)

include_directories(
  src/vendor/glfw-3.3.6/include/GLFW/
  src/vendor/glew-2.1.0/include/
  src/vendor/glm-0.9.9.8/
  src/vendor/Simple-OpenGL-Image-Library/include/
  src/vendor/tinyxml2/include/
  src/vendor/tinyobjloader/include/
  src/Chess3D/
  )

set(ALL_LIBS
  ${OPENGL_LIBRARY}
  glfw
  GLEW
  SOIL
  TINYXML2
  )

add_definitions(
  -DTW_STATIC
  -DTW_NO_LIB_PRAGMA
  -DTW_NO_DIRECT3D
  -DGLEW_STATIC
  -D_CRT_SECURE_NO_WARNINGS
  )

file(GLOB_RECURSE CHESS3D_SOURCE CONFIGURE_DEPENDS "src/Chess3D/*.cpp")
file(GLOB_RECURSE CHESS3D_HEADERS CONFIGURE_DEPENDS "src/Chess3D/*.h")

add_executable(Chess3D
  ${CHESS3D_HEADERS}
  ${CHESS3D_SOURCE}
  )

target_link_libraries(Chess3D
  ${ALL_LIBS}
  )

# copy assets
add_custom_command(TARGET Chess3D POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets/ ${CMAKE_CURRENT_BINARY_DIR}/assets/)
