cmake_minimum_required(VERSION 3.20)

project(coli VERSION 0.1.0 LANGUAGES C CXX)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(LIBS_DIR    ${CMAKE_SOURCE_DIR}/libs)
set(SOURCE_DIR  ${CMAKE_SOURCE_DIR}/src)

add_executable(coli_test ${SOURCE_DIR}/Main.cpp)
target_include_directories(coli_test PRIVATE ${INCLUDE_DIR})

set_property(TARGET coli_test PROPERTY CXX_STANDARD 23)
set_property(TARGET coli_test PROPERTY CXX_STANDARD_REQUIRED ON)

#source_group(Graphics FILES ${INCLUDE_DIR}/Graphics/Buffer.hxx
#							${INCLUDE_DIR}/Graphics/Shader.hxx)

# glad
add_library(glad STATIC ${LIBS_DIR}/glad/glad.c)
target_include_directories(glad PRIVATE ${LIBS_DIR}/glad)
target_link_libraries(coli_test PRIVATE glad)
target_include_directories(coli_test PRIVATE ${LIBS_DIR}/glad)

# glm
find_package(glm CONFIG REQUIRED)
target_link_libraries(coli_test PRIVATE glm::glm-header-only)

# glfw
find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(coli_test PRIVATE glfw)

# json
find_package(nlohmann_json 3.11.3 REQUIRED)
target_link_libraries(coli_test PRIVATE nlohmann_json::nlohmann_json)