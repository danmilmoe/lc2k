          lw    0   1   mcand       reg1 = 6203
          lw    0   2   mplier      reg2 = 1429
          lw    0   6   addr        load address of func in reg 6
          jalr  6   7               call func
          add   0   3   1           copy the func result from reg3 to reg1
          halt                    
          noop 
          noop                      Function func
func      noop                      multiply reg1 with reg2 and store result in reg3 
          noop                      return address in reg7
          noop
          lw    0   4  minus2       mask = -2
          lw    0   5  iter         number of iteration = 16
          lw    0   3  zero         sum = 0
loop      noop
          nor   4   2   6           reg6 = ~(reg4 | reg2). If 6 == 0, then ith bit is set. So add reg1 to sum 
          beq   0   6   add         
          beq   0   0   skipadd     
add       add   3   1   3            
skipadd   add   1   1   1           left shift reg1 
          lw    0   6   posone      below 3 instructions - reg4 = (reg4 << 1) + 1. Moves the zero bit to left most position
          add   4   4   4           
          add   4   6   4           
          lw    0   6   minus1      decrement iter by 1
          add   5   6   5
          beq   0   5   return      if iter == 0, then return from function 
          beq   0   0   loop        continue loop
return    jalr  7   6
addr     .fill    func
iter     .fill    16
zero     .fill    0
posone   .fill    1
minus2   .fill   -2
minus1   .fill   -1
mcand    .fill   6203
mplier   .fill   1429