\expandafter\ifx\csname graph\endcsname\relax
   \csname newbox\expandafter\endcsname\csname graph\endcsname
\fi
\ifx\graphtemp\undefined
  \csname newdimen\endcsname\graphtemp
\fi
\expandafter\setbox\csname graph\endcsname
 =\vtop{\vskip 0pt\hbox{%
    \special{pn 8}%
    \special{pa 125 7000}%
    \special{pa 1625 7000}%
    \special{pa 1625 6375}%
    \special{pa 125 6375}%
    \special{pa 125 7000}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 6.688in
    \rlap{\kern 0.875in\lower\graphtemp\hbox to 0pt{\hss masked\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 6.688in
    \rlap{\kern 0.875in\lower\graphtemp\hbox to 0pt{\hss sequences\hss}}%
    \special{pa 1875 7000}%
    \special{pa 3375 7000}%
    \special{pa 3375 6375}%
    \special{pa 1875 6375}%
    \special{pa 1875 7000}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 6.688in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss uncovered\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 6.688in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss sequences\hss}}%
    \special{pa 125 6250}%
    \special{pa 1625 6250}%
    \special{pa 1625 5625}%
    \special{pa 125 5625}%
    \special{pa 125 6250}%
    \special{fp}%
    \graphtemp=.5ex
    \advance\graphtemp by 5.938in
    \rlap{\kern 0.875in\lower\graphtemp\hbox to 0pt{\hss alignments\hss}}%
    \special{pa 3750 7000}%
    \special{pa 5250 7000}%
    \special{pa 5250 6375}%
    \special{pa 3750 6375}%
    \special{pa 3750 7000}%
    \special{fp}%
    \graphtemp=.5ex
    \advance\graphtemp by 6.688in
    \rlap{\kern 4.500in\lower\graphtemp\hbox to 0pt{\hss alignments\hss}}%
    \special{pa 5500 7000}%
    \special{pa 7000 7000}%
    \special{pa 7000 6375}%
    \special{pa 5500 6375}%
    \special{pa 5500 7000}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 6.688in
    \rlap{\kern 6.250in\lower\graphtemp\hbox to 0pt{\hss masked\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 6.688in
    \rlap{\kern 6.250in\lower\graphtemp\hbox to 0pt{\hss sequences\hss}}%
    \special{pa 5500 6250}%
    \special{pa 7000 6250}%
    \special{pa 7000 5625}%
    \special{pa 5500 5625}%
    \special{pa 5500 6250}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 5.938in
    \rlap{\kern 6.250in\lower\graphtemp\hbox to 0pt{\hss unique\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 5.938in
    \rlap{\kern 6.250in\lower\graphtemp\hbox to 0pt{\hss substrings\hss}}%
    \special{pa 5500 5500}%
    \special{pa 7000 5500}%
    \special{pa 7000 4875}%
    \special{pa 5500 4875}%
    \special{pa 5500 5500}%
    \special{fp}%
    \graphtemp=.5ex
    \advance\graphtemp by 5.188in
    \rlap{\kern 6.250in\lower\graphtemp\hbox to 0pt{\hss clusters\hss}}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 2650 6275}%
    \special{pa 2625 6375}%
    \special{pa 2600 6275}%
    \special{pa 2650 6275}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 2625 6275}%
    \special{pa 2625 6000}%
    \special{fp}%
    \special{sh 0.500}%
    \special{ar 2625 5938 63 63 0 6.28319}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 2650 5775}%
    \special{pa 2625 5875}%
    \special{pa 2600 5775}%
    \special{pa 2650 5775}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 2625 5775}%
    \special{pa 2625 5500}%
    \special{fp}%
    \special{ar 2625 5188 750 312 0 6.28319}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 5.188in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss selection\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 5.188in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss function\hss}}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 4525 6275}%
    \special{pa 4500 6375}%
    \special{pa 4475 6275}%
    \special{pa 4525 6275}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 4500 6275}%
    \special{pa 4500 6000}%
    \special{fp}%
    \special{sh 0.500}%
    \special{ar 4500 5938 63 63 0 6.28319}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 4525 5775}%
    \special{pa 4500 5875}%
    \special{pa 4475 5775}%
    \special{pa 4525 5775}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 4500 5775}%
    \special{pa 4500 5500}%
    \special{fp}%
    \special{ar 4500 5188 750 312 0 6.28319}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 5.188in
    \rlap{\kern 4.500in\lower\graphtemp\hbox to 0pt{\hss selection\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 5.188in
    \rlap{\kern 4.500in\lower\graphtemp\hbox to 0pt{\hss function\hss}}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 2650 4775}%
    \special{pa 2625 4875}%
    \special{pa 2600 4775}%
    \special{pa 2650 4775}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 2625 4775}%
    \special{pa 2625 4625}%
    \special{fp}%
    \special{pa 1875 4625}%
    \special{pa 3375 4625}%
    \special{pa 3375 4000}%
    \special{pa 1875 4000}%
    \special{pa 1875 4625}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 4.312in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss maximal\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 4.312in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss matches\hss}}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 4525 4775}%
    \special{pa 4500 4875}%
    \special{pa 4475 4775}%
    \special{pa 4525 4775}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 4500 4775}%
    \special{pa 4500 4625}%
    \special{fp}%
    \special{pa 3750 4625}%
    \special{pa 5250 4625}%
    \special{pa 5250 4000}%
    \special{pa 3750 4000}%
    \special{pa 3750 4625}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 4.312in
    \rlap{\kern 4.500in\lower\graphtemp\hbox to 0pt{\hss maximal\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 4.312in
    \rlap{\kern 4.500in\lower\graphtemp\hbox to 0pt{\hss repeats\hss}}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 2650 3900}%
    \special{pa 2625 4000}%
    \special{pa 2600 3900}%
    \special{pa 2650 3900}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 2625 3900}%
    \special{pa 2625 3750}%
    \special{fp}%
    \special{ar 2625 3438 750 312 0 6.28319}%
    \graphtemp=.5ex
    \advance\graphtemp by 3.438in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss vmatch\hss}}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 2650 3025}%
    \special{pa 2625 3125}%
    \special{pa 2600 3025}%
    \special{pa 2650 3025}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 2625 3025}%
    \special{pa 2625 2750}%
    \special{fp}%
    \special{pa 1875 2750}%
    \special{pa 3375 2750}%
    \special{pa 3375 1750}%
    \special{pa 1875 1750}%
    \special{pa 1875 2750}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -2
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 2.250in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss index\hss}}%
    \graphtemp=.5ex
    \advance\graphtemp by 2.250in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss (enhanced\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 2
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 2.250in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss suffix array)\hss}}%
    \special{ar 4500 3438 750 312 0 6.28319}%
    \graphtemp=.5ex
    \advance\graphtemp by 3.438in
    \rlap{\kern 4.500in\lower\graphtemp\hbox to 0pt{\hss vmatch\hss}}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 4525 3900}%
    \special{pa 4500 4000}%
    \special{pa 4475 3900}%
    \special{pa 4525 3900}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 4500 3900}%
    \special{pa 4500 3750}%
    \special{fp}%
    \special{ar 4500 2562 750 312 0 6.28319}%
    \graphtemp=.5ex
    \advance\graphtemp by 2.562in
    \rlap{\kern 4.500in\lower\graphtemp\hbox to 0pt{\hss vsubseqselect\hss}}%
    \special{ar 4500 1812 750 312 0 6.28319}%
    \graphtemp=.5ex
    \advance\graphtemp by 1.812in
    \rlap{\kern 4.500in\lower\graphtemp\hbox to 0pt{\hss vseqselect\hss}}%
    \special{ar 4500 1063 750 312 0 6.28319}%
    \graphtemp=.5ex
    \advance\graphtemp by 1.063in
    \rlap{\kern 4.500in\lower\graphtemp\hbox to 0pt{\hss vstree2tex\hss}}%
    \special{ar 4500 313 750 312 0 6.28319}%
    \graphtemp=.5ex
    \advance\graphtemp by 0.313in
    \rlap{\kern 4.500in\lower\graphtemp\hbox to 0pt{\hss vseqinfo\hss}}%
    \special{pa 5625 625}%
    \special{pa 7125 625}%
    \special{pa 7125 0}%
    \special{pa 5625 0}%
    \special{pa 5625 625}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 0.313in
    \rlap{\kern 6.375in\lower\graphtemp\hbox to 0pt{\hss sequence\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 0.313in
    \rlap{\kern 6.375in\lower\graphtemp\hbox to 0pt{\hss information\hss}}%
    \special{pa 5625 1375}%
    \special{pa 7125 1375}%
    \special{pa 7125 750}%
    \special{pa 5625 750}%
    \special{pa 5625 1375}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 1.063in
    \rlap{\kern 6.375in\lower\graphtemp\hbox to 0pt{\hss latex\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 1.063in
    \rlap{\kern 6.375in\lower\graphtemp\hbox to 0pt{\hss format\hss}}%
    \special{pa 5625 2125}%
    \special{pa 7125 2125}%
    \special{pa 7125 1500}%
    \special{pa 5625 1500}%
    \special{pa 5625 2125}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 1.812in
    \rlap{\kern 6.375in\lower\graphtemp\hbox to 0pt{\hss selected\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 1.812in
    \rlap{\kern 6.375in\lower\graphtemp\hbox to 0pt{\hss sequences\hss}}%
    \special{pa 5625 2875}%
    \special{pa 7125 2875}%
    \special{pa 7125 2250}%
    \special{pa 5625 2250}%
    \special{pa 5625 2875}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 2.562in
    \rlap{\kern 6.375in\lower\graphtemp\hbox to 0pt{\hss selected\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 2.562in
    \rlap{\kern 6.375in\lower\graphtemp\hbox to 0pt{\hss subsequences\hss}}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 2650 1650}%
    \special{pa 2625 1750}%
    \special{pa 2600 1650}%
    \special{pa 2650 1650}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 2625 1650}%
    \special{pa 2625 1500}%
    \special{fp}%
    \special{ar 2625 1188 750 312 0 6.28319}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 1.188in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss mkvtree\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 1.188in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss mkdna6idx\hss}}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 2650 775}%
    \special{pa 2625 875}%
    \special{pa 2600 775}%
    \special{pa 2650 775}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 2625 775}%
    \special{pa 2625 625}%
    \special{fp}%
    \special{pa 1875 625}%
    \special{pa 3375 625}%
    \special{pa 3375 0}%
    \special{pa 1875 0}%
    \special{pa 1875 625}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 0.312in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss alphabet\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 0.312in
    \rlap{\kern 2.625in\lower\graphtemp\hbox to 0pt{\hss transformation\hss}}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 1775 1163}%
    \special{pa 1875 1188}%
    \special{pa 1775 1212}%
    \special{pa 1775 1163}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 1775 1188}%
    \special{pa 1625 1188}%
    \special{fp}%
    \special{pa 125 1688}%
    \special{pa 1625 1688}%
    \special{pa 1625 687}%
    \special{pa 125 687}%
    \special{pa 125 1688}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 1.188in
    \rlap{\kern 0.875in\lower\graphtemp\hbox to 0pt{\hss database\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 1.188in
    \rlap{\kern 0.875in\lower\graphtemp\hbox to 0pt{\hss sequences\hss}}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 1775 3412}%
    \special{pa 1875 3438}%
    \special{pa 1775 3462}%
    \special{pa 1775 3412}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 1775 3438}%
    \special{pa 1500 3438}%
    \special{fp}%
    \special{pa 0 3937}%
    \special{pa 1500 3937}%
    \special{pa 1500 2938}%
    \special{pa 0 2938}%
    \special{pa 0 3937}%
    \special{fp}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by -1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 3.438in
    \rlap{\kern 0.750in\lower\graphtemp\hbox to 0pt{\hss query\hss}}%
    \graphtemp=\baselineskip
    \multiply\graphtemp by 1
    \divide\graphtemp by 2
    \advance\graphtemp by .5ex
    \advance\graphtemp by 3.438in
    \rlap{\kern 0.750in\lower\graphtemp\hbox to 0pt{\hss sequences\hss}}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 1725 5963}%
    \special{pa 1625 5938}%
    \special{pa 1725 5913}%
    \special{pa 1725 5963}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 1725 5938}%
    \special{pa 2563 5938}%
    \special{fp}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 1726 6355}%
    \special{pa 1625 6375}%
    \special{pa 1705 6310}%
    \special{pa 1726 6355}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 1716 6333}%
    \special{pa 2563 5938}%
    \special{fp}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 5400 5913}%
    \special{pa 5500 5938}%
    \special{pa 5400 5963}%
    \special{pa 5400 5913}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 5400 5938}%
    \special{pa 4563 5938}%
    \special{fp}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 5420 6310}%
    \special{pa 5500 6375}%
    \special{pa 5399 6355}%
    \special{pa 5420 6310}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 5409 6333}%
    \special{pa 4563 5938}%
    \special{fp}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 5406 5230}%
    \special{pa 5500 5188}%
    \special{pa 5438 5269}%
    \special{pa 5406 5230}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 5422 5250}%
    \special{pa 4563 5938}%
    \special{fp}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 3883 3160}%
    \special{pa 3970 3217}%
    \special{pa 3867 3207}%
    \special{pa 3883 3160}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 3875 3184}%
    \special{pa 2625 2750}%
    \special{fp}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 3689 2479}%
    \special{pa 3750 2562}%
    \special{pa 3657 2518}%
    \special{pa 3689 2479}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 3673 2498}%
    \special{pa 3375 2250}%
    \special{fp}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 3666 1872}%
    \special{pa 3750 1812}%
    \special{pa 3704 1905}%
    \special{pa 3666 1872}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 3685 1888}%
    \special{pa 3375 2250}%
    \special{fp}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 3696 1150}%
    \special{pa 3750 1063}%
    \special{pa 3744 1165}%
    \special{pa 3696 1150}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 3720 1158}%
    \special{pa 3375 2250}%
    \special{fp}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 3706 406}%
    \special{pa 3750 313}%
    \special{pa 3756 415}%
    \special{pa 3706 406}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 3731 411}%
    \special{pa 3375 2250}%
    \special{fp}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 5525 288}%
    \special{pa 5625 313}%
    \special{pa 5525 338}%
    \special{pa 5525 288}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 5525 313}%
    \special{pa 5250 313}%
    \special{fp}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 5525 1038}%
    \special{pa 5625 1063}%
    \special{pa 5525 1088}%
    \special{pa 5525 1038}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 5525 1063}%
    \special{pa 5250 1063}%
    \special{fp}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 5525 1787}%
    \special{pa 5625 1812}%
    \special{pa 5525 1837}%
    \special{pa 5525 1787}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 5525 1812}%
    \special{pa 5250 1812}%
    \special{fp}%
    \special{sh 1.000}%
    \special{pn 1}%
    \special{pa 5525 2537}%
    \special{pa 5625 2562}%
    \special{pa 5525 2587}%
    \special{pa 5525 2537}%
    \special{fp}%
    \special{pn 8}%
    \special{pa 5525 2562}%
    \special{pa 5250 2562}%
    \special{fp}%
    \hbox{\vrule depth7.000in width0pt height 0pt}%
    \kern 7.125in
  }%
}%
