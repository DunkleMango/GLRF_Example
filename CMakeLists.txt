cmake_minimum_required (VERSION 3.8)

project("glrf_app" VERSION 1.0 LANGUAGES C CXX)

# Add source to this project's executable.
add_executable(glrf_app "App.cpp" "App.hpp")

# TODO: Add tests and install targets if needed.
target_include_directories(glrf_app PRIVATE "${PROJECT_SOURCE_DIR}/external/glad/include")
target_include_directories(glrf_app PRIVATE ${GLAD_HEADERS})
target_include_directories(glrf_app PRIVATE ${GLAD_SOURCES})
target_link_libraries(glrf_app glfw)
target_link_libraries(glrf_app glm)
target_link_libraries(glrf_app glrf)