#!/usr/bin/env perl
use strict;
use warnings;
use English;

use Data::Dumper;
use GD::Simple;
use Math::Trig;

my $infile = $ARGV[0];
my $outfile = $ARGV[1];
unless (defined $infile && defined $outfile) {
    die "Usage: $PROGRAM_NAME <infile.txt> <outfile.png>\n";
}

my @ranges = ();
open my $fh, "<", $infile
    or die "Failed to open $infile: $!\n";
while (<$fh>) {
    chomp;
    push @ranges, $_
        if (length($_));
}
close $fh;

my $img = GD::Simple->new(800, 800);
my $col_white = $img->colorAllocate(255, 255, 255);
my $col_black = $img->colorAllocate(0, 0, 0);
my $col_red = $img->colorAllocate(255, 0, 0);
$img->bgcolor($col_white);
$img->fgcolor($col_red);
$img->penSize(3, 3);
$img->moveTo(400, 400);
$img->ellipse(10, 10);
$img->fgcolor($col_black);
$img->penSize(1, 1);

my $angle = 0;
foreach my $range (@ranges) {
    if ($range) {
        my $rad = $angle * pi / 180;
        my $a = $range * sin($rad);
        my $b = $range * cos($rad);

        my $x = 400 + 50 * $b;
        my $y = 400 + 50 * $a;

        print "x=$x, y=$y\n";

        $img->moveTo($x, $y);
        $img->ellipse(2, 2);
    }
    $angle += 0.5;
}

open $fh, ">", $outfile
    or die "Failed to open $outfile: $!\n";
print $fh $img->png;
close $fh;
