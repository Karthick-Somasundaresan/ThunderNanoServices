# If not stated otherwise in this file or this component's LICENSE file the
# following copyright and licenses apply:
#
# Copyright 2022 Metrological
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

project(SubsysDependentPlugin)

cmake_minimum_required(VERSION 3.15)

find_package(WPEFramework)

project_version(1.0.0)

set(MODULE_NAME ${NAMESPACE}${PROJECT_NAME})

message("Setup ${MODULE_NAME} v${PROJECT_VERSION}")

find_package(${NAMESPACE}Core REQUIRED)
find_package(${NAMESPACE}Plugins REQUIRED)
find_package(${NAMESPACE}Definitions REQUIRED)
find_package(${NAMESPACE}Messaging REQUIRED)
find_package(CompileSettingsDebug REQUIRED)

set(PLUGIN_SUBSYSDEPENDENTPLUGIN_IMPLEMENTATION "${MODULE_NAME}Impl" CACHE STRING "Library with implementation of sample_rdk_plugin")

set(PLUGIN_SUBSYSDEPENDENTPLUGIN_STARTMODE "Deactivated" CACHE STRING "Controls automatic start of the SubsysDependentPlugin plugin")
set(PLUGIN_SUBSYSDEPENDENTPLUGIN_MODE "Local" CACHE STRING "Controls if the plugin should run in its own process, in process or remote")

file(GLOB DATA_FILES "data/*")

add_library(${MODULE_NAME} SHARED
    SubsysDependentPlugin.cpp
    SubsysDependentPluginImplementation.cpp
    Module.cpp)

#add_dependencies(${MODULE_NAME}
#    ${PLUGIN_SUBSYSDEPENDENTPLUGIN_DEFINITIONS})

target_include_directories(${MODULE_NAME} SYSTEM
    PRIVATE
        ".")

target_link_libraries(${MODULE_NAME}
    PRIVATE
        ${NAMESPACE}Core::${NAMESPACE}Core
        ${NAMESPACE}Plugins::${NAMESPACE}Plugins
        ${NAMESPACE}Definitions::${NAMESPACE}Definitions
        ${NAMESPACE}Messaging::${NAMESPACE}Messaging
        CompileSettingsDebug::CompileSettingsDebug)
    #${PLUGIN_SUBSYSDEPENDENTPLUGIN_DEFINITIONS})

# Library installation section
install(TARGETS ${MODULE_NAME} DESTINATION lib/${STORAGE_DIRECTORY}/plugins COMPONENT ${NAMESPACE}_Runtime)
write_config()
