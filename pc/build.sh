#!/bin/sh
# 用主機的 gcc 編 PC 預覽與測試，並跑測試。只編 lib/ 與遊戲的純 C 部分，不碰 hal/。
#   sh pc/build.sh && build_pc/preview out.png 1 RRA
set -e
HERE=$(cd "$(dirname "$0")" && pwd)
GAME=$HERE/..
# 找 SDK：$RETRO_SDK_PATH > ./sdk（submodule）> ../..（SDK 的 examples/ 底下）
if [ -n "$RETRO_SDK_PATH" ]; then SDK=$RETRO_SDK_PATH
elif [ -f "$GAME/sdk/lib/gfx.c" ]; then SDK=$GAME/sdk
elif [ -f "$GAME/../../lib/gfx.c" ]; then SDK=$GAME/../..
else echo "找不到 rp2040-retro-sdk，請設 RETRO_SDK_PATH" >&2; exit 1
fi
OUT=${OUT:-$GAME/build_pc}
mkdir -p "$OUT"
CFLAGS="-std=c11 -O2 -Wall -Wextra -I$SDK/lib -I$SDK/tools/pc -I$GAME"
CORE="$GAME/game.c $GAME/levels.c $GAME/pack.c $GAME/art.c $SDK/lib/gfx.c"

${CC:-gcc} $CFLAGS "$GAME/test/test_game.c" $CORE -o "$OUT/test_game"
${CC:-gcc} $CFLAGS "$HERE/preview.c" "$GAME/scene.c" $CORE "$SDK/tools/pc/png.c" \
    -o "$OUT/preview"
"$OUT/test_game"
