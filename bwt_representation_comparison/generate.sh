#!/bin/sh

for i in 4 8 16 21 32 64
do
  perl GenerateSeqFromTemplate.pl $i genus445.txt.bwt > sigma_genus445_${i}.txt 
done
