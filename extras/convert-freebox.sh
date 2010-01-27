#!/bin/sh

test -n "$LD_LIBRARY_PATH" && LD_LIBRARY_PATH="/usr/lib/convert-freebox:$LD_LIBRARY_PATH"
test -z "$LD_LIBRARY_PATH" && LD_LIBRARY_PATH=/usr/lib/convert-freebox

export LD_LIBRARY_PATH

/usr/lib/convert-freebox/convert-freebox
