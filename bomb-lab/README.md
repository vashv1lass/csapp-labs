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

### PHASE 3

An algorithm that you need to perform is almost identical to the phase 2.

```bash
(lldb) disassemble # see the `phase_3` func address
(lldb) expr $rsp = $rsp - 8 # allocate the space for the return address.
(lldb) memory write -s 8 $rsp <address of the next instruction after `phase_3` call>
(lldb) thread jump --address <`phase_3` func address>
(lldb) disassemble # see the assembly code so as not to get lost
```

You see that there are 2 local variables stored on the stack. Also you see that `sscanf` function must return a value greater than 1, so the bomb won't explode.

```assembly
callq  <`__isoc99_scanf` func address>
cmpl   $0x1, %eax
jg     <`phase_3` func address + 39>
callq  <`explode_bomb` func address>
```

Then, you see that the value stored in the first variable is compared with 7, and if it is greater, then bomb explodes, but if it is less or equal, some indirect jump is performed.

```assembly
cmpl   $0x7, 0x8(%rsp)
ja     <`phase_3` func address + 106> ; `explode_bomb` function call is here
movl   0x8(%rsp), %eax
jmpq   *<some_address>(,%rax,8)
```

Let the value of this variable be equal to 7. Lets read the value stored in the address of <some_address>(,%rax,8).

```bash
(lldb) memory read -f x -s 8 `0x0 + $rax * 0x8 + <some_address>`
```

We see the lldb answer like:

```
<some_address+56>:  <`phase_3` func address + 99>  0x737265697564616d
<some_address+72>:  0x6c796276746f666e             0x7420756f79206f53
<some_address+88>:  0x756f79206b6e6968             0x6f7473206e616320
<some_address+104>: 0x6f62206568742070             0x206874697720626d
```

Of all these values, we are interested in the first one -- it's the address of some instruction in `phase_3` function. Let's see it.

```bash
(lldb) disassembly
```

```assembly
movl   $0x147, %eax
jmp    <`phase_3 func address` + 123>
```

If we read the memory at the address of 0x402470, via
```bash
(lldb) memory read -f x -s 8 `0x402470`
```
command, LLDB answers:
```
0x00402470: <`stage_3` func address + 57> <`stage_3` func address + 118>
0x00402480: <`stage_3` func address + 64> <`stage_3` func address + 71>
0x00402490: <`stage_3` func address + 78> <`stage_3` func address + 85>
0x004024a0: <`stage_3` func address + 92> <`stage_3` func address + 99>
```

Looks like a jump table for switch-case... And it is! If we reverse-engineer this switch-case statement, we'll get something like:
```c
if (a > 7) {
    explode_bomb();
}

switch (a) { // a = 0x8(%rsp)
case 0:
    result = 207; // movl   $0xcf, %eax
    break;        // jmp    <`phase_3` func address + 123>
case 2:
    result = 707; // movl   $0x137, %eax
    break;        // jmp    <`phase_3` func address + 123>
case 3:
    result = 256; // movl   $0x100, %eax
    break;        // jmp    <`phase_3` func address + 123>
case 4:
    result = 389; // movl   $0x185, %eax
    break;        // jmp    <`phase_3` func address + 123>
case 5:
    result = 206; // movl   $0xce, %eax
    break;        // jmp    <`phase_3` func address + 123>
case 6:
    result = 682; // movl   $0x2aa, %eax
    break;        // jmp    <`phase_3` func address + 123>
case 7:
    result = 327; // movl   $0x147, %eax
    break;        // jmp    <`phase_3` func address + 123>
default:
    result = 311; // movl   $0x137, %eax
}
```

The `result` is stored in `%eax`. Then it is compared with the value stored in the second variable. If it is equal, stage 3 is defused, if not -- the bomb explodes.

So, there are multiple correct answers. They are:
```
(1) 0 207
(2) 2 707
(3) 3 256
(4) 4 389
(5) 5 206
(6) 6 682
(7) 7 327
(8) 1 311
```

### PHASE 4

```bash
(lldb) disassemble # see the `phase_4` func address
(lldb) disassemble --address <`phase_4` func address>
```

You see that the function is not very complicated, but it calls some `func4` fuction. Let's see, what's going on there:

```bash
(lldb) disassemble --address <`func4` func address>
```

You see, that the function is quite straightforward, except the fact that it is recursive. Let's translate the assembly code into C code to understand more:

```c
// straightforward assembly code translation for `func4`
int func4(int a, int b, int c) {
    int result = c;
    result -= b;
    unsigned int temp = result;
    temp >>= 31;
    result += temp;
    result >>= 1;
    int d = result + b;
    if (d > a) {
        c = d - 1;
        result = func4(a, b, c);
        result *= 2;
    } else {
        result = 0;
        if (d < a) {
            b = d + 1;
            result = func4(a, b, c);
            result = 2 * result + 1;
        }
    }
    return result;
}
```

Let's try to understand this code via trying to determine the purpose of the variables and giving them meaningful names. Also we can try to reduce the code.

```c
int func4(int a, int b, int c) {
    // looks like the distance to the average value between c and b (?)
    // nevermind. dont care what that means.
    int avg_distance;
    if (c > b) {
        avg_distance = c - b;
    } else {
        avg_distance = c - b + 1;
    }
    avg_distance /= 2;

    int d = avg_distance + b;
    if (d > a) {
        return func4(a, b, d - 1) * 2;
    }

    int result = 0;
    if (d < a) {
        return 2 * func4(a, d + 1, c) + 1;
    }

    return 0;
}
```

Let's have a look to `phase_4` function. We see, that `func4` is called like `func4(a, 0, 14)`, where `a` is less than 14 and not negative (it uses the unsigned jump instruction `jbe`, so the negative values will be taken by the program as very large positives) or the bomb explodes. Also the value returned from this function must be 0, the bomb will explode otherwise. So, we must not allow the condition `d < a` be true. If this condition is true, this line of the code
```c
return 2 * func4(a, d + 1, c) + 1;
```
will be executed. It is not allowed, because this expression will make the return value not equal to 0, so the bomb will explode. Let's find out the correct numbers using brute force method. I wrote a simple brute force program in file `phase4_brute_force.c`

```bash
gcc -o phase4_brute_force phase4_brute_force.c
./phase4_brute_force
```

The output is:
```
a=2 is a bad case.
a=4 is a bad case.
a=5 is a bad case.
a=6 is a bad case.
a=8 is a bad case.
a=9 is a bad case.
a=10 is a bad case.
a=11 is a bad case.
a=12 is a bad case.
a=13 is a bad case.
a=14 is a bad case.
```

So, the "good" cases are:
```
a=0
a=1
a=3
a=7
```

Variable `a` is located in the `8(%rsp)` position. The second variable (located in `0xc(%rsp)`) must have the value of 0. It's pretty simple to understand if we see the assembly code of `phase_4` one more time.
```bash
(lldb) disassemble
```
```assembly
cmpl   $0x0, 0xc(%rsp)
je     <`phase_4 epilogue address`>
callq  <`explode_bomb` address>
```
So, there are also multiple correct answers for phase 4. They are:
```
(1) 0 0
(2) 1 0
(3) 3 0
(4) 7 0
```

### PHASE 5

```bash
(lldb) disassemble # see the `phase_5` func address
(lldb) disassemble --address <`phase_5` func address`>
```

In the assembly code of `phase_5` function, we see that it calls functions like `strings_not_equal` and `string_length`, so this phase is about strings.

We also see, that the function `phase_5` uses value from `%edi`/`%rdi` registers, so it takes one argument. If we see the `main` function assembly code, we'll figure out that the value in this register is the address of string we need to enter.

Let's analyze the assembly code for `phase_5`. We see that it checks the length of the string, and if it is not `6`, the bomb will explode. So, the entered string must have the length of `6`.

After this, the execution flow jumps to the <`phase_5` func address + 112> address, zeroes the `%eax` register and jumps to the <`phase_5` func address + 41>. Then the execution flow does something with the
```assembly
movzbl (%rbx,%rax), %ecx
```
value (it's the address of `%rax`-th symbol in argument string since `%rdi` was moved in `%rbx` in line 3 of `phase_5` funciton assembly code) and at the address of <`phase_5` func address + 66> we see that `%rax` register incremets and then it compares with 6, if not equal jumps to <`phase_5` func address + 41>. It's just the iteration over the argument string characters using the `for` loop!

Let's determine what's going on in the body of this loop.
```assembly
movzbl (%rbx,%rax), %ecx
movb   %cl, (%rsp)
movq   (%rsp), %rdx
andl   $0xf, %edx
movzbl <some sample string address>(%rdx), %edx
movb   %dl, 0x10(%rsp,%rax)
```
Let `%rax` be `i` and `%rbx` be `arg`. We see, that the code extracts the `arg[i]` value, stores it at the top of the stack and in the `%rdx` register, cuts everything after the first nibble via `andl $0xf, %edx` instruction.
Then, let's have a look at the address of \<some sample string address\>.
```bash
(lldb) memory read <some sample string address>
```
We see
```
<some sample string address>:      6d 61 64 75 69 65 72 73 6e 66 6f 74 76 62 79 6c  maduiersnfotvbyl
<some sample string address + 10>: 53 6f 20 79 6f 75 20 74 68 69 6e 6b 20 79 6f 75  So you think you
```
Looks like a shuffled string of the length of 15, and the `arg` string must decode that shuffled string to give us an answer for the 5-th phase. If wee see the code after the body of `for` loop, we will see, that thats the correct guess.
```assembly
movb   $0x0, 0x16(%rsp) ; the string is also stored on the stack, thats the null-terminator
movl   <some sample string 2 address>, %esi
leaq   0x10(%rsp), %rdi
callq  <`strings_not_equal` func address>
testl  %eax, %eax
je     <`phase_5` func address + 119> ; go on
callq  <`explode_bomb` func address>
```
It compares that string with some string located at the \<some sample string 2 address\> address. Let's see what's there.
```bash
(lldb) memory read <some sample string 2 address>
```
And we see
```
<some sample string 2 address>:        66 6c 79 65 72 73 00 00 00 00 00 00 00 00 00 00  flyers..........
<some sample string 2 address + 0x10>: 00 00 7c 0f 40 00 00 00 00 00 b9 0f 40 00 00 00  ..|.@.......@...
```
that the correct string is `flyers`. Let's assemble that string using the given symbols at the address of <some sample string address>.

The symbol `'f'` is located at the <some sample string address + 9> address, the symbol `'l'` is located at the <some sample string address + 15> address, the symbol `'y'` is located at the <some sample string address + 14> address,
the symbol `'e'` is located at the <some sample string address + 5> address, the symbol `'r'` is located at the <some sample string address + 6> address, and the symbol `'s'` is located at the <some sample string address + 7> address.

So, the `arg` string needs to have such values as:
```
arg[0] = 9;
arg[1] = 15;
arg[2] = 14;
arg[3] = 5;
arg[4] = 6;
arg[5] = 7;
```
But if we remember, the `for` loop body use the instruction
```assembly
andl   $0xf, %edx
```
to zero the all the bits except the least significant nibble. So, the correct answer is:
```
arg[0] = 9 + 16 * n;
arg[1] = 15 + 16 * n;
arg[2] = 14 + 16 * n;
arg[3] = 5 + 16 * n;
arg[4] = 6 + 16 * n;
arg[5] = 7 + 16 * n;
```
where n is a natural number.
When n=4, if we see the ASCII table, the answer is:
```
arg[0] = 'I'
arg[1] = 'O'
arg[2] = 'N'
arg[3] = 'E'
arg[4] = 'F'
arg[5] = 'G'
```
So, the correct answer is `IONEFG`.

Fun fact: `ionefg` (lower case) is the correct answer too (n=6).

### PHASE 6
```bash
(lldb) disassemble
(lldb) disassemble --address <`phase_6` func address>
```

The function is very complicated, but we see, that it wants us to enter six numbers. The value of the first number is quite easy to understand.
```assembly
subq   $0x50, %rsp
movq   %rsp, %r13
movq   %rsp, %rsi
callq  <`read_six_numbers` func address>
movq   %rsp, %r14
movl   $0x0, %r12d
movq   %r13, %rbp
movl   (%r13), %eax
subl   $0x1, %eax
cmpl   $0x5, %eax
jbe    <`phase_6` func address + 52>
callq  <`explode_bomb` func address>
```
So, the first value may be in the range of [1; 6]. We see, that it is copied into the `%rax` register and then decremented. Then it compares via unsigned comparison (`jbe` instruction) with `5` and if it is "above", the bomb explodes. The zero value is also tracted as "above" since the decrementaion turns `000..00` into `111...11`.

Then, let's have a look to a next part of the disassembled `phase_6` function.
```assembly
<+52>  addl   $0x1, %r12d
<+56>  cmpl   $0x6, %r12d
<+60>  je     <`phase_6` func address + 95>
<+62>  movl   %r12d, %ebx
<+65>  movslq %ebx, %rax
<+68>  movl   (%rsp,%rax,4), %eax
<+71>  cmpl   %eax, (%rbp)
<+74>  jne    <`phase_6` func address + 81>
<+76>  callq  <`explode_bomb` func address>
<+81>  addl   $0x1, %ebx
<+84>  cmpl   $0x5, %ebx
<+87>  jle    <`phase_6` func address + 65>
<+89>  addq   $0x4, %r13
<+93>  jmp    <`phase_6` func address + 32>
```
We see, that it compares every number from 2nd to 6th with the 1st one. The bomb explodes, when at least one of these numbers is equal to 1st.

Let's execute the `phase_6` function instruction-by-instruction.

```bash
(lldb) disassemble # see the `phase_6` function address
(lldb) expression $rsp = $rsp - 8
(lldb) memory write -s 8 $rsp <`phase_6` func address>
(lldb) thread jump --address <`phase_6` func address>
(lldb) stepi # use stepi to execute instruction-by-instruction
```
We see, that ALL of the numbers are compared in pairs via an embedded loop. If one pair contains equal numbers, the bomb explodes. So, now we see, that ALL of the numbers must be different and ALL of them must be from `1` to `6`.

Let's analyze the second part of this phase.
```assembly
<+95>:  leaq   0x18(%rsp), %rsi
<+100>: movq   %r14, %rax
<+103>: movl   $0x7, %ecx
<+108>: movl   %ecx, %edx
<+110>: subl   (%rax), %edx
<+112>: movl   %edx, (%rax)
<+114>: addq   $0x4, %rax
<+118>: cmpq   %rsi, %rax
<+121>: jne    <+108>
```
You see, that it turns every `x` into `7-x` for every entered variable.

Let's analyze the code part from <+130> to <+183>. The code flow starts at <+163>, `%rsi = 0`
```assembly
<+130>: movq   0x8(%rdx), %rdx
<+134>: addl   $0x1, %eax
<+137>: cmpl   %ecx, %eax
<+139>: jne    <+130>
<+141>: jmp    <+148>
<+143>: movl   <some address>, %edx
<+148>: movq   %rdx, 0x20(%rsp,%rsi,2)
<+153>: addq   $0x4, %rsi
<+157>: cmpq   $0x18, %rsi
<+161>: je     <+183>
<+163>: movl   (%rsp,%rsi), %ecx
<+166>: cmpl   $0x1, %ecx
<+169>: jle    <+143>
<+171>: movl   $0x1, %eax
<+176>: movl   <some address>, %edx
<+181>: jmp    <+130>
```
If you take a paper and a pen, think a little bit, execute the program instruction-by-instruction, you will figure out that every value has its pointer analogue:

| 7 - entered value | pointer |
| :--- | :--- |
| 1 | `<some address>` |
| 2 | `(<some address> + 0x8)` |
| 3 | `((<some address> + 0x8) + 0x8)` |
| 4 | `(((<some address> + 0x8) + 0x8) + 0x8)` |
| 5 | `((((<some address> + 0x8) + 0x8) + 0x8) + 0x8)` |
| 6 | `(((((<some address> + 0x8) + 0x8) + 0x8) + 0x8) + 0x8)` |

And this code pushes these pointers in the same order, as the variables (remember that they are currently `7-x`) are located in stack. So, our next task is dereference these pointers. In the following assembly code, `%rbx` is `%rsp+0x20` (the first pointer value), `%rax` is `%rsp+0x28` (the second pointer value) and `%rsi` is `%rsp+50` (the pointer to value that follows 6th), `%rcx` initially is `%rbx`.

```assembly
<+201>: movq   (%rax), %rdx
<+204>: movq   %rdx, 0x8(%rcx)
<+208>: addq   $0x8, %rax
<+212>: cmpq   %rsi, %rax
<+215>: je     <+222>
<+217>: movq   %rdx, %rcx
<+220>: jmp    <+201>
```
This loop just dereferences all of these pointers (makes them like "level 1 pointers" -- dereferencing them will yield its values). Let's see, what values are hiding in these pointers using the `memory read` lldb commands.

| 7 - entered value | pointer | dereferenced pointer (hex, dec)
| :--- | :--- | :--- |
| 1 | `<some address>` | 0x14c, 332
| 2 | `(<some address> + 0x8)` | 0xa8, 168
| 3 | `((<some address> + 0x8) + 0x8)` | 0x39c, 924
| 4 | `(((<some address> + 0x8) + 0x8) + 0x8)` | 0x2b3, 691
| 5 | `((((<some address> + 0x8) + 0x8) + 0x8) + 0x8)` | 0x1dd, 477
| 6 | `(((((<some address> + 0x8) + 0x8) + 0x8) + 0x8) + 0x8)` | 0x1bb, 443

It also just occurred to me that it was a structure with the first field being the entered value, the second field being the dereferenced pointer, and the third field being a pointer itself. Let's go to the next part of the code. I guess our task now is to determine how these values must be ordered. In the following assembly code, `%rdx` is the "level 1" pointer that corresponds the last entered value.

```assembly
<+222>: movq   $0x0, 0x8(%rdx)
<+230>: movl   $0x5, %ebp
<+235>: movq   0x8(%rbx), %rax
<+239>: movl   (%rax), %eax
<+241>: cmpl   %eax, (%rbx)
<+243>: jge    <+250>
<+245>: callq  <`explode_bomb` func address>
<+250>: movq   0x8(%rbx), %rbx
<+254>: subl   $0x1, %ebp
<+257>: jne    <+235>
```

You see, that the loop begins with the first entered value (the corresponding pointer) and goes on to the last. Also we see that the current value is compared with the next one, and it must be greater or equal or the bomb explodes. So, the values must be sorted in non-increasing order. Let's sort the table.

| 7 - entered value | pointer | dereferenced pointer (hex, dec)
| :--- | :--- | :--- |
| 3 | `((<some address> + 0x8) + 0x8)` | 0x39c, 924
| 4 | `(((<some address> + 0x8) + 0x8) + 0x8)` | 0x2b3, 691
| 5 | `((((<some address> + 0x8) + 0x8) + 0x8) + 0x8)` | 0x1dd, 477
| 6 | `(((((<some address> + 0x8) + 0x8) + 0x8) + 0x8) + 0x8)` | 0x1bb, 443
| 1 | `<some address>` | 0x14c, 332
| 2 | `(<some address> + 0x8)` | 0xa8, 168

Or
| entered value | pointer | dereferenced pointer (hex, dec)
| :--- | :--- | :--- |
| 4 | `((<some address> + 0x8) + 0x8)` | 0x39c, 924
| 3 | `(((<some address> + 0x8) + 0x8) + 0x8)` | 0x2b3, 691
| 2 | `((((<some address> + 0x8) + 0x8) + 0x8) + 0x8)` | 0x1dd, 477
| 1 | `(((((<some address> + 0x8) + 0x8) + 0x8) + 0x8) + 0x8)` | 0x1bb, 443
| 6 | `<some address>` | 0x14c, 332
| 5 | `(<some address> + 0x8)` | 0xa8, 168

So, the correct answer for phase 6 is
```
4 3 2 1 6 5
```

### THE SECRET PHASE

Go to phase 1 and try to read memory after the `Border relations...` string.

```bash
(lldb) memory read <phase 1 sample string address>
(lldb) memory read <phase 1 sample string address + 0x20>
(lldb) memory read <phase 1 sample string address + 0x40>
```

You'll get
```
<phase 1 sample string address>:        42 6f 72 64 65 72 20 72 65 6c 61 74 69 6f 6e 73  Border relations
<phase 1 sample string address + 0x10>: 20 77 69 74 68 20 43 61 6e 61 64 61 20 68 61 76   with Canada hav
<phase 1 sample string address + 0x20>: 65 20 6e 65 76 65 72 20 62 65 65 6e 20 62 65 74  e never been bet
<phase 1 sample string address + 0x30>: 74 65 72 2e 00 00 00 00 57 6f 77 21 20 59 6f 75  ter.....Wow! You
<phase 1 sample string address + 0x40>: 27 76 65 20 64 65 66 75 73 65 64 20 74 68 65 20  've defused the 
<phase 1 sample string address + 0x50>: 73 65 63 72 65 74 20 73 74 61 67 65 21 00 66 6c  secret stage!.fl
```

There is a message about some "secret" stage. Let's analyze the assembly code of `main` function.

```bash
(lldb) disassemble
```

There's a function `read_line`. It is a very nice candidate for the "secret stage" because it reads the line from standard input. Let's analyze this function.

```bash
(lldb) disassemble --address <`read_line` function address>
```

There are some instructions that call the C `puts` function. Let's see what do they print.

```assembly
<`read_line` func address> <+35>:  movl   <some address 1>, %edi
<`read_line` func address> <+40>:  callq  <`puts` func address>
```
```bash
(lldb) memory read <some address 1>
```
```
<some address 1>:      45 72 72 6f 72 3a 20 50 72 65 6d 61 74 75 72 65  Error: Premature
<some address 1 + 10>: 20 45 4f 46 20 6f 6e 20 73 74 64 69 6e 00 47 52   EOF on stdin.GR
```
Nah, thats not it. Let's try the second `puts` call.

```assembly
<`read_line` func address> <+109>:  movl   <some address 1>, %edi
<`read_line` func address> <+114>:  callq  <`puts` func address>
```

That's the same address. And the last one.
```assembly
<`read_line` func address> <+182>:  movl   <some address 2>, %edi
<`read_line` func address> <+187>:  callq  <`puts` func address>
```
```bash
(lldb) memory read <some address 2>
```
```
<some address 2>:        45 72 72 6f 72 3a 20 49 6e 70 75 74 20 6c 69 6e  Error: Input lin
<some address 2 + 0x10>: 65 20 74 6f 6f 20 6c 6f 6e 67 00 25 64 20 25 64  e too long.%d %d
```
Aw, that was a mistake. But we also have a `phase_defused` function. Let's disassemble it.
```bash
(lldb) disassemble --address <`phase_defused` func address>
```
And bingo! We see the `secret_phase` function call. Let's determine, how the flow gets into this function by executing `phase_defused` instruction-by-instruction. We see, that it goes there when this condition is true:
```assembly
<+20>:  cmpl   $0x6, 0x202181(%rip)
```

This condition is true, when the 6th stage was defused. Then it calls `sscanf` function, where the first argument is the string we entered in phase 4 (found out by trial and error), checks if we entered 3 input units at this phase (two numbers and a string) and if it is true, the flow goes on:
```assembly
movl   <fmt string address>, %esi ; equals "%d %d %s"
movl   <phase 4 input string address>, %edi
callq  <__isoc99_sscanf>
cmpl   $0x3, %eax
jne    <+113> ; message about the defused bomb
```

Then the entered string is compared with the string at some address and goes on if that string is equal to entered.
```assembly
movl   <that string address>, %esi
leaq   0x10(%rsp), %rdi ; 0x10(%rsp) - entered string
callq  <`strings_not_equal` func address>
testl  %eax, %eax
jne    <+113>
```

Let's see what string is at that address.
```bash
(lldb) memory read <that string address>
```
```
<that string address>:        44 72 45 76 69 6c 00 67 72 65 61 74 77 68 69 74  DrEvil.greatwhit
<that string address + 0x10>: 65 2e 69 63 73 2e 63 73 2e 63 6d 75 2e 65 64 75  e.ics.cs.cmu.edu
```
So, the string is `DrEvil` and to find the secret phase you need to enter in phase 4 one of these strings:
```
0 0 DrEvil
1 0 DrEvil
3 0 DrEvil
7 0 DrEvil
```

Now we know how to activate the secret phase. Let's disassemble it.
```bash
(lldb) disassemble --address <`secret_phase` func address>
```

```assembly
<+0>:  pushq  %rbx
<+1>:  callq  <`read_line` func address>
<+6>:  movl   $0xa, %edx
<+11>: movl   $0x0, %esi
<+16>: movq   %rax, %rdi
<+19>: callq  <`strtol` func address>
<+24>: movq   %rax, %rbx
<+27>: leal   -0x1(%rax), %eax
<+30>: cmpl   $0x3e8, %eax
<+35>: jbe    <+42>
<+37>: callq  <`explode_bomb` func address>
```

In this part of code, we see, that in this phase, we need to input one number. This number will be stored in `%rbx`, In `%rax` will be stored `%rbx-1`. Then, if entered number minus 1 is not in the range of [0; 1000] (because of unsigned `jbe` instruction), so the entered number must be in range [1; 1001] or the bomb explodes. Let's analyze the next part of the disassembled code.

```assembly
<+42>: movl   %ebx, %esi
<+44>: movl   <some address>, %edi
<+49>: callq  <`fun7` func address>
<+54>: cmpl   $0x2, %eax
<+57>: je     <+64>
<+59>: callq  <`explode_bomb` func address>
```

There, we see, that the `secret_phase` function calls some `fun7` function, and if the value returned from it not `2`, then the bomb explodes. There are two arguments passed in `fun7` function: `some_address` and the entered number. So, let's disassemble the `fun7` function.

```bash
(lldb) disassemble --address <`fun7` func address>
```

We see, that `fun7` is pretty similar to `fun4` (see PHASE 4). So, let's reverse-engineer it like in phase 4.

```c
// straightforward assembly code translation into C.
int fun7(void *ptr, int y) {
    if (!ptr) {
        return 0xFFFFFFFF;
    }
    
    int x = *(int *)ptr;
    if (x <= y) {
        if (x == y) {
            return 0;
        }
        return 2 * fun7(ptr + 16, y) + 1;
    }

    return 2 * fun7(ptr + 8, y);
}
```

Let's see, what is going on at ptr.
```bash
(lldb) memory read -s 8 -f x <ptr>
```
We will get something like:
```
<ptr>:        0x0000000000000024 <ptr + 0x20>
<ptr + 0x10>: <ptr + 0x40>       0x0000000000000000
<ptr + 0x20>: 0x0000000000000008 <ptr + 0xa0>
<ptr + 0x30>: <ptr + 0x60>       0x0000000000000000
```

So, at the first call, the value of `x` is 0x24 (36). Let's read some more values at this memory section.

```bash
(lldb) memory read -s 8 -f x -c 58 <ptr> # the value if 58 is also found by trial and error.
```
```
<ptr>:         0x0000000000000024 <ptr + 0x20>
<ptr + 0x10>:  <ptr + 0x40>       0x0000000000000000
<ptr + 0x20>:  0x0000000000000008 <ptr + 0xa0>
<ptr + 0x30>:  <ptr + 0x60>       0x0000000000000000
<ptr + 0x40>:  0x0000000000000032 <ptr + 0x80>
<ptr + 0x50>:  <ptr + 0xc0>       0x0000000000000000
<ptr + 0x60>:  0x0000000000000016 <ptr + 0x180>
<ptr + 0x70>:  <ptr + 0x140>      0x0000000000000000
<ptr + 0x80>:  0x000000000000002d <ptr + 0xe0>
<ptr + 0x90>:  <ptr + 0x1a0>      0x0000000000000000
<ptr + 0xa0>:  0x0000000000000006 <ptr + 0x100>
<ptr + 0xb0>:  <ptr + 0x160>      0x0000000000000000
<ptr + 0xc0>:  0x000000000000006b <ptr + 0x120>
<ptr + 0xd0>:  <ptr + 0x1c0>      0x0000000000000000
<ptr + 0xe0>:  0x0000000000000028 0x0000000000000000
<ptr + 0xf0>:  0x0000000000000000 0x0000000000000000
<ptr + 0x100>: 0x0000000000000001 0x0000000000000000
<ptr + 0x110>: 0x0000000000000000 0x0000000000000000
<ptr + 0x120>: 0x0000000000000063 0x0000000000000000
<ptr + 0x130>: 0x0000000000000000 0x0000000000000000
<ptr + 0x140>: 0x0000000000000023 0x0000000000000000
<ptr + 0x150>: 0x0000000000000000 0x0000000000000000
<ptr + 0x160>: 0x0000000000000007 0x0000000000000000
<ptr + 0x170>: 0x0000000000000000 0x0000000000000000
<ptr + 0x180>: 0x0000000000000014 0x0000000000000000
<ptr + 0x190>: 0x0000000000000000 0x0000000000000000
<ptr + 0x1a0>: 0x000000000000002f 0x0000000000000000
<ptr + 0x1b0>: 0x0000000000000000 0x0000000000000000
<ptr + 0x1c0>: 0x00000000000003e9 0x0000000000000000
```

Let's write a unique dereferenced values in this dump. Other values are not correct because the stack overflows/function returns 0xFFFFFFFF (recursion exit condition is `y == x`).

```
0x0000000000000000 // 0
0x0000000000000001 // 1
0x0000000000000006 // 6
0x0000000000000007 // 7
0x0000000000000008 // 8
0x0000000000000014 // 20
0x0000000000000016 // 22
0x0000000000000023 // 35
0x0000000000000024 // 36
0x0000000000000028 // 40
0x000000000000002d // 45
0x000000000000002f // 47
0x0000000000000032 // 50
0x0000000000000063 // 99
0x000000000000006b // 107
0x00000000000003e9 // 1001
```

And let's just manually brute force them (cuz i have 2 small brain for brute forcing it programmatically lmao + i think that i've translated `fun7` incorrectly + manually is just faster + i'm just so fckn tired). So, the correct answers for the secret stage, are:
```
(1) 20
(2) 22
```