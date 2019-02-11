s/\"//g
s/\/vol\/biodata\/genomes\/bacteria\///g
s/\/vol\/biodata\/genomes\///g
s/\/vol\/biodata\/est\///g
s/\/vol\/biodata\/swissprot\///g
s/mkvtree -v -protein -pl 4 -db //g
s/mkvtree -lcp -bwt -suf -v -protein -pl 4 -db //g
s/mkvtree -v -dna -pl [0-9] -db //g
s/mkvtree -lcp -bwt -suf -v -dna -pl [0-9] -db //g
s/mkvtree -indexname [a-z]*.list -lcp -bwt -suf -v -dna -pl [0-9] -db //g
s/main=//g
s/secondary=//g
s/\/[0-9A-Za-z\.]*\.fna//g
s/A_thaliana.*CHR_IV/Athaliana/g
s/C_elegans\/sanger\/C_elegans_concat.fna/Celegans/g
s/D_melanogaster.*armX_noN\.fna/Drosophila/g
s/H_sapiens.*hs_chr22\.fna/Human/g
s/S_cerevisiae\/Chr01.*ChrMT/Scerevisiae/g
s/_//g
