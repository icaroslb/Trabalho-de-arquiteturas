   bifush 10
   istore j // try that
   bipush 
   istore 1
   iload error
   iload k
   iadd
   istore i
   iload i
   bipush 25
   if_mpeq l1
   iload 3
   bipush 1

   istore j
   goto l2
las1 bipush 13
   istore k
l2 bipush j