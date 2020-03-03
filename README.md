# CS153 Lab 3 (Changing Memory Layout and Growing the Stack)
## Lab 3 - Changing Memory Layout and Growing the Stack
### a) Changing Memory Layout

Currently the xv6 address space is set up as: CODE --> STACK --> HEAP, where the stack is fixed in size
and the heap grows toward the high-end of the address space. We are going to change it where the address
space is: CODE --> HEAP --> STACK. 

In order make this change we need to alter the way xv6 loads the program into memory and sets up the page
table. This happens in the file, `exec.c` within the `exec(char*, char**)` function. Previously xv6 would allocate two pages with a fixed page size and a second page meant to act as a guard page. This guard page would prevent the stack from growing into the heap by giving the program a memory error to stop it. Since we want the stack to grow down we must now change where the stack is allocated. In the following picture we have made changes in the section of `exec.c` that handles the allocation of the stack.

**EXEC.cPhoto1**

In order to change it we created a new `uint` variable called `stacksz` that will handle the location of where the stack will be allocated. Previously `sz` was set to `allocuvm()` to ensure that the stack was created at the boundaries of the user code and the heap. To change this we instead used `uint stacksz = KERNBASE - PGSIZE` in order to create the stack above the heap and below the kernel. We then changed the second and third parameter to `stacksz` and `stacksz+1` in order to allocate the first and last page of our pagetable respectively. We want the first page to point to the top of user memory thus being under `KERNBASE` and the last page to be anything larger than the first page since they are virtual addresses. Next we commented out `clearpreu()` since we no longer need a guard page as we want the stack to be able to grow. Finally we will set our `sp` to the top word in the stack page, so we will set it to our own created address, `KERNBASE2`, which is defined in the following picture.

**KERNBASE2**

### b) Helper Functions and copyuvm()

Since we have moved the stack, a few places in the Kernel that hard coded the previous location of the stack need to be changed. These include all the functions that are defined in `syscall.c` and `sysfile.c`. 
The functions we have changed include:

```
fetchint()
fetchstr()
argptr()
```

The following pictures depict the changes we made to the previously mentioned functions. 

aa
aa
aa

Within all the functions we made changes to all calls that deal with the previous location of the stack. Anywhere the helper functions made calls to `curproc->sz` has been changed to `KERNBASE2` to match the location of our new stack. In addition any call that checked whether an address was out of the stack's set bounds has been changed to compare the address to `KERNBASE2`.

Next we have to make changes to the file `vm.c` in our function `copyuvm()` . This function is used in `fork()` in order to create a copy of the address space of the parent process calling fork to the child process. Previously in only iterated through the bottom of the address space, 0, to `sz`. This worked when the stack was a fixed size but since it now grows we need to make sure we take into account for the number of stack pages. In order to do that we will create a variable within our `struct proc` called `uint stacksz`. We will also now pass in `uint stacksz` in our `copyuvm()` function and thus change all subsequent calls of `copyuvm()` to pass in `curproc()->stacksz`. Finally we need to now iterate through the new stack pages. We do this by creating a similar loop but instead iterate from `KERNBASE2 - PGSIZE + 1` and decrement both `PGSIZE` and `stacksz` until our `stacksz = 0`. This will iterate through the top of the user space down to the current process space. The following pictures show the implementation. 

aa
aa
aa

### c) Page faults

Finally we need to take into account of page faults with our growing stack. In order to do that we need to implement our own case for page faults in `trap.c`. In `traps.h` are the defined trap cases. We can see that trap 14 is our set trap for faults. Now that we know that we now need to implement a new trap fault in `trap.c`. In the following photo you can see the case we added which gives us our new page fault.

aaa

To implement the page fault case we checked the address of the faulted page using the `rcr2()` function which gives us the CR2 register. After defining it we then check if the fault address is greater than our `KERNBASE2 - (PGSIZE*numPages) + 1`. After doing so we then check if allocuvm failed at the fault address and the fault address + 1.

### d) Testbench












xv6 is a re-implementation of Dennis Ritchie's and Ken Thompson's Unix
Version 6 (v6).  xv6 loosely follows the structure and style of v6,
but is implemented for a modern x86-based multiprocessor using ANSI C.

ACKNOWLEDGMENTS

xv6 is inspired by John Lions's Commentary on UNIX 6th Edition (Peer
to Peer Communications; ISBN: 1-57398-013-7; 1st edition (June 14,
2000)). See also http://pdos.csail.mit.edu/6.828/2016/xv6.html, which
provides pointers to on-line resources for v6.

xv6 borrows code from the following sources:
    JOS (asm.h, elf.h, mmu.h, bootasm.S, ide.c, console.c, and others)
    Plan 9 (entryother.S, mp.h, mp.c, lapic.c)
    FreeBSD (ioapic.c)
    NetBSD (console.c)

The following people have made contributions: Russ Cox (context switching,
locking), Cliff Frey (MP), Xiao Yu (MP), Nickolai Zeldovich, and Austin
Clements.

We are also grateful for the bug reports and patches contributed by Silas
Boyd-Wickizer, Anton Burtsev, Cody Cutler, Mike CAT, Tej Chajed, Nelson Elhage,
Saar Ettinger, Alice Ferrazzi, Nathaniel Filardo, Peter Froehlich, Yakir Goaron,
Shivam Handa, Bryan Henry, Jim Huang, Alexander Kapshuk, Anders Kaseorg,
kehao95, Wolfgang Keller, Eddie Kohler, Austin Liew, Imbar Marinescu, Yandong
Mao, Hitoshi Mitake, Carmi Merimovich, Joel Nider, Greg Price, Ayan Shafqat,
Eldar Sehayek, Yongming Shen, Cam Tenny, Rafael Ubal, Warren Toomey, Stephen Tu,
Pablo Ventura, Xi Wang, Keiichi Watanabe, Nicolas Wolovick, Grant Wu, Jindong
Zhang, Icenowy Zheng, and Zou Chang Wei.

The code in the files that constitute xv6 is
Copyright 2006-2016 Frans Kaashoek, Robert Morris, and Russ Cox.

ERROR REPORTS

Please send errors and suggestions to Frans Kaashoek and Robert Morris
(kaashoek,rtm@mit.edu). The main purpose of xv6 is as a teaching
operating system for MIT's 6.828, so we are more interested in
simplifications and clarifications than new features.

BUILDING AND RUNNING XV6

To build xv6 on an x86 ELF machine (like Linux or FreeBSD), run
"make". On non-x86 or non-ELF machines (like OS X, even on x86), you
will need to install a cross-compiler gcc suite capable of producing
x86 ELF binaries. See http://pdos.csail.mit.edu/6.828/2016/tools.html.
Then run "make TOOLPREFIX=i386-jos-elf-". Now install the QEMU PC
simulator and run "make qemu".



