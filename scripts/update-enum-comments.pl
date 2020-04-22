#!/usr/bin/perl

# updates enum comments; see 0e66b34d481b5ba4db6142c83754ef659ed73c8c how the diff looks like

use Encode qw(encode decode);
use FindBin;
use Mojo::File 'path';

use warnings;
use strict;
use utf8;

my $verbose = 0;
my $dry_run = 0;

sub format_enum_entry {
    my ($full_match, $enum_name, $enum_value, $existing_comment) = @_;

    # convert the enum value to a human readable representation
    my $as_text = decode('iso-8859-1', pack('N', hex $enum_value)); # interpret hex number as latin-1
    $as_text =~ s/[^[:print:]]/?/g;                                 # remove any non-printable characters

    # preserve existing comment
    my $additional_comment = '';
    if (defined $existing_comment) {
        $existing_comment =~ s/^.*:\s*//;
        $existing_comment =~ s/^\s+|\s+$//g;
        $additional_comment = ": $existing_comment" if $existing_comment ne $as_text;
    }

    # format the enum entry
    return "$enum_name = $enum_value, /**< $as_text$additional_comment */";
}

my $src_files = path($FindBin::Bin, '..')->realpath->list_tree;
for my $src_file (@$src_files) {
    # process only files containing IDs
    next unless $src_file =~ qr/.*ids.h$/;
    print("processing $src_file\n") if $verbose;

    # replace all enum value definitions to have a comment
    my $file_contents = decode('UTF-8', $src_file->slurp, Encode::FB_CROAK);
    next unless $file_contents =~ s/(([a-zA-Z][a-zA-Z0-9]*) *= *(0x[a-fA-F0-9][a-fA-F0-9][a-fA-F0-9]+) *,? *((\/\/|\/\*\*\<) *([^\*\n]*) *(\*\/)?)?)/format_enum_entry($1, $2, $3, $6)/eg;

    # write the amended file contents back to the file
    my $output = encode('UTF-8', $file_contents, Encode::FB_CROAK);
    print("writing $src_file\n") if $verbose;
    print($output) if $dry_run;
    $src_file->spurt($output) unless $dry_run;
}
