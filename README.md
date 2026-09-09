## Folders
- bwt_represenation_comparison: scripts for evaluting wavelet tree, RLBWT, RBBWT, and RBBWT-1S. 
- sim: scripts regarding generating simulated data
- SRA: scripts regarding obtaining data sets from SRA
- plot: scripts for drawing the plots, including the scRNA-seq data analysis 

## Running commands
### Build index
Centrifuger:
```
centrifuger-download -o library -P 16 -d bacteria,archaea,virus -p refseq > pv_seqid_to_tid.map # -p flags download protein sequences
find library -type f -name "*.faa.gz" > file.list
cat file.list | xargs -I{} zcat {} | gzip -c > pv_cat.faa.gz
centrifuger-build -r pv_cat.faa.gz --taxonomy-tree ./nodes.dmp --name-table ./names.dmp --conversion-table pv_seqid_to_tid.map -t 16 -o cfr_pv
```

Kaiju:
```
centrifuger-inspect --conversion-table -x cfr_pv > cfr_pv_seq2taxid.map
perl GenerateKaijuFileFromCentrifugerDownloadWithSeqID.pl cfr_pv_seqid2taxid.map library/bacteria library/archaea library/viral > kaiju_protein.faa
kaiju-mkbwt -n 16 -a ACDEFGHIKLMNPQRSTVWY -o kaiju_protein kaiju_protein.faa
kaiju-mkfmi kaiju_protein
```

Kraken2:
```
kraken2-build --download-taxonomy --protein --db kraken2
kraken2-build --no-masking -add-to-library pv_cat.faa -threads 16 --protein --db kraken2
kraken2-build --threads 16 --build --protein --db kraken2
```

### Running command
Refer to the sim/README.md file
