cmake_minimum_required(VERSION 3.19)

function(add_git_submodule dir)
# add a Git submodule directory to CMake
#
# Usage: in CMakeLists.txt
# 
# include(AddGitSubmodule.cmake)
# add_git_submodule(mysubmod_dir)

find_package(Git REQUIRED)

if(NOT EXISTS ${dir}/.git)
  execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive -- ${dir}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND_ERROR_IS_FATAL ANY)
endif()

endfunction(add_git_submodule)