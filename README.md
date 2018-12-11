# SAM2Alignment

This is a small utility for converting the alignment records in a SAM file into a simple pairwise alignment format.

It should work reliably for secondary and supplementary alignments and also support SAM records with missing sequence information.

Note that the tool assumes that the SAM file is **sorted by read read ID** - it will not work for positionally sorted files (but of course you can always re-sort the entries in any existing BAM/SAM/CRAM).

We mainly use this tool internally, but make it public in case anyone else finds it useful. We'd also appreciate bug reports if you come across a problem.

## Installation ##

After cloning this repository, a simple `make all` should be all it takes.

## Running SAM2Alignment ##

When calling `sam2alignment`, provide two parameters:
- the path to the SAM file you want to analyze
- the path to the utilized reference genome in FASTA format

## Application to BAM and CRAM ##

Only SAM is directly supported as input format. If you want to analyze BAM or CRAM files, use `samtools view` for conversion into SAM format.

