
cmake_minimum_required(VERSION 3.20)
include(cmake/vcpkg/Include.cmake)

# we need to set it here instead of e.g. the CmakeList file of editor because VS_STARTUP_PROJECT requires to be in the same file scope as project call
set( BINARY_TARGET_NAME "Headlessy" )

project(Headlessy)

if(MSVC)
    # MP and Zf flags go together, see https://learn.microsoft.com/en-us/cpp/build/reference/zf?view=msvc-170
    add_compile_options("/MP" "/Zf" "/Zi" "/W2")
    # Generate PDB, but let optimizer do its thing
    add_link_options("$<IF:$<CONFIG:Release>,/DEBUG;/OPT:REF;/OPT:ICF,/DEBUG>")
    
endif()

include(cmake/vcpkg/Include.cmake)

add_subdirectory(wow_client)

if ( ${CMAKE_GENERATOR} MATCHES "Visual Studio" )
    # This only works in the same file scope as the project call
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${BINARY_TARGET_NAME})
endif()


