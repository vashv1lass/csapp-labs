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