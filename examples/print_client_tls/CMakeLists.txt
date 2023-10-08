SET(CMAKE_C_FLAGS "-std=c++17 ")
SET(CMAKE_CXX_FLAGS "-std=c++17  ")
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(ENABLE_CPP11 "true")
set(WEBSOCKETPP_BIN "client_p")
include(../../cmake/CMakeHelpers.cmake)
set (INCLUDE_DIRS  "/Users/VinayDipti/YellowBallMDFeed/websocketpp;/usr/local/include/boost")
list(APPEND includePath "/Users/VinayDipti/YellowBallMDFeed/websocketpp/" "/usr/local/include/" "/usr/local/opt/openssl/include" )
file (GLOB SOURCE_FILES *.cpp)
file (GLOB HEADER_FILES *.hpp)
find_package(Threads REQUIRED)
if(THREADS_HAVE_PTHREAD_ARG)
    message("pthread")
  target_compile_options(${TARGET_NAME} PUBLIC "-pthread")
endif()
if(CMAKE_THREAD_LIBS_INIT)
#  target_link_libraries(my_app "${CMAKE_THREAD_LIBS_INIT}")
endif()
find_package(OpenSSL REQUIRED)

if (OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
message(CMAKE_THREAD_LIBS_INIT=${CMAKE_THREAD_LIBS_INIT})
init_target (print_client_tls)

build_executable (${TARGET_NAME} ${SOURCE_FILES} ${HEADER_FILES})
target_include_directories(${TARGET_NAME} PUBLIC  "$<BUILD_INTERFACE:${includePath}>")
target_link_libraries(${TARGET_NAME} ${ZeroMQ_LIBRARY})
  target_compile_options(${TARGET_NAME} PUBLIC "-pthread")

find_package(Boost REQUIRED filesystem program_options regex timer chrono random thread)
find_package(PkgConfig)
pkg_check_modules(PC_ZeroMQ QUIET zmq)
link_boost ()
link_openssl()
find_library(ZeroMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )

final_target ()

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "examples")

endif()
