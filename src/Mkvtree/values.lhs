>prefixlength k n = logBase k (n `div` 8)

>show3digits s | l == 3  = s
>              | l == 2  = "0" ++ s
>              | l == 1  = "00" ++ s   
>                          where l = length s

>num2triplestring n 
>  | n < 1000     = show n
>  | otherwise    = num2triplestring (n `div` 1000) ++ 
>                                               "{,}" ++ show3digits v
>                   where v = show (n `mod` 1000)

>showtrip (a,b,c) = "$" ++ num2triplestring a ++ "-" ++ 
>                          num2triplestring b ++ "$ & " ++ 
>                          show c ++ " \\\\\\hline"

>rangetableDNA = unlines (map showtrip [(4^(i-1) * 8 + 1, 4^i * 8,i) | 
>                                        i<-[1..13]])

>rangetableProtein = unlines (map showtrip [(20^(i-1) * 8 + 1, 20^i * 8,i) | 
>                                        i<-[1..5]])

>outranges = writeFile "tmp" rangetableProtein
