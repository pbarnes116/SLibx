cmake_minimum_required(VERSION 2.8)
project(slibx)

file (GLOB SLIBX_SFILE_FILES ${CMAKE_CURRENT_LIST_DIR}/../../../inc/slibx/sfile/*.h ${CMAKE_CURRENT_LIST_DIR}/../../../src/slibx/sfile/*.cpp)
add_library(slibx-sfile ${SLIBX_SFILE_FILES})

file (GLOB SLIBX_SNET_FILES ${CMAKE_CURRENT_LIST_DIR}/../../../inc/slibx/snet/*.h ${CMAKE_CURRENT_LIST_DIR}/../../../src/slibx/snet/*.cpp)
add_library(slibx-snet ${SLIBX_SNET_FILES})

target_link_libraries(slibx-sfile slibx-snet)
