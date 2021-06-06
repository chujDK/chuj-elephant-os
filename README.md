# chuj-elephant-OS

My implement of the operating system in the book 《操作系统真象还原》

# where to run 

you can run in

* [bochs](https://bochs.sourceforge.io/)
* A real IA-32 machine

# making dependence

Only tested under Ubuntu 20.04, with following program installed

* gcc-4.8
* nasm
* dd
* ld


# how to run

if using bochs, simply use the following script

```
./start_bochs.sh
```

directly running in a real machine is not suggested, so, do not do that!

Makefile is provided, simply type `make` to build the disk image

# finished

[MBR](https://www.cjovi.icu/OS/1320.html)

[GDT setup (just a little) and enter Protected Mode](https://www.cjovi.icu/OS/1323.html)

[enter Paging mode](https://www.cjovi.icu/OS/1327.html)

[boot kernel](https://www.cjovi.icu/OS/1330.html)

[sys_putchar and sys_putstr](https://www.cjovi.icu/OS/1336.html)

[interrupt framework](https://www.cjovi.icu/OS/1339.html)

[memory mapping](https://www.cjovi.icu/OS/1349.html)

[thread scheduling](https://www.cjovi.icu/OS/1359.html)

[keyboard IO with buf](https://www.cjovi.icu/OS/1371.html)