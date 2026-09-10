### Obtain certain species/genus sequences 
0. Requires the downloaded prokaryotic genomes data(p_genomes.fna); taxonomy tree file (nodes.dm); seqid to taxid map file (p_seqid.map)

Compile the testing program rbbwt by copying the rbbwt.cpp code to the compactds folder in centrifuger package and run `make rbbwt` to generate the rbbwt executable.

1. Get the seq id associated with genus 455. "taxonomy.py" can be found in the script folder in the Centrifuger package.
```
fgrep -w -f <(python3 ./taxonomy.py --op subtree --taxid 455 --tree nodes.dmp) p_seqid.map | cut -d' ' -f1 > genus455_seqid.out
```

2. Get the sequences with those seqids
```
perl ~/Tools/SelectFa.pl genus455_seqid.out < p_genomes.fna > genus455_genomes.fna
```

3. Get non-plasmid sequences
```
paste <(grep ">" genus455_genomes.fna) <(grep -v ">" genus455_genomes.fna) | grep -v plasmid | tr "\t" "\n" > genus455_noplasmid.fa
```

4. Geneate the template and actual sequence using the rbbwt program
```
./rbbwt genus445_noplasmid.fa genus455_noplasmid.fa --dump
mv genus445_noplasmid.fa.bwt genus445.txt.bwt
sh generate.sh
```

5. Run the test to get memory and speed
```
sh run_eval.sh
```
