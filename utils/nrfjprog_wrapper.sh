#!/bin/bash
set -e
set -o pipefail
set -u

"$(brew --prefix)/bin/nrfjprog" \
    --jdll "${JLINK_DLL_X86_64_PATH}"/libjlinkarm.dylib \
    "$@"