#!/bin/sh
set -e

# get short HEAD hash
printf "%s" "$(git rev-parse --verify --short HEAD 2>/dev/null)"

# Update index only on r/w media
[ -w . ] && ( git update-index --refresh --unmerged > /dev/null || true )

# Check for uncommitted changes
if git diff-index --name-only HEAD \
    | read dummy; then
	printf '%s' -dirty
fi
