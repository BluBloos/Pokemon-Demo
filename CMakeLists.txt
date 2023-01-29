cmake_minimum_required(VERSION 3.10)

project(Pokemon-Demo
    VERSION 0.1.0
    DESCRIPTION "The best game of all time."
    LANGUAGES CXX)

# NOTE(Noah): It appears that the precise mechanism that these CMake cache things work is that
# ... lookup of a particular name first searches for regular variables of this name, and if
# not found will look for cache variables of this name.
set(ProjectName "Pokemon-Demo")
set(ProjectRoot "${CMAKE_CURRENT_SOURCE_DIR}")
set(ProjectBackend "CPU_BACKEND")
set(ProjectExplicitSources ON)
set(PROJ_SOURCES "${ProjectRoot}/src/PokemonDemo_automata_engine_wrapper.cpp")
set(ProjectResourcesExtraPath "${CMAKE_CURRENT_SOURCE_DIR}/Data/")
set(ProjectExplicitResOutputDir Data)
message( STATUS "ProjectResourcesExtraPath = ${ProjectResourcesExtraPath}")

add_subdirectory(Automata-Engine)