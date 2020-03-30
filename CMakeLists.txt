  
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
                    /usr/local/include/opencv4)
                    
link_directories(${Boost_LIBRARY_DIRS})
# Important: Include all source files on this list
set (SOURCE
    ${SRC_DIR}/object.cc
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
                       Threads::Threads
                       opencv_gapi 
                       opencv_stitching 
                       opencv_aruco 
                       opencv_bgsegm 
                       opencv_bioinspired 
                       opencv_ccalib 
                       opencv_dnn_objdetect 
                       opencv_dnn_superres
                       opencv_dpm 
                       opencv_highgui 
                       opencv_face 
                       opencv_freetype 
                       opencv_fuzzy 
                       opencv_hfs 
                       opencv_img_hash 
                       opencv_line_descriptor 
                       opencv_quality 
                       opencv_reg 
                       opencv_rgbd 
                       opencv_saliency 
                       opencv_stereo 
                       opencv_structured_light 
                       opencv_phase_unwrapping 
                       opencv_superres 
                       opencv_optflow 
                       opencv_surface_matching 
                       opencv_tracking 
                       opencv_datasets 
                       opencv_text 
                       opencv_dnn 
                       opencv_plot 
                       opencv_videostab 
                       opencv_video 
                       opencv_videoio 
                       opencv_xfeatures2d 
                       opencv_shape 
                       opencv_ml 
                       opencv_ximgproc 
                       opencv_xobjdetect 
                       opencv_objdetect 
                       opencv_calib3d 
                       opencv_imgcodecs 
                       opencv_features2d 
                       opencv_flann 
                       opencv_xphoto 
                       opencv_photo 
                       opencv_imgproc 
                       opencv_core)