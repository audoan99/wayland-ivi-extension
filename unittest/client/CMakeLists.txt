############################################################################
#
# Copyright 2014 BMW Car IT GmbH
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#               http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
############################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
cmake_policy(SET CMP0046 OLD)

PROJECT(ilm_client_unittest)

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ivi-layermanagement-api/ilmCommon/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ivi-layermanagement-api/ilmControl/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ivi-layermanagement-api/ilmInput/include
    ${CMAKE_CURRENT_BINARY_DIR}/../../ivi-layermanagement-api/ilmControl
    ${CMAKE_CURRENT_BINARY_DIR}/../../protocol
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${WAYLAND_CLIENT_INCLUDE_DIRS}
    ${gtest_INCLUDE_DIRS}
)

LINK_DIRECTORIES(
    ${WAYLAND_CLIENT_LIBRARY_DIRS}
)

SET(LIBS
    ilmCommon
    ilmControl
    ilmInput
    ${gtest_LIBRARIES}
)

SET(SRC_COMMON_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/client_api_fake.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../common/common_fake_api.c
)

SET(SRC_IVI_COMMON_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ilm_common_unittests.cpp
)

SET(SRC_IVI_CONTROL_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ilm_control_base_class.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ilm_control_unittests.cpp
)

SET(SRC_IVI_INPUT_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ilm_input_uinttests.cpp
)


SET(GCC_SANITIZER_COMPILE_FLAGS "-fsanitize=address -fsanitize=undefined -fno-sanitize-recover -fstack-protector-all -fpermissive")
SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_SANITIZER_COMPILE_FLAGS} -pthread" )
SET( CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -static-libasan -static-libubsan" )

ADD_EXECUTABLE(test_ilm_client_common ${SRC_COMMON_FILES} ${SRC_IVI_COMMON_FILES})
TARGET_LINK_LIBRARIES(test_ilm_client_common ${LIBS})
ADD_DEPENDENCIES(test_ilm_client_common ${LIBS})
INSTALL(TARGETS test_ilm_client_common DESTINATION bin/tests)

ADD_EXECUTABLE(test_ilm_client_input ${SRC_COMMON_FILES} ${SRC_IVI_INPUT_FILES})
TARGET_LINK_LIBRARIES(test_ilm_client_input ${LIBS})
ADD_DEPENDENCIES(test_ilm_client_input ${LIBS})
INSTALL(TARGETS test_ilm_client_input DESTINATION bin/tests)

ADD_EXECUTABLE(test_ilm_client_control ${SRC_COMMON_FILES} ${SRC_IVI_CONTROL_FILES})
TARGET_LINK_LIBRARIES(test_ilm_client_control ${LIBS})
ADD_DEPENDENCIES(test_ilm_client_control ${LIBS})
INSTALL(TARGETS test_ilm_client_control DESTINATION bin/tests)