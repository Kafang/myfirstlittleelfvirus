# My first little Virus
I had some spare time at hand and thus decided to deepen my knowledge about the ELF file format.
And since Learning by Breaking is my favourite method of learning, I decided to write my very first virus.
The challenge looked like this: I have to finish the project in one day, I may only use the manpages and 
other offline ressources (no spoilers and I have to solve issues my own way), the virus should be written in c, and the result has to be a working POC in order to pass the challenge.


## The idea:
My idea was relatively simple, it consisted of the following steps:
- Iterate the programheaders until an executable segment is found
- Increase the size of this segment on disc
- Increase the size of this segment in the corresponding header
- Insert the viruscode into the file
- Update the elf entrypoint to point to the original entrypoint
- Add a call to the original entry point into the file
- Done... And they all lived happily ever after

## Compiling 
You can compile the little virus like this:
```
gcc virus.c -nostdlib -fno-stack-protector -pie -o virus
```
I made use of a lot of very hacky undefined behaviour, which means, that this project might only work with my gcc version, which was 11.2

## Running
The virus will only infect the file a.out (which has to be an elf file), and he will not check if this file has been infected already.
Thus in order to play around, it is necessary to infect an a.out and rename it to something different and let this renamed file infect another a.out

## TODO:
- IMPORTANT: the registers might contain stuff, that is needed by the original entry, which makes us crash...
- Scan for other elf files than only a.out
- Add a mechanism to check if the file is already infected 
- Check if an elf file is a x86_64 binary before infecting it.

## DISCLAIMER
- Learn, have fun and don't do stupid things!
- Stupid things include: running this in production, running this in on your friends pc without his or her explicit consent, running this at all.

