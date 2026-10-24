#!/bin/bash
set -e
export PSPDEV="/home/abdul/pspdev"
export PATH="$PSPDEV/bin:/usr/local/bin:/usr/bin:/bin"
export PSPSDK="$PSPDEV/psp/sdk"

cd "$(dirname "$0")"
make clean || true
make
make package
ls -lh EBOOT.PBP 2PlayerGames_PSP.zip
