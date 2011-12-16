#!/bin/bash

# sync documentation with the project web server.
# required arguments:
#	1: local working copy
#	2: web server username

REMOTE_HOST=shell.sourceforge.net
REMOTE_DIR=/home/groups/r/ra/raceintospace/htdocs/doxygen/
DOC_DIR=./doc/html/

set -e
dir=${1:?"arg 1 required: working copy dir"}
user=${2:?"arg 2 required: remote username"}

cd $dir
if cvs diff -NauR >/dev/null ; then exit ; fi
cvs update -R >/dev/null
doxygen Doxyfile.quick >/dev/null
chmod a+r -R "$DOC_DIR"
rsync --archive --quiet \
	--cvs-exclude --exclude '*.md5'	--exclude '*.map' \
	--delete-after --delete-excluded \
	"$DOC_DIR" "$user@$REMOTE_HOST:$REMOTE_DIR"
