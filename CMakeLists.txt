cmake_minimum_required (VERSION 3.8)

project("glrf_app" VERSION 1.0 LANGUAGES C CXX)

find_package(Git)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
	message(STATUS "Git found!")
	option(GIT_SUBMODULE "Check submodules during build" ON)
	if (GIT_SUBMODULE)
		message(STATUS "Updating submodules..")
		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} RESULT_VARIABLE GIT_SUBMODULE_RESULT)
		if(NOT GIT_SUBMODULE_RESULT EQUAL "0")
			message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
		endif()
		message(STATUS "Pulling changes..")
		execute_process(COMMAND ${GIT_EXECUTABLE} pull origin master WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/external/GLRF")
	endif()
endif()

# Add source to this project's executable.
add_executable(${PROJECT_NAME} "App.cpp")

add_subdirectory("external/GLRF")

# TODO: Add tests and install targets if needed.
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(${PROJECT_NAME} PRIVATE ${GLAD_HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE ${GLAD_SOURCES})
target_link_libraries(${PROJECT_NAME} glfw)
target_link_libraries(${PROJECT_NAME} glm)
target_link_libraries(${PROJECT_NAME} glrf)