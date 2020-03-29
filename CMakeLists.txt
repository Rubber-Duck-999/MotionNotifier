  
cmake_minimum_required(VERSION 3.10)
set (PROJECT_NAME exeCameraMonitor)
project(${PROJECT_NAME} VERSION 1.0.0 LANGUAGES CXX)

set(Boost_USE_STATIC_LIBS ON)
#ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
# Boost::log required Boost version >= 1.54.0 
find_package(Boost 1.61.0 REQUIRED COMPONENTS log)
find_package(Threads)

# Folder where the Coverage module is
#set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

# Source files folder
set (SRC_DIR src)

# Header files folder
set (INCL_DIR include)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O3 -Wall")

include_directories(${Boost_INCLUDE_DIRS} 
                    /usr/local/include
                    ${PROJECT_SOURCE_DIR}/${INCL_DIR}
                    ${PROJECT_SOURCE_DIR}/boost)
                    
link_directories(${Boost_LIBRARY_DIRS})
# Important: Include all source files on this list
set (SOURCE
    ${SRC_DIR}/main.cc
)

link_directories(/usr/local/lib)

# Output folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build")

# Add the executable for the program
add_executable(${PROJECT_NAME} ${SOURCE})

target_link_libraries(${PROJECT_NAME}
                       ${Boost_LIBRARIES} 
                       pthread
                       amqpcpp
                       event
                       Threads::Threads)