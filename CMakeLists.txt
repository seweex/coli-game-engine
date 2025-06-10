cmake_minimum_required(VERSION 3.18)

project(coli-game-engine VERSION 1.0.0 LANGUAGES C CXX)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(LIBS_DIR    ${CMAKE_SOURCE_DIR}/libs)



file(GLOB_RECURSE HEADERS ${INCLUDE_DIR}/**/*.hxx)
file(GLOB_RECURSE SOURCES ${SOURCE_DIR}/*.cpp)

add_library (glad STATIC ${LIBS_DIR}/glad/glad.c)
set_target_properties (glad PROPERTIES
    LANGUAGE 		    C
    C_STANDARD          17
    C_STANDARD_REQUIRED YES
)

add_library (${PROJECT_NAME} INTERFACE)
set_target_properties (${PROJECT_NAME} PROPERTIES
    LANGUAGE 		      CXX
    CXX_STANDARD 	      23
    CXX_STANDARD_REQUIRED YES
)
target_include_directories (${PROJECT_NAME}
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

install(
    DIRECTORY include/ DESTINATION include
    FILES_MATCHING PATTERN "*.hxx" PATTERN "*.h"
)
install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}Targets
    INCLUDES DESTINATION include
)

find_package (glm	        CONFIG REQUIRED)
find_package (glfw3 	    CONFIG REQUIRED)
find_package (nlohmann_json CONFIG REQUIRED)
find_package (tinyobjloader CONFIG REQUIRED)
 
target_include_directories (glad            PRIVATE ${LIBS_DIR}/glad)
target_include_directories (${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})

target_link_libraries (${PROJECT_NAME} PRIVATE
	tinyobjloader::tinyobjloader
	nlohmann_json::nlohmann_json
	glm::glm-header-only
	glad
	glfw
)

source_group (Source TREE ${CMAKE_SOURCE_DIR})