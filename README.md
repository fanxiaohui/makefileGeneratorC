# A small and portable Makefile Generator for C project
###### This project is functionnal but it will be updated soon. Only works on linux.

## Installation

**All you need to do is to clone this repo and recompile .c files in mkgen folder.**

## How to use it

First, you need to recompile MakeFileGenerator.c and makeheaders.c on your own using : 
```BASH
cd mkgen
gcc -c MakeFileGenerator.c ; gcc -c makeheaders.c
gcc -o mkgen MakeFileGenerator.o makeheaders.o
```
All you need from now is to execute ./mkgen/mkgen from your project folder. 
For example, you should execute it from the directory "sampleProject". All .h will be automatically created. Then, just copy the "makefile" file into your project folder and you'll be good to go. 
Each time you need to regenerate .h, launch mkgen. 
The makefile needs to be copied only once into your project folder. 
 
