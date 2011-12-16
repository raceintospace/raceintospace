#!/usr/bin/perl

use Fcntl qw/:seek/;
use File::Basename;
use Getopt::Std;

my $SCRIPT = basename($0, qw/.pl/);

sub usage(;$) {
    print "usage: $SCRIPT [-d dest_dir] {us}news.cdr...\n";
    exit (shift) ? 1 : 0;
}

getopts('hd:') or usage(1);
usage() if $opt_h;
usage(1) if @ARGV < 1;

my $dest_dir = '.';
$dest_dir = $opt_d if defined $opt_d;

my $base_name = basename($ARGV[0], qw/.cdr/);

my @range = ();

sub process_file($) {
    my $hdr;
    my @headers = ();
    my @crange = @range;
    my ($offset, $size);
    my $fname = shift;

    if (!open INPUT, $fname) {
        warn "can't open $fname: $!\n";
        return;
    }

    # loop over headers
    my $i = 0;
    while (read INPUT, $hdr, 8) {
        # VV = two little-endian 32-bit unsigned ints
        ($offset, $size) = unpack "VV", $hdr;
        # the audio data is 8-bit unsigned, and that's why probably all 4
        # bytes will be larger than max 3-byte value 
        if ($i and ($offset > 1 << 24 or $size > 1 << 24)) {
            seek INPUT, -8, SEEK_CUR or die "seek: $!\n";
            last;
        }
        push @headers, [$i, $offset, $size];
        $i++;
    }

    warn sprintf "%s has %d header%s\n", fname, $i, ($i > 1 ? "s" : "");

    for (@headers) {
        my ($i, $off, $len) = @$_;
        my $fname = sprintf "%s/%s_%03d.raw", $dest_dir, $base_name, $i;
        open OUT, ">", $fname or die "open to write: $!\n";
        seek INPUT, $off, SEEK_SET or die "seek: $!\n";
        my $read = read INPUT, $bytes, $len;
        die "read: $!\n" if not defined $read;
        die "end of file?\n" if $read == 0;
        warn "short read for sample $i, read $read instead of $len bytes\n"
        if $read != $len;
        print OUT $bytes or die "write: $!\n";
        close OUT or die "close: $!\n";
    }

    close INPUT;
}

for my $fname (@ARGV) {
    warn "processing $fname\n";
    process_file($fname);
}
