cmake_minimum_required(VERSION 3.5)
project(potato_contracts VERSION 1.5.1)

set(POTATO_CDT_VERSION_MIN "1.4")
set(POTATO_CDT_VERSION_SOFT_MAX "1.4")
#set(POTATO_CDT_VERSION_HARD_MAX "")

find_package(potato.cdt)

### Check the version of potato.cdt
set(VERSION_MATCH_ERROR_MSG "")
POTATO_CHECK_VERSION(VERSION_OUTPUT "${POTATO_CDT_VERSION}"
                                   "${POTATO_CDT_VERSION_MIN}"
                                   "${POTATO_CDT_VERSION_SOFT_MAX}"
                                   "${POTATO_CDT_VERSION_HARD_MAX}"
                                   VERSION_MATCH_ERROR_MSG)
if(VERSION_OUTPUT STREQUAL "MATCH")
   message(STATUS "Using potato.cdt version ${POTATO_CDT_VERSION}")
elseif(VERSION_OUTPUT STREQUAL "WARN")
   message(WARNING "Using potato.cdt version ${POTATO_CDT_VERSION} even though it exceeds the maximum supported version of ${POTATO_CDT_VERSION_SOFT_MAX}; continuing with configuration, however build may fail.\nIt is recommended to use potato.cdt version ${POTATO_CDT_VERSION_SOFT_MAX}.x")
else() # INVALID OR MISMATCH
   message(FATAL_ERROR "Found potato.cdt version ${POTATO_CDT_VERSION} but it does not satisfy version requirements: ${VERSION_MATCH_ERROR_MSG}\nPlease use potato.cdt version ${POTATO_CDT_VERSION_SOFT_MAX}.x")
endif(VERSION_OUTPUT STREQUAL "MATCH")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
   set(TEST_BUILD_TYPE "Debug")
   set(CMAKE_BUILD_TYPE "Release")
else()
   set(TEST_BUILD_TYPE ${CMAKE_BUILD_TYPE})
endif()

if(NOT DEFINED ENV{SYSTEM_CONTRACTS_PATH})
   message(FATAL_ERROR "not defined environment variable:SYSTEM_CONTRACTS_PATH")
endif()

add_subdirectory(poker)
add_subdirectory(dice)

if (APPLE)
   set(OPENSSL_ROOT "/usr/local/opt/openssl")
elseif (UNIX)
   set(OPENSSL_ROOT "/usr/include/openssl")
endif()
set(SECP256K1_ROOT "/usr/local")

