
### GLFW ###

add_subdirectory(glfw-3.3.6)

include_directories(
    glfw-3.3.6/include/GLFW/
    glew-2.1.0/include/
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(OPENGL_LIBRARY
    ${OPENGL_LIBRARY}
    -lGL -lGLU -lXrandr -lXext -lX11 -lrt
    ${CMAKE_DL_LIBS}
    ${GLFW_LIBRARIES}
)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(OPENGL_LIBRARY
    ${OPENGL_LIBRARY}
    ${CMAKE_DL_LIBS}
    ${GLFW_LIBRARIES}
)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

### GLEW ###

set(GLEW_SOURCE
    glew-2.1.0/src/glew.c
)

set(GLEW_HEADERS
)


add_library( GLEW STATIC
    ${GLEW_SOURCE}
    ${GLEW_INCLUDE}
)

target_link_libraries(GLEW
    ${OPENGL_LIBRARY}
    ${EXTRA_LIBS}
)

set_target_properties(GLEW
    PROPERTIES
    PROJECT_LABEL glew
    FOLDER "Libraries"
)

# SOIL
add_subdirectory(Simple-OpenGL-Image-Library)

### TINYXML2 ###

set(TINYXML2_SOURCE
    tinyxml2/src/tinyxml2.cpp
)

set(TINYXML2_HEADERS
	tinyxml2/include/tinyxml2.h
)

include_directories(tinyxml2/include/)

add_library(TINYXML2 STATIC
    ${TINYXML2_SOURCE}
    ${TINYXML2_HEADERS}
)
target_link_libraries(TINYXML2)
set_target_properties(TINYXML2
    PROPERTIES
    PROJECT_LABEL tinyxml2
    FOLDER "Libraries"
)
