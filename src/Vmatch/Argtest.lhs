This module generates all calls to vmatch combining the following options.

    -tandem
    -supermax
    -mum
    -complete
    -dbnomatch
    -qnomatch
    -dbmaskmatch
    -qmaskmatch
    -dbcluster
    -h
    -e
    -hxdrop
    -exdrop
    -l
    -q
    -online
    -selfun

ADD: -best

>module Main where

>data Matchkind = Substring |
>                 Mum |
>                 Tandem |
>                 Supermax |
>                 Complete deriving (Bounded,Enum)

>instance Show Matchkind where
>  showsPrec _ Substring = showString ""
>  showsPrec _ Tandem = showString "-tandem"
>  showsPrec _ Supermax = showString "-supermax"
>  showsPrec _ Mum = showString "-mum"
>  showsPrec _ Complete = showString "-complete"

>matchkind::[Matchkind]
>matchkind = enumFrom minBound

>data Postprocess = OutputNum |
>                   OutputDesc |
>                   Dbnomatch |
>                   Qnomatch |
>                   Dbmaskmatch |
>                   Qmaskmatch |
>                   Dbcluster |
>                   PPchain |
>                   PPmatchclustererate deriving (Bounded,Enum)

>instance Show Postprocess where
>  showsPrec _ OutputNum = showString ""
>  showsPrec _ OutputDesc = showString "-showdesc 20"
>  showsPrec _ Dbnomatch = showString "-dbnomatch 100"
>  showsPrec _ Qnomatch = showString "-qnomatch 100"
>  showsPrec _ Dbmaskmatch = showString "-dbmaskmatch X"
>  showsPrec _ Qmaskmatch = showString "-qmaskmatch X"
>  showsPrec _ Dbcluster = showString "-dbcluster 50 50"
>  showsPrec _ PPchain = showString "-pp chain local 6b"
>  showsPrec _ PPmatchclustererate 
>    = showString "-pp matchcluster erate 3 outprefix clout"

>postprocess::[Postprocess]
>postprocess = enumFrom minBound

>data Constraint = Exact |
>                  Hamming |
>                  Edit | 
>                  HammingXdrop |
>                  EditXdrop deriving (Bounded,Enum)

>instance Show Constraint where
>  showsPrec _ Exact = showString ""
>  showsPrec _ Hamming = showString "-h ${mindist}"
>  showsPrec _ Edit = showString "-e ${mindist}"
>  showsPrec _ HammingXdrop 
>   = showString "-seedlength ${seedlength} -hxdrop ${mindist}"
>  showsPrec _ EditXdrop 
>   = showString "-seedlength ${seedlength} -exdrop ${mindist}"

>constraint::[Constraint]
>constraint = enumFrom minBound

>data Allmaxvalue = Allmax |
>                   Best |
>                   Noallmax deriving (Bounded,Enum)

>instance Show Allmaxvalue where
>  showsPrec _ Allmax = showString "-allmax"
>  showsPrec _ Best = showString "-best 10"
>  showsPrec _ Noallmax = showString ""

>allmaxvalue::[Allmaxvalue]
>allmaxvalue = enumFrom Allmax

>data Lengthvalue = Length |
>                   Nolength deriving (Bounded,Enum)

>instance Show Lengthvalue where
>  showsPrec _ Length = showString "-l ${minlength}"
>  showsPrec _ Nolength = showString ""

>lengthvalue::[Lengthvalue]
>lengthvalue = enumFrom Length

>data Queryvalue = Self |
>                  Query deriving (Bounded,Enum)

>instance Show Queryvalue where
>  showsPrec _ Self = showString "${database}"
>  showsPrec _ Query = showString "-q ${query} ${database}"

>queryvalue::[Queryvalue]
>queryvalue = enumFrom Self

>data Onlinevalue = Online |
>                   Offline deriving (Bounded,Enum)

>instance Show Onlinevalue where
>  showsPrec _ Online = showString "-online"
>  showsPrec _ Offline = showString ""

>onlinevalue::[Onlinevalue]
>onlinevalue = enumFrom Online

>data Selfun = ApplySelfun |
>              NoSelfun deriving (Bounded,Enum)

>instance Show Selfun where
>  showsPrec _ ApplySelfun = showString "-selfun selnone.so"
>  showsPrec _ NoSelfun = showString ""

>selfunvalue::[Selfun]
>selfunvalue = enumFrom ApplySelfun

>data Direction = DirectionForward |
>                 DirectionReverse deriving (Bounded,Enum)

>instance Show Direction where
>  showsPrec _ DirectionForward = showString "-d"
>  showsPrec _ DirectionReverse = showString "-p"

>directionvalue::[Direction]
>directionvalue = enumFrom DirectionForward

>data Preinfo = Preinfoout |
>               Preinfono deriving (Bounded,Enum)

>instance Show Preinfo where
>  showsPrec _ Preinfoout = showString "-i"
>  showsPrec _ Preinfono = showString ""

>preinfovalue::[Preinfo]
>preinfovalue = enumFrom Preinfoout

>pfppallow::Preinfo->Postprocess->Bool
>pfppallow Preinfoout Dbnomatch = False
>pfppallow Preinfoout Qnomatch = False
>pfppallow Preinfoout Dbmaskmatch = False
>pfppallow Preinfoout Qmaskmatch = False
>pfppallow Preinfoout Dbcluster = False
>pfppallow Preinfoout PPchain = False
>pfppallow Preinfoout PPmatchclustererate = False
>pfppallow Preinfoout OutputDesc = False
>pfppallow _ _ = True

>mkqvallow::Matchkind->Queryvalue->Bool
>mkqvallow Mum Self = False        
>  -- requires special db/query index => exclude it
>mkqvallow Tandem Query = False    
>  -- tandem = repeat => no matching model
>mkqvallow Supermax Query = False  
>  -- supermax = repeat => no matching model
>mkqvallow Complete Self = False
>  -- complete requires queries
>mkqvallow _ _ = True

>qvppallow::Queryvalue->Postprocess->Bool
>qvppallow Self Qnomatch = False
>  -- q refers to query => no self matches
>qvppallow Self Qmaskmatch = False
>  -- q refers to query => no self matches
>qvppallow Query Dbcluster = False
>  -- q refers to query => no self matches => no clustering
>qvppallow _ _ = True

>mkppallow::Matchkind->Postprocess->Bool
>mkppallow Tandem Dbcluster = False
>mkppallow Tandem PPchain = False
>  -- tandem matches are in one sequence => no clustering
>mkppallow _ _ = True

>mklvallow::Matchkind->Lengthvalue->Bool
>mklvallow Tandem Nolength = False
>  -- tandem requires length 
>mklvallow Substring Nolength = False
>  -- substring requires length
>mklvallow Mum Nolength = False
>  -- mum requires length
>mklvallow Supermax Nolength = False
>  -- supermax requires length
>mklvallow Complete Length = False
>  -- complete does not allow length
>mklvallow _ _ = True

>mkcsallow::Matchkind->Constraint->Bool
>mkcsallow Tandem Hamming = False
>  -- tandem is always exact => no hamming distance
>mkcsallow Tandem Edit = False
>  -- tandem is always exact => no edit distance
>mkcsallow Tandem HammingXdrop = False
>  -- tandem is always exact => no hamming distance
>mkcsallow Tandem EditXdrop = False
>  -- tandem is always exact => no edit distance
>mkcsallow Complete HammingXdrop = False
>  -- xdrop is only possible for extension, no complete match
>mkcsallow Complete EditXdrop = False
>  -- xdrop is only possible for extension, no complete match
>mkcsallow _ _ = True

>qvovallow::Queryvalue->Onlinevalue->Bool
>qvovallow Self Online = False
>  -- online available only for query matches
>qvovallow _ _ = True

>mkcsovallow::Matchkind->Constraint->Onlinevalue->Bool
>mkcsovallow Mum _ Online = False
>   -- Mum only offline, because Mums always require to first sort matches
>mkcsovallow _ _ _ = True

>ammkcoallow::Allmaxvalue->Matchkind->Constraint->Bool
>ammkcoallow Allmax Tandem _ = False
>ammkcoallow Allmax Complete _ = False
>ammkcoallow Allmax _ HammingXdrop = False
>ammkcoallow Allmax _ EditXdrop = False
>ammkcoallow Allmax _ Exact = False
>ammkcoallow Noallmax _ Hamming = False
>ammkcoallow Noallmax _ Edit = False
>ammkcoallow Best _ Hamming = False
>ammkcoallow Best _ Edit = False
>ammkcoallow _ _ _ = True

>dimkallow::Direction->Matchkind->Bool
>dimkallow DirectionReverse Tandem = False
>dimkallow DirectionReverse Supermax = False
>dimkallow _ _ = True

>concatcall::[String]->String
>concatcall [] = []
>concatcall [x] = x
>concatcall (x:xs) 
>  | x == ""   = concatcall xs
>  | otherwise = x ++ " " ++ concatcall xs

>completevmatchcall::Direction->
>                    Matchkind->
>                    Postprocess->
>                    Constraint->
>                    Lengthvalue->
>                    Onlinevalue->
>                    Selfun->
>                    Allmaxvalue->
>                    Preinfo->
>                    Queryvalue->
>                    String
>completevmatchcall di mk pp cs lv ov sf am pf qv
>  = concatcall [show di,
>                show mk,
>                show pp,
>                show cs,
>                show lv,
>                show ov,
>                show sf,
>                show am,
>                show pf,
>                show qv]

>vmatchargcombinations::[String]
>vmatchargcombinations
>  = [completevmatchcall di mk pp cs lv ov sf am pf qv | 
>       di<-directionvalue,
>       mk<-matchkind,
>       dimkallow di mk,
>       qv<-queryvalue,
>       mkqvallow mk qv,
>       pp<-postprocess,
>       qvppallow qv pp,
>       mkppallow mk pp,
>       cs<-constraint,
>       mkcsallow mk cs,
>       lv<-lengthvalue,
>       mklvallow mk lv,
>       ov<-onlinevalue,
>       mkcsovallow mk cs ov,
>       qvovallow qv ov,
>       pf<-preinfovalue,
>       pfppallow pf pp,
>       sf<-selfunvalue,
>       am<-allmaxvalue,
>       ammkcoallow am mk cs]

>perlpreface::String
>perlpreface = unlines
>               ["use strict;",
>                "use warnings;\n",
>                "sub makevmatchargumenttable",
>                "{",
>                "  my ($minlength,$mindist,$query,$database) = @_;",
>                "  my $seedlength = $minlength - 2;",
>                "  my @arglisttable =",
>                "  ("]

>perlpostface::[String]
>perlpostface = ["  );",
>                "  return \\@arglisttable;",
>                "}\n",
>                "1;"]

>gencall::String->String
>gencall args = "    \"" ++ args ++ "\","

>vmatchcalls::[String]
>vmatchcalls = perlpreface : (map gencall vmatchargcombinations) ++ perlpostface

>main::IO ()
>main = do {writeFile "Vmcallstab.pm" (unlines vmatchcalls)}
