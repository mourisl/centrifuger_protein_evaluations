#!/bin/perl

use strict ; 
use warnings ;

die "usage: a.pl alphabet_size template.txt\n" if (@ARGV == 0) ;
srand(17) ;

my $rlen = 3 ;
my $sigma = $ARGV[0] ;

my $template ;
open FP, $ARGV[1] ;
$template = <FP> ;
chomp $template ;
close FP ;


my ($i, $j) ;
my $seqlen = length($template) ;
my @chars ;

my $startChr = '0' ;
for ($i = 0 ; $i < $sigma; ++$i)
{
  push @chars, chr(ord($startChr) + $i) ;
}

my $output ;
my $prevTemplateC = "-1" ;
my $prevc = "-1" ;
my $c ;
for ($i = 0 ; $i < $seqlen ; ++$i)
{
  if (substr($template, $i, 1) ne $prevTemplateC)
  {
    $c = $chars[int(rand($sigma))] ;
    while ($c eq $prevc)
    {
      $c = $chars[int(rand($sigma))] ;
    }
 
    $prevTemplateC = substr($template, $i, 1) ;
    $prevc = $c ;
  }
  
  $output .= $c ;
}
print($output) ;
