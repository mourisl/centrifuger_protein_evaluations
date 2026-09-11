# Read in the folders and find the files downloaded by centrifuger-download and create a file that works
#!/bin/env/perl 

use strict ;
use warnings ;

die "usage: a.pl cfr_inspected_seq2taxid.map folders...\n" if (@ARGV == 0) ;

my %seqIdToTaxId ;
open FP, $ARGV[0] ;
while (<FP>)
{
  chomp ;
  my @cols = split ;
  $seqIdToTaxId{$cols[0]} = $cols[1] ;
}
close FP ;

my $i ;
my %usedSeqId ;
for ($i = 1 ; $i < scalar(@ARGV) ; ++$i)
{
  my $folder = $ARGV[$i] ;
  my %fname2TaxId ;
  open FP, $folder."/assembly_summary_filtered.txt" ; # Use this file in case the centrifuger inspect does not provide the sequence ID (e.g., the seueqnce is filtered).
  while (<FP>)
  {
    chomp ;
    my @cols = split /\t/, $_ ;
    $fname2TaxId{ $cols[0] } = $cols[6] ;
  }
  close FP ;

  for my $f (glob "$folder/*.faa.gz")
  {
    my @cols = split /_/, (split /\//, $f)[1] ;
    my $prefix = $cols[0]."_".$cols[1] ;
    if (!defined $fname2TaxId{$prefix})
    {
      die "$prefix is not in assembly summary file!\n" ; 
    }
    my $rawtaxid = $fname2TaxId{$prefix} ;
    my $filter = 0 ;
    open FP, "zcat $f | "  ;
    while (<FP>)
    {
      if (/^>/)
      {
        my $seqid = substr((split)[0], 1) ;
        if (defined $usedSeqId{$seqid}) 
        {
          $filter = 1 ;
          next ;
        }

        $usedSeqId{$seqid} = 1 ;
        $filter = 0 ;
        if (defined $seqIdToTaxId{$seqid})
        {
          print ">".$seqid."_".$seqIdToTaxId{$seqid}."\n" ;
        }
        else
        {
          print ">".$seqid."_$rawtaxid\n" ;
        }
      }
      elsif ($filter == 0)
      {
        print $_ ;
      }
    }
    close FP ;
  }
}
