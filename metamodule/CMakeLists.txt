cmake_minimum_required(VERSION 3.22)

set(METAMODULE_SDK_DIR ${CMAKE_CURRENT_LIST_DIR}/../metamodule-plugin-sdk)

message("METAMODULE_SDK_DIR set to ${METAMODULE_SDK_DIR}")

include(${METAMODULE_SDK_DIR}/plugin.cmake)

project(SchlappiEngineeringMetaModulePlugin
        VERSION 0.1
        DESCRIPTION "SchlappiEngineering Plugin for Metamodule"
        LANGUAGES   C CXX ASM
)

add_library(schlappiengineering STATIC)

set (SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../src)

target_sources(schlappiengineering
        PRIVATE
        ${SOURCE_DIR}/btfld.cpp
        ${SOURCE_DIR}/btmx.cpp
        ${SOURCE_DIR}/plugin.cpp
        ${SOURCE_DIR}/nibbler.cpp
)

target_include_directories(schlappiengineering
        PRIVATE
        ${SOURCE_DIR}
)

set_property(TARGET schlappiengineering PROPERTY CXX_STANDARD 20)

create_plugin(
        SOURCE_LIB schlappiengineering
        PLUGIN_NAME schlappiengineering
        PLUGIN_JSON ${CMAKE_CURRENT_LIST_DIR}/../plugin.json
        SOURCE_ASSETS ${CMAKE_CURRENT_LIST_DIR}/assets
        DESTINATION ${INSTALL_DIR}
)