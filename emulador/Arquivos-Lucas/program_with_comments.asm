   bipush 10 // Start here

                //comment line
   istore j
   bipush 10 //comment
   istore k
                    iload j
   iload k
   iadd
   istore i
                iload i
                bipush 25
                if_icmpeq l1
                iload j
   bipush 1
   isub
   istore j
   goto l2 // comment
l1 bipush 13
// Comment line
   istore k
l2 bipush 25
