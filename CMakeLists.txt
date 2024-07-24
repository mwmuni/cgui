cmake_minimum_required(VERSION 3.15)
project(compressor C)

find_package(glad REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glu REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(tinycthread REQUIRED)

add_executable(${PROJECT_NAME} src/main.c)
target_link_libraries(${PROJECT_NAME} glad::glad)
target_link_libraries(${PROJECT_NAME} OpenGL::GL)
target_link_libraries(${PROJECT_NAME} GLEW::GLEW)
target_link_libraries(${PROJECT_NAME} glu::glu)
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} glm::glm)
target_link_libraries(${PROJECT_NAME} tinycthread::tinycthread)

if (WIN32)
    # Set the subsystem to Windows to prevent the console window from appearing
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE YES
    )
endif()