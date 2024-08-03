cmake_minimum_required(VERSION 3.10.2)
project(TeleGacha)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(Boost_USE_MULTITHREADED ON)

# Find required packages
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost COMPONENTS system REQUIRED)
find_package(CURL)

# Include directories
include_directories(/usr/local/include ${OPENSSL_INCLUDE_DIR} ${Boost_INCLUDE_DIR})
if (CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIRS})
    add_definitions(-DHAVE_CURL)
endif()
include_directories(include)

# JSON library
include(FetchContent)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

# Cairo library
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)
include_directories(${CAIRO_INCLUDE_DIRS})
link_directories(${CAIRO_LIBRARY_DIRS})

# Add executable
add_executable(TeleGacha src/main.cpp src/UserManager.cpp src/GameUser.cpp)

# Link libraries
target_link_libraries(TeleGacha 
    /usr/local/lib/libTgBot.a 
    ${CAIRO_LIBRARIES} 
    ${CMAKE_THREAD_LIBS_INIT} 
    ${OPENSSL_LIBRARIES} 
    ${Boost_LIBRARIES} 
    ${CURL_LIBRARIES} 
    nlohmann_json::nlohmann_json
)

# Custom target for running the executable
add_custom_target(run
    COMMAND TeleGacha
    DEPENDS TeleGacha
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Builds and then runs TeleGacha"
)