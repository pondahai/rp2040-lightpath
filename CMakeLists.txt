# 光路反射 —— 以 rp2040-retro-sdk 撰寫的第一款原生遊戲
#
#   載入器版 + standalone 版：
#       cmake -B build_offset -G Ninja -DLOADER_OFFSET_BUILD=ON && ninja -C build_offset
#   編完直接推進掌機 SD 卡（掌機停在載入器選單、USB 接著）：
#       ninja -C build_offset lightpath_push   # 只推
#       ninja -C build_offset lightpath_run    # 推完直接啟動
#   直接 USB 燒錄版：
#       cmake -B build -G Ninja && ninja -C build
cmake_minimum_required(VERSION 3.13)

# 找 SDK：-DRETRO_SDK_PATH=... > ./sdk（git submodule）> ../..（放在 SDK 的 examples/ 底下）
if(NOT RETRO_SDK_PATH)
    if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/sdk/cmake/retro_sdk.cmake")
        set(RETRO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/sdk")
    elseif(EXISTS "${RETRO_SDK_PATH}/cmake/retro_sdk.cmake")
        set(RETRO_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}/../..")
    else()
        message(FATAL_ERROR "找不到 rp2040-retro-sdk。請 git submodule add 到 ./sdk，"
                            "或用 -DRETRO_SDK_PATH=<路徑> 指定。")
    endif()
endif()

include(pico_sdk_import.cmake)
project(lightpath C CXX ASM)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()

include(${RETRO_SDK_PATH}/cmake/retro_sdk.cmake)

add_executable(lightpath main.c scene.c art.c game.c levels.c pack.c)
target_link_libraries(lightpath retro_sdk_sd)
target_compile_options(lightpath PRIVATE -Wall -Wextra)
retro_app(lightpath COVER ${CMAKE_CURRENT_LIST_DIR}/assets/LIGHTPATH.RAW)
