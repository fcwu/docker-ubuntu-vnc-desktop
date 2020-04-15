#!/bin/sh

if [ -z "$ALSADEV" ]; then
    zenity --error --text "To support audio, please read README.md and run container with --device /dev/snd -e ALSADEV=..."
    exit 1
fi

exec /usr/bin/chromium-browser --no-sandbox --alsa-output-device="$ALSADEV" "$@"
