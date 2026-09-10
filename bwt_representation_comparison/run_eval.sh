#!/bin/sh

for i in 4 8 16 21 32 64
do
  ./rbbwt sigma_genus445_${i}.txt -b 2  
done
