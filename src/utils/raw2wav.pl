#!/usr/bin/perl -w

use Getopt::Std;
use File::Basename;
use File::Spec::Functions;

sub make_header($) { # arg1: raw data length
    # after http://ccrma.stanford.edu/courses/422/projects/WaveFormat/
    return pack(
        "A4 V A4 A4 V v2 V2 v2 A4 V",
        "RIFF",
        36 + $_[0],     # length of whole file - 8
        "WAVE",
        "fmt",
        16,             # 16 for PCM
        1,              # 1 = PCM
        1,              # Mono
        11025,          # Sample rate
        11025,          # Byte rate
        1,              # bytes/sample (all channels)
        8,              # bits per sample
        "data",
        $_[0],          # data length
    );
}

my $outdir = ".";
my $USAGE = "usage: raw2wav [-d output_dir] [files...]\n";

getopts('d:') or die $USAGE;
$outdir = $opt_d if defined $opt_d;
die "Not a writable directory: $outdir\n"
    unless -d $outdir and -w _;

undef $/;   # slurp files at once
if (@ARGV) {
    for (@ARGV) {
        my $outname = catfile($outdir, basename($_, qw/.raw .RAW/).".wav");
        open FI, "<", $_
            or do { warn "raw2wav: skipping $_: $!\n"; next };
        open FO, ">", $outname
            or do { warn "raw2wav: skipping $outname: $!\n"; next };
        binmode FI;
        binmode FO;
        # could do stat, but thats easy and works for pipes, too
        my $bytes = <FI>;
        print FO make_header(length $bytes);
        print FO $bytes;
        close FI;
        close FO;
    }
} else {
    # go for stdin > stdout
    die "raw2wav: stdin & stdout must not be tty\n"
        if -t STDIN or -t STDOUT;
    binmode STDIN;
    binmode STDOUT;
    my $bytes = <>;
    print make_header(length $bytes);
    print $bytes;
    close STDOUT;
}

exit 0;
