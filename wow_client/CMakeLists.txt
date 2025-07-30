find_package(OpenSSL REQUIRED)
find_package(Boost 1.71 REQUIRED COMPONENTS thread program_options)
find_package(gsl REQUIRED)

file(GLOB_RECURSE client_sources *.cpp *.hpp)

add_executable(${BINARY_TARGET_NAME} ${client_sources})

target_include_directories( ${BINARY_TARGET_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries( ${BINARY_TARGET_NAME}
    PRIVATE
        ${Boost_LIBRARIES}
        OpenSSL::SSL OpenSSL::Crypto
)

target_compile_options( ${BINARY_TARGET_NAME}
    PRIVATE
        /std:c++latest /await:strict
)
