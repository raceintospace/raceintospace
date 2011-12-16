#!/bin/sh

SCRIPT=`basename $0 .sh`
TMPDIR=/tmp/frames
BASENAME=f

function die () {
	echo 1>&2 "$SCRIPT:" "$@"
	exit 1
}

if [ $# -lt 2 -o "$1" = "-h" -o "$1" = "--help" ] ; then
	echo 1>&2 "usage: $SCRIPT output_dir [frm_files...]"
	exit 0
fi

OUTDIR="$1"; shift

[ -d "$TMPDIR" ] || remove=yes
mkdir -p $TMPDIR || die "cant create dir $TMPDIR"
mkdir -p $OUTDIR || die "cant create dir $OUTDIR"

# XXX: on my system ffmpeg2theora misbehaves and turns off echoing on tty
trap 'stty echo' EXIT

for file in "$@" ; do

	rm -f "$TMPDIR/$BASENAME".*

	fps=`./mkmovie -d $TMPDIR -b $BASENAME "$file" | 
			sed 's/^.*[^0-9]\([0-9]*\)fps.*/\1/'`

	if [ $? -ne 0 ] ; then
		echo "$SCRIPT: processing $file failed, continuing..."
		continue
	fi

	outfile="$OUTDIR/`basename "$file" .frm`.ogg"

	ffmpeg2theora --inputfps $fps -f ppm --optimize -o "$outfile" \
	"$TMPDIR/$BASENAME.%04d.ppm"

done

[ "x$remove" = xyes ] && rm -fr "$TMPDIR"

