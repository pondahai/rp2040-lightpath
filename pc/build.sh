#!/bin/sh
# 用主機的 gcc 編 PC 預覽。只編 lib/ 與遊戲的純 C 部分，不碰 hal/。
#   sh examples/lightpath/pc/build.sh && ./build_pc/preview out.png RRDA
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
SDK=$HERE/../../..
OUT=${OUT:-$HERE/../build_pc}
mkdir -p "$OUT"
${CC:-gcc} -std=c11 -O2 -Wall -Wextra \
    -I"$SDK/lib" -I"$SDK/tools/pc" -I"$HERE/.." \
    "$HERE/preview.c" "$HERE/../scene.c" "$HERE/../art.c" \
    "$SDK/lib/gfx.c" "$SDK/tools/pc/png.c" \
    -o "$OUT/preview"
echo "$OUT/preview"
