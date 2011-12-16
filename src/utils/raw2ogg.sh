#!/bin/sh

SCRIPT=`basename $0 .sh`

function die () {
	echo 1>&2 "$SCRIPT:" "$@"
	exit 1
}

if [ $# -lt 2 -o "$1" = "-h" -o "$1" = "--help" ] ; then
	echo 1>&2 "usage: $SCRIPT output_dir [raw_files...]"
	exit 0
fi

OUTDIR="$1"; shift

mkdir -p $OUTDIR || die "cant create dir $OUTDIR"

for file in "$@" ; do

	outfile="$OUTDIR/`basename "$file" .raw`.ogg"

	oggenc --raw --raw-bits=8 --raw-chan=1 --raw-rate=11025 \
		--quiet --bitrate=16 "$file" -o "$outfile"
done
