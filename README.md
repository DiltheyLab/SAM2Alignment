# SAM2Alignment

This is a small utility for converting the alignment records in a SAM file into a simple pairwise alignment format.

It should work reliably for secondary and supplementary alignments and also support SAM records with missing sequence information.

Note that the tool assumes that the SAM file is **sorted by read ID** - it will not work for positionally sorted files (but of course you can always re-sort the entries in any existing SAM/BAM/CRAM).

We mainly use this tool internally, but make it public in case anyone else finds it useful. We'd also appreciate bug reports if you come across a problem.

## Output ##

Each alignment record is represented by three lines of output:
- Header line that contains the following fields
    - read ID (carrying a `/1` or `/2` suffix for paired reads)
    - alignment coordinates on the reference
    - alignment coordinates on the read (relative to the original read; reverse-complemented alignments have inverted coordinate order)
    - number of alignment mismatches (incl. gaps), total alignment length
    - mapping quality (from SAM)
    - FLAGs field (from SAM)
    - secondary and supplementary alignment flags
    - TAGs (from SAM)
- The reference component of the alignment
- The read component of the alignment

### Example output ###
```
M02092:56:000000000-AE80E:1:1102:10073:14237/2 chr1:736702-736740 read:1-39 (0/39) mapQ=0 flags=387 secondary=1 supplementary=0 NM:i:0
CTTCAAAGACCTTGCGGGGGGGGTATTAAGTAAAAATTA
CTTCAAAGACCTTGCGGGGGGGGTATTAAGTAAAAATTA
```

## Installation ##

After cloning this repository, a simple `make all` should be all it takes.

## Running SAM2Alignment ##

When calling `sam2alignment`, provide two parameters:
- the path to the SAM file you want to analyze
- the path to the utilized reference genome in FASTA format

## Application to BAM and CRAM ##

Only SAM is directly supported as input format. If you want to analyze BAM or CRAM files, use `samtools view` for conversion into SAM format.

