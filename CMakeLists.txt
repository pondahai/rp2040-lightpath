# 光路反射 —— 第一款原生遊戲，見 docs/GAME-PLAN.md
#
#   載入器版 + standalone 版：
#       cmake -B build_offset -G Ninja -DLOADER_OFFSET_BUILD=ON && ninja -C build_offset
#   編完直接推進掌機 SD 卡（掌機停在載入器選單、USB 接著）：
#       ninja -C build_offset lightpath_push
#   直接 USB 燒錄版：
#       cmake -B build -G Ninja && ninja -C build
cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(lightpath C CXX ASM)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()

include(${CMAKE_CURRENT_LIST_DIR}/../../cmake/retro_sdk.cmake)

add_executable(lightpath main.c scene.c art.c)
target_link_libraries(lightpath retro_sdk)
target_compile_options(lightpath PRIVATE -Wall -Wextra)
retro_app(lightpath)
