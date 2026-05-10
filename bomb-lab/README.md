# This is an x86-64 bomb for self-study students. 

## I USED LLDB, NOT GDB

### PHASE 1

```bash
(lldb) breakpoint set --name main
(lldb) run
(lldb) next
(lldb) next
(lldb) next
(lldb) next
(lldb) disassemble # see the `phase_1` func address
(lldb) disassemble --address <`phase_1` func address>
```
The second line of the assembly code is preparing the second argument for the function `strings_not_equal` via moving it into %esi register. You need to read the value located at the address that is moved into this register.
```bash
(lldb) memory read <phase 1 password address>
```
You are going to see something like that:
```
<phase 1 password address>:      42 6f 72 64 65 72 20 72 65 6c 61 74 69 6f 6e 73  Border relations
<phase 1 password address + 10>: 20 77 69 74 68 20 43 61 6e 61 64 61 20 68 61 76   with Canada hav
```
You need to read more information.
```bash
(lldb) memory read <phase 1 password address + 20>
```
```
<phase 1 password address + 20>: 65 20 6e 65 76 65 72 20 62 65 65 6e 20 62 65 74  e never been bet
<phase 1 password address + 30>: 74 65 72 2e 00 00 00 00 57 6f 77 21 20 59 6f 75  ter.....Wow! You
```
Look at the last line printed. You see the `00` byte. It means that there is the NULL-terminator of the string. You see that the character is 1-byte data type, so the 5th character is the NULL-terminator and hence the 4th symbol is the last payload symbol.
```bash
(lldb) next
Border relations with Canada have never been better.
(lldb) next
(lldb) next
(lldb) next
Phase 1 defused. How about the next one?
```
So, the answer for the phase 1 is `Border relations with Canada have never been better.`

### PHASE 2

```bash
(lldb) disassemble # see the `phase_2` func address
```
Try to get into the `phase_2` function to figure out what's going on there. You need to write the return address for the `phase_2` function. It's the address of the instruction right after the
```assembly
callq <`phase_2` func address>
```
instruction.
```bash
(lldb) expr $rsp = $rsp - 8 # allocate the space for the return address.
(lldb) memory write -s 8 $rsp <address of the next instruction after `phase_2` call>
(lldb) thread jump --address <`phase_2` func address>
(lldb) disassemble # see the assembly code so as not to get lost
(lldb) step
(lldb) step
(lldb) step
(lldb) step
(lldb) thread jump --address <`phase_2` func address + 14> # step over the "callq" instruction.
(lldb) disassemble
```
You are currently at the
```assembly
cmpl $0x1, (%rsp)
```
instruction. The 'l' instruction suffix means that the data is 4 bytes length. The next two instructions are
```assembly
je     <`phase_2` func address + 52>
callq  <`explode_bomb` func address>
```
It means that the value at the top of the stack must be 0x1 value of 32-bit (4-byte) type. So, write the value 0x1 here and continue.
```bash
(lldb) memory write -s 4 $rsp 0x1
(lldb) step
(lldb) step
```
You've jumped to the <`phase_2` func address + 52> address. You see the
```assembly
leaq   0x4(%rsp), %rbx
leaq   0x18(%rsp), %rbp
```
instructions and the
```assembly
jmp    <`phase_2 func address + 27`>
```
instruction right after.
```bash
(lldb) step
(lldb) step
(lldb) step
```
You've jumped to the <`phase_2` func address + 27> address. Let's analyze instructions that we see here.
```bash
(lldb) disassemble
```
```assembly
movl   -0x4(%rbx), %eax               ; you are here
addl   %eax, %eax
cmpl   %eax, (%rbx)
je     <`phase_2` func address + 41>
callq  <`explode_bomb` func address>
addq   $0x4, %rbx
cmpq   %rbp, %rbx
jne    <`phase_2` func address + 27>  ; the first line of this assembly code
jmp    <`phase_2` func address + 64>
```
Looks like a loop, isn't it? This loop runs over the stack and compares current element %rbx with the previous -0x4(%rbx) element (stored in %eax) multiplied by 2. This loop repeats 5 times (while %rbx is not equal %rbp, since %rbp is 0x18(%rsp)) and compares 6 values. So, every next value must be 2 times greater than the previous. Since the first value is 1, the values are:
```
1 2 4 8 16 32
```
Write such values into the memory
```bash
(lldb) memory write -s 4 `$rsp+4` 0x2
(lldb) memory write -s 4 `$rsp+8` 0x4
(lldb) memory write -s 4 `$rsp+12` 0x8
(lldb) memory write -s 4 `$rsp+16` 0x10
(lldb) memory write -s 4 `$rsp+20` 0x20
```
and continue executing the program.
```bash
(lldb) continue
That's number 2.  Keep going!
```

So, the answer for phase 2 is
```
1 2 4 8 16 32
```