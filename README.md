# A small and portable Makefile Generator for C project
###### This project is functionnal but it will be updated soon

## Installation

**Just download this project, place it in your C project directory and execute *install.sh*. For now the makefile generator only support C project whose files are all in the same folder.**

## How to use it

All you need to do is **include the dependences you need into you .c files**.
Here is an example.
Assuming you have two files, *main.c* and *hello.c* .

###### main.c
```c
#include "hello.h"

int main(void){
  printHello();
  exit(1);
}
```

###### hello.c
```c
#include <stdio.h>

void printHello(){
  printf("Hello !\n");
}
```

Once then, all you need to do is execute *generate.sh*, to automatically update your makefile project (which will be overwritten) if it already exsists, or create it if it does not.
