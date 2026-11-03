#!/bin/bash
# Double-click me to install SuperTune for FL Studio / any DAW on Mac.
cd "$(dirname "$0")"
VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
AU_DIR="$HOME/Library/Audio/Plug-Ins/Components"
mkdir -p "$VST3_DIR" "$AU_DIR"

echo "Installing SuperTune..."
rm -rf "$VST3_DIR/SuperTune.vst3" "$AU_DIR/SuperTune.component"
cp -R "SuperTune.vst3" "$VST3_DIR/"
[ -d "SuperTune.component" ] && cp -R "SuperTune.component" "$AU_DIR/"

# Plugin isn't Apple-signed, so remove the download quarantine flag so macOS will load it.
xattr -dr com.apple.quarantine "$VST3_DIR/SuperTune.vst3" 2>/dev/null
xattr -dr com.apple.quarantine "$AU_DIR/SuperTune.component" 2>/dev/null
killall -9 AudioComponentRegistrar 2>/dev/null   # refresh the AU list

echo ""
echo "Done! Open FL Studio -> Options -> Manage plugins -> Find more plugins. Put SuperTune on your vocal mixer track."
echo "You can close this window."
