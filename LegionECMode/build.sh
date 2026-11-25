#!/bin/bash
set -euo pipefail

SDK=$(xcrun --show-sdk-path --sdk macosx)
OUT_DIR="build/LegionECMode.kext"

rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR/Contents/MacOS"
cp Info.plist "$OUT_DIR/Contents/Info.plist"

clang++ -arch x86_64 \
  -isysroot "$SDK" \
  -I"$SDK/System/Library/Frameworks/Kernel.framework/Headers" \
  -I"$SDK/System/Library/Frameworks/Kernel.framework/PrivateHeaders" \
  -DAPPLE -DKERNEL -DKERNEL_PRIVATE -DX86_64 -D__x86_64__ \
  -O2 -Wall -fno-builtin -fno-exceptions -fno-rtti \
  -mkernel -nostdlib -Xlinker -kext -lkmodc++ -lkmod -lcc_kext \
  LegionECMode.cpp -o "$OUT_DIR/Contents/MacOS/LegionECMode"

chmod 755 "$OUT_DIR/Contents/MacOS/LegionECMode"
plutil -lint "$OUT_DIR/Contents/Info.plist"
nm "$OUT_DIR/Contents/MacOS/LegionECMode" | grep -E "kmod_info|_start|_stop"
echo "Built: $OUT_DIR"
