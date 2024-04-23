#!/bin/sh

echo Install directory: $2 > /tmp/sw_install.log

if [ -d "/Library/Audio/Plug-Ins/Components/SpectrumWorx.component" ]
then
    ln -s -f "$2Library/Application Support/Little Endian/SpectrumWorx/SpectrumWorx.dylib" "$2Library/Audio/Plug-Ins/Components/SpectrumWorx.component/Contents/MacOS/SpectrumWorx" >> /tmp/sw_install.log
else
    echo "<AU not found>" >> /tmp/sw_install.log
fi

if [ -d "$2Library/Audio/Plug-Ins/VST/SpectrumWorx.vst" ]
then
    ln -s -f "$2Library/Application Support/Little Endian/SpectrumWorx/SpectrumWorx.dylib" "$2Library/Audio/Plug-Ins/VST/SpectrumWorx.vst/Contents/MacOS/SpectrumWorx" >> /tmp/sw_install.log
else
    echo "<VST not found>" >> /tmp/sw_install.log
fi

exit 0
