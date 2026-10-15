cmake_minimum_required(VERSION 3.16)

# LDR version: only common/ is needed (no esp-idf-lib I2C components)
list(APPEND EXTRA_COMPONENT_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../../common"
)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(light_sensor)
