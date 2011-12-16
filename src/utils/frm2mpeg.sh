#!/bin/sh -x

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

mkdir -p $TMPDIR || die "cant create dir $TMPDIR"
mkdir -p $OUTDIR || die "cant create dir $OUTDIR"

for file in "$@" ; do
	lastline=`./mkmovie -d $TMPDIR -b $BASENAME "$file"`
	if [ $? -ne 0 ] ; then
		echo "processing $file failed, continuing..."
		continue
	fi

	numfiles=`echo "$lastline" | sed -e '$!d' -e 's/^\([0-9]*\).*/\1/'`
	lastfile=`expr $numfiles - 1`
	outfile="$OUTDIR/`basename "$file" .frm`.mpeg"

	# for explanation of parameters below see
	# http://netpbm.sourceforge.net/doc/ppmtompeg.html
	cat <<EOF | ppmtompeg -realquiet /dev/stdin
INPUT_DIR           $TMPDIR
INPUT
$BASENAME.*.ppm [0000-$lastfile]
END_INPUT
INPUT_CONVERT       *
BASE_FILE_FORMAT    PPM
OUTPUT              $outfile

PATTERN             IBBPBBPBBPBBPBB
IQSCALE             16
BQSCALE             16
PQSCALE             16
PSEARCH_ALG         TWOLEVEL
BSEARCH_ALG         CROSS2
SLICES_PER_FRAME    1
SIZE                160x100
GOP_SIZE            15
PIXEL               HALF
SLICES_PER_FRAME    5
RANGE               5
REFERENCE_FRAME     ORIGINAL
FRAME_RATE          24
FORCE_ENCODE_LAST_FRAME
EOF
done

rm -rf "$TMPDIR"
