cmake_minimum_required (VERSION 3.18)
project(RollbackGame)
  
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


option(Gpr_Assert "Activate Assertion" ON)
option(Gpr_Abort "Activate Assertion with std::abort" OFF)
option(Gpr_Exit_On_Warning "Exit on Warning Assertion" ON)
option(ENABLE_PROFILING "Enable Tracy Profiling" OFF)
option(ENABLE_SQLITE_STORE "Enable info storing in sqlite" OFF)

include(cmake/data.cmake)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wpedantic )
endif()

if(ENABLE_PROFILING)
    add_subdirectory(externals/tracy)
endif()
add_subdirectory(core)
add_subdirectory(game)
