cmake_minimum_required(VERSION 3.13)
set(CMAKE_EXPORT_COMPILE_COMMANDS "true")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -O3 -lrt -Werror")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -O3 -lrt")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -Wextra -Wpedantic -O3 -lrt")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -g -Wextra -Wpedantic -O0 -lrt")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE RelWithDebInfo)
#set(CMAKE_BUILD_TYPE Relise)
project(udp-sm VERSION "1.1.0" LANGUAGES C CXX)
#get google test
include(FetchContent)
FetchContent_Declare(googletest	SOURCE_DIR "${CMAKE_SOURCE_DIR}/googletest/")
FetchContent_GetProperties(googletest)
FetchContent_MakeAvailable(googletest)
#googletest_POPULATED
#googletest_SOURCE_DIR
#googletest_BUILD_DIR
if(NOT googletest_POPULATED)
	FetchContent_Populate(googletest)
	add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BUILD_DIR})
endif()
include(FindPackageHandleStandardArgs)
find_path(EFVI_INCLUDE_DIR NAMES etherfabric/ef_vi.h)
find_library(EFVI_LIBRARY NAMES ciul1)
find_package_handle_standard_args(EFVI DEFAULT_MSG
        EFVI_LIBRARY
        EFVI_INCLUDE_DIR)
#add_library(udp-sm_lib SHARED 
add_library(udp-sm_lib STATIC 
	lib/ciapp/net.c
	inc/ci/app.h
	src/UDPJubmoFrameReceiverImplementation.h
	src/UDPJubmoFrameReceiverImplementation.cpp
	src/InputArguments.h
	src/InputArguments.cpp
	src/Logger.h
	src/Logger.cpp
	src/ProcessorState.h
	src/ProcessorState.cpp
	src/Tsc.h
	src/Tsc.cpp
	src/Percentile.h
	src/Percentile.cpp
	src/RingFIFOBuffer.h
	src/RingFIFOBuffer.cpp
        src/NetworkReceiverInterface.h
	src/NetworkReceiverImplementationEFVI.h
	src/NetworkReceiverImplementationEFVI.cpp
	src/ParserInterface.h
	src/ParserImplementation.h
	src/ParserImplementation.cpp
	src/InserterInterface.h
	src/InserterImplementation.h
	src/InserterSharedMemoryFormatDefinitions.h
	src/InserterImplementation.cpp
	src/CommonData.h
	src/defaults.h
	src/defines.h
	src/flags.h
	src/mjson.h
	src/NetInterface.h
	src/KeyboardReadInput.h
	src/KeyboardReadInput.cpp
	src/RingFIFOBuffer.h
	src/utils.h
	src/mjson.cpp
	src/NetInterface.cpp
	src/utils.cpp)
target_include_directories(udp-sm_lib
        PRIVATE
        ${EFVI_INCLUDE_DIR}
	lib/capp
	inc)
add_executable(udp-sm
	src/main.cpp
)
target_link_libraries(udp-sm
	udp-sm_lib
        ${EFVI_LIBRARY})
install(TARGETS udp-sm RUNTIME DESTINATION bin)
add_executable(udp-sm_test src/main_test.cpp)
target_link_libraries(udp-sm_test
	udp-sm_lib
        ${EFVI_LIBRARY})
target_link_libraries(udp-sm_test udp-sm_lib gtest_main gmock_main)

