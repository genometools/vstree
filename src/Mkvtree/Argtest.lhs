>module Main where

>import List

>db = [s ++ " -db $DBdna" | s<-[" -dna"," -smap TransDNA"]] ++
>     [s ++ " -db $DBprot"  | s<-[" -protein"," -smap TransProt11"]]
>plen = [s ++ " -pl " ++ show i | 
>        s<-[" -bck -sti1", " -bck"," "], i<-[1,2]]

>calls::[String]
>calls = ["use strict;",
>         "use warnings;\n",
>         "sub makemkvtreeargumenttable",
>         "{",
>         "  my($DBdna,$DBprot) = @_;",
>         "  my @arglisttable =",
>         "  ("] ++
>        [gencall (d ++ p ++ tis ++ ois ++ suf ++ bwt) | 
>         d<-db,
>         p<-plen,
>         tis<-[" -tis"," "],
>         ois<-[" -ois"," "],
>         suf<-[s ++ " -suf -lcp" | s<-[" -skp"," "]] ++ [" "],
>         bwt<-[" -bwt"," "]
>         ] ++
>         ["  );",
>          "  return \\@arglisttable;",
>          "}\n",
>          "1;"]

>gencall s = "   \"" ++ s ++ "\","

>main::IO ()
>main = writeFile "Mkvcallstab.pm" (unlines calls)
