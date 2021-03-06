# ########################################################################
# Copyright 2013 Advanced Micro Devices, Inc.
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ########################################################################

cmake_minimum_required(VERSION 3.1)
project(PROTOBUF_SGEMM C CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif()

if(TARGET_PLATFORM EQUAL 32 OR TARGET_PLATFORM EQUAL 64)
    set(TARGET_PLATFORM ${TARGET_PLATFORM} CACHE STRING "Target platform type (32-bit or 64-bit)" FORCE)
else()
    if(CMAKE_SIZEOF_VOID_P MATCHES 8)
        set(TARGET_PLATFORM "64" CACHE STRING "Target platform type (32-bit or 64-bit)" FORCE)
    else()
        set(TARGET_PLATFORM "32" CACHE STRING "Target platform type (32-bit or 64-bit)" FORCE)
    endif()
endif()

message(STATUS "Target platform: ${TARGET_PLATFORM}-bit")
if(TARGET_PLATFORM EQUAL 32)
    set(_arch "x86" INTERNAL)
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS FALSE)
else()
    set(_arch "x86_64" INTERNAL)
    set_property(GLOBAL PROPERTY FIND_LIBRARY_USE_LIB64_PATHS TRUE)
endif()

set(PROTOBUF_SGEMM_SRC protobuf_sgemm.cpp)


add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D EXINTERFACE)

# Having problems on build server, compiling gtest headers with -pedantic; disabling detection of long long
# http://code.google.com/p/googletest/issues/detail?id=334
if( CMAKE_COMPILER_IS_GNUCXX )
    add_definitions( -Wno-long-long )
endif( )


# We have custom written Find* modules now in the root source directory
set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR} )
# message(STATUS ${CMAKE_MODULE_PATH})

#define OPENCL_FOUND
find_package( OpenCL )
find_package( Protobuf REQUIRED)






include_directories(
    ${OPENCL_INCLUDE_DIRS}
    ${PROTOBUF_INCLUDE_DIRS}
    .)

add_executable(PROTOBUF_SGEMM ${PROTOBUF_SGEMM_SRC})
target_link_libraries(PROTOBUF_SGEMM 
                      ${OPENCL_LIBRARIES}
                      ${PROTOBUF_LIBRARIES})
message(STATUS "OpenCL libararies: ${OPENCL_LIBRARIES}")

