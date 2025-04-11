cmake_minimum_required(VERSION 3.18)

project(coli VERSION 0.2.0 LANGUAGES C CXX)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(LIBS_DIR    ${CMAKE_SOURCE_DIR}/libs)
set(SOURCE_DIR  ${CMAKE_SOURCE_DIR}/src)

file(GLOB_RECURSE HEADERS ${INCLUDE_DIR}/**/*.hxx)
file(GLOB_RECURSE SOURCES ${SOURCE_DIR}/*.cpp)

add_executable (coli_test   ${HEADERS} ${SOURCES})
add_library    (glad STATIC ${LIBS_DIR}/glad/glad.c)

set_target_properties (coli_test PROPERTIES
    LANGUAGE 		  CXX
    CXX_STANDARD 	  23
    CXX_STANDARD_REQUIRED YES
)

set_target_properties (glad PROPERTIES
    LANGUAGE 		C
    C_STANDARD          17
    C_STANDARD_REQUIRED YES
)

find_package (glm	    CONFIG REQUIRED)
find_package (glfw3 	    CONFIG REQUIRED)
find_package (nlohmann_json CONFIG REQUIRED)

target_include_directories (glad      PUBLIC  ${LIBS_DIR}/glad)
target_include_directories (coli_test PRIVATE ${INCLUDE_DIR})

target_link_libraries (coli_test PRIVATE
	nlohmann_json::nlohmann_json
	glm::glm-header-only
	glad
	glfw
)

source_group (Source TREE ${CMAKE_SOURCE_DIR})
