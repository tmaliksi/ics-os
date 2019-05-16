/*

  Name: DEX-OS 1.0 Beta Kernel Main file
  Copyright: 
  Author: Joseph Emmanuel Dayo
  Date: 13/03/04 06:20
  Description: This is the kernel main file that gets called after startup.asm.
  
   
    DEX educational extensible operating system 1.0 Beta
    Copyright (C) 2004  Joseph Emmanuel DL Dayo

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. 
*/

#define NULL 0
#define DEBUGX
#define USE_CONSOLEDDL


/*some defines that are used for debugging purposes*/

#define DEBUG_FLUSHMGR
//#define DEBUG_COFF
//#define DDL_DEBUG
//#define DEBUG_FORK
#define FULLSCREENERROR
//#define DEBUG_KSBRK
#define DEBUG_FAT12
#define DEBUG_STARTUP
#define DEBUG_EXTENSION
//#define DEBUG_USER_PROCESS
//#define MODULE_DEBUG
//#define DEBUG_PEMODULE
//#define DEBUG_VFSREAD
//#define DEBUG_MEMORY
//#define USE_DIRECTFLOPPY
//#define DEBUG_VFS
//#define DEBUG_IOREADWRITE
//#define DEBUG_IOREADWRITE2
//#define WRITE_DEBUG2
//#define WRITE_DEBUG
//#define DEBUG_READ
//#define DEBUG_BRIDGE



//timer set to switch to new task (see time.h or time.c)
int context_switch_rate=100; 

//pointer to vga mem
char *scr_debug = (char*)0xb8000;

int op_success;

//points to the location of the multiboot header defined in startup.asm
extern int multiboothdr; 

//defined in asmlib.asm
extern void textcolor(unsigned char c);


//order is important for some include files, DO NOT CHANGE!
#include <stdarg.h>
#include <limits.h>
#include <math.h>

#include "build.h"
#include "version.h"
#include "dextypes.h"
#include "process/sync.h"
#include "stdlib/time.h"
#include "stdlib/dexstdlib.h"
#include "startup/multiboot.h"
#include "memory/dexmem.h"
#include "console/dex_DDL.h"
#include "vfs/vfs_core.h"
#include "process/process.h"
#include "process/pdispatch.h"
#include "devmgr/dex32_devmgr.h"
#include "devmgr/devmgr_error.h"
#include "console/dexio.h"
#include "hardware/keyboard/keyboard.h"
#include "hardware/keyboard/mouse.h"
#include "hardware/hardware.h"
#include "memory/kheap.h"
#include "hardware/chips/ports.c"
#include "hardware/vga/dexvga.c"
#include "stdlib/qsort.c"
#include "hardware/floppy/floppy.h"
#include "hardware/ATA/ataio.h"
#include "hardware/exceptions.h"
#include "hardware/chips/speaker.h"
#include "dexapi/dex32API.h"
#include "filesystem/fat12.h"
#include "filesystem/iso9660.h"
#include "filesystem/devfs.h"
#include "process/event.h"
#include "devmgr/extensions.h"
#include "process/environment.h"
#include "console/foreground.h"
#include "console/console.h"
#include "stdlib/stdlib.h"
#include "devmgr/bridges.h"
#include "process/scheduler.h"
#include "console/script.h"
#include "vfs/vfs_aux.h"
#include "iomgr/iosched.h"

//structure to hold the boot info
typedef struct _kernel_sysinfo {
   int boot_device;
	int part[3];
} kernel_sysinfo;

kernel_sysinfo kernel_systeminfo;

//This stores the current virtual console the kernel will use
DEX32_DDL_INFO *consoleDDL;

//forward declarations.needed in process.c so must be here first 
void dex_init();

/*I know there are some disadvantages to directly including files
  in the source code instead of using object files, but it simplifies
  compilation without the use of a makefile*/

#include "console/dex_DDL.c"
#include "hardware/dexapm.c"
#include "hardware/chips/irqhandlers.c"
#include "memory/dlmalloc.c"
#include "memory/bsdmallo.c"
#include "stdlib/time.c"
#include "hardware/floppy/floppy.c"
#include "vfs/vfs_core.c"
#include "module/module.c"
#include "process/pdispatch.c"
#include "console/console.c"
#include "console/dexio.c"
#include "stdlib/stdlib.c"
#include "process/dex_taskmgr.c"
#include "hardware/keyboard/keyboard.c"
#include "hardware/keyboard/mouse.c"
#include "hardware/pcibus/dexpci2.c"
//#include "hardware/pcibus/i386-ports.c"
//#include "hardware/pcibus/access.c"
//#include "hardware/pcibus/generic.c"
//#include "hardware/pcibus/jachpci.c"
#include "hardware/exceptions.c"
#include "hardware/hardware.c"
#include "hardware/chips/speaker.c"
#include "devmgr/dex32_devmgr.c"
#include "devmgr/extension.c"
#include "process/environment.c"
#include "console/foreground.c"
#include "devmgr/bridges.c"
#include "process/sync.c"
#include "console/script.c"
#include "process/process.c"
#include "dexapi/dex32API.c"
#include "hardware/ATA/ide.c"
#include "vfs/vfs_aux.c"
#include "memory/kheap.c"
#include "memory/dexmem.c"
#include "memory/dexmalloc.c"
#include "vmm/vmm.c"

//another set of forward declarations
void dex32_startup(); 
extern startup();

fg_processinfo *fg_kernel = 0;

//holds the name of the device that booted the kernel
char boot_device_name[255]="";

/*the start of the main kernel-- The task here is to setup the memory
  so that we could use it, we also enable some devices like the keyboard 
  and the floppy disk etc.
  
  Assumptions:
  DEX assumes that at this point the following should be true:
  
  * Protected Mode is enabled
  * paging is disabled
  * interrupts are disabled
  * The CS,DS,SS,ESP must already be set up, meaning that the GDT should already be present, see startup.asm
  
  ORDER is important when starting up the kernel modules!!*/

multiboot_header *mbhdr = 0;


//here we go!
void main(){
   char temp[255];
    
   /*obtain the multiboot information structure from GRUB which contains info about memory
      and the device that booted this kernel*/
   mbhdr =(multiboot_header*)multiboothdr;
    
   /* Enable the keyboard IRQ,Timer IRQ and the Floppy Disk IRQ.As more devices that uses IRQs get supported, we should OR more of them here*/
   //program8259(IRQ_TIMER | IRQ_KEYBOARD | IRQ_FDC | IRQ_MOUSE | IRQ_CASCADE); 
   program8259(IRQ_TIMER | IRQ_KEYBOARD | IRQ_FDC | IRQ_CASCADE); 

   //sets up the default interrupt handlers, like the PF handler,GPF handler
   setdefaulthandlers();   
    
   /*and some device handlers like the keyboard handler
     initializes the keyboard*/
   installkeyboard(); 

    //obtain the device which booted this operating system         
   kernel_systeminfo.boot_device = mbhdr->boot_device >> 24;

   if (kernel_systeminfo.boot_device == 0){  
      //floppy
      strcpy(boot_device_name,"fd0");
   }else{ //hard disk
      kernel_systeminfo.part[0] =    (mbhdr->boot_device >> 16) & 0xFF;
      kernel_systeminfo.part[1] =    (mbhdr->boot_device >> 8) & 0xFF;
      kernel_systeminfo.part[2] =    (mbhdr->boot_device & 0xFF);
      int n=kernel_systeminfo.boot_device - 0x80;
      sprintf(boot_device_name,"hdp%dp%d",n,kernel_systeminfo.part[0]);
   }

   //obtain information about the memory configuration
   memory_map = mbhdr->mmap_addr;
   map_length = mbhdr->mmap_length;
        
   
   /*
    DEX stores the free physical pages as a stack of free pages, therefore
    when a physical page of memory is needed, DEX just pops it off the stack.
    If DEX recovers used memory, it is pushed to the stack.
    The createstack() function creates the physical pages stack.
    See dexmem.c for details*/
    
   memamount = mem_detectmemory(memory_map, map_length);

    
   /*The mem_init() function first sets up the page table/directories which
     is used by the MMU of the CPU to map vitual memory locations to physical 
     memory locations. Basically the first 3MB of physical memory is mapped
     one-to-one (meaning virtual memory location = physical memory location.
     Finally it assigns the the location of the page directory to the CR3
     register and then enables paging.
      
     NOtE: DEX uses the flat memory model and all segment registers used by
     DEX has a base equal to zero*/
   mem_init(); 
    
   /*The default values of the current_process variable, which is the kernel
     PCB*/
   current_process = &sPCB;

   //Program the Timer to context switch n times a second	
   dex32_set_timer(context_switch_rate);

   //initialize the bridge manager, see bridges.c for details
   bridges_init();
    
   //initialize the virtual console manager
   fg_init();
    
   //Create a virtual console that the kernel will send its output to
   consoleDDL = Dex32CreateDDL();
   fg_kernel = fg_register(consoleDDL, 0);
   fg_setforeground(fg_kernel);
    
   /* Preliminary initializaation complete, start up the operating system*/
   dex32_startup(); 
};

//next stage
void dex32_startup(){
    
   /*At this point, memory accesses should already be safe, and
     until the scheduler starts, the interrupts must be disabled*/

   //Display some output for introductory purposes :)
   //clrscr();

   /*show parameter information sent by the multiboot compliant bootloader.*/
   //printf("Bootloader name : %s\n", mbhdr->boot_loader_name);
    
   //obtain CPU information using the CPUID instruction
   printf("Obtaining CPU information...\n");
   hardware_getcpuinfo(&hardware_mycpu);
   hardware_printinfo(&hardware_mycpu);
    
   printf("Available memory: %d KB\n", memamount/1024);

   //Initialize the extension manager
   printf("Initializing the extension manager...");
   extension_init();
   printf("[OK]\n");

   //initialize the device manager
   printf("Initializing the device manager...");
   devmgr_init();
   printf("[OK]\n");

   //register the memory manager
   printf("Registering the memory manager and the memory allocator...");
   mem_register();

   //register the different memory allocators
   bsdmalloc_init();       //BSD malloc
   dlmalloc_init();        //Doug Lea's malloc
   dexmalloc_init();       //Joseph Dayo's (*poor*) first fit malloc function
    
   /* initialize the malloc server, place the device name of the malloc
      function you wish to use as the paramater*/
   alloc_init("dl_malloc"); 
   printf("[OK]\n");
    
   //register the hardware ports manager
   printf("Initializing ports...");
   ports_init();
   printf("[OK]\n");

   //Initialize the PCI bus driver
   //printf("Initializing PCI devices...");
   //show_pci();
   //delay(400/80);
   //printf("[OK]\n");
				
   //initialize the API module
   printf("Initializing kernel API...");		  
   api_init();
   printf("[OK]\n");

   //initialize the keyboard device driver
   printf("Initializing keyboard and mouse drivers...");
   init_keyboard();
   installmouse();
   init_mouse();
   printf("[OK]\n");
   
   //Initialize the process manager and the initial
   //processes
   printf("Initializing the process manager...");
   process_init();    //defined in process.c
   printf("[OK]\n");

   //process manager is ready, pass execution to the taskswitcher
   taskswitcher();      //defined in process.h

    //============ we should not reach this point at all =================
   while (1)
      ;
};

#define STARTUP_DELAY 400

/*This function is the first function that is called by the taskswitcher
 see process/process.c
 incidentally it is also the first process that gets run
 it is the equivalent of the init() process in *nix systems
*/
void dex_init(){
   char temp[255],spk;
   int consolepid,i,baremode = 0;
   int delay_val =  STARTUP_DELAY / 80;
   devmgr_block_desc *myblock;
   dex32_datetime date;
    
   textcolor(GREEN);
   printf("\n");
   printf("\t\t");printf(OS_NAME);printf(" ");printf(OS_VERSION);
   printf(" (Build: %s)\n\n",build_id);
   textcolor(WHITE);
   printf("Starting dex_init()...\n");
   printf("Press space to skip autoexec.bat processing\n");

   //At this point, the kernel has fininshed setting up memory and the process scheduler.
   //More importantly, interrupts are already operational, which means we can now set up
   //devices that require IRQs like the floppy disk driver 
      
    
   //add some hotkeys to the keyboard
   //kb_addhotkey(KEY_F6+CTRL_ALT, 0xFF, fg_next);
   //kb_addhotkey(KEY_F5+CTRL_ALT, 0xFF, fg_prev);
   //kb_addhotkey('\t', KBD_META_ALT, fg_toggle);
   kb_addhotkey(KEY_F12, 0xFF, fg_next); //move accross the consoles
   kb_addhotkey(KEY_F11, 0xFF, fg_prev);
   kb_addhotkey(KEY_F2, 0xFF, console_new);
   kb_addhotkey('\t', KBD_META_ALT, fg_toggle);
    
   keyboardflush();
    
   /*Now that the timer is active we can now use time based functions.
     Delay for two seconds in order to see previous messages */  
    
   textbackground(GREEN);
   for (i=0 ;i < 79; i++){
      printf(" ");  
      if (kb_ready()){
         if (getch() ==' ') {
            baremode = 1;
            break;
         };        
         delay( delay_val );
      };
   }
   textbackground(BLACK);
   printf("\n");  

   printf("Getting date and time...");
   getdatetime(&date);
   getmonthname(date.month);
   printf("[OK]\n");   

   //Install the built-in floppy disk driver
   printf("Installing floppy driver...");
   floppy_install("fd0"); 
   printf("[OK]\n");   
    
   /*Install the IDE, ATA-2/4 compliant driver in order to be able to
      use CD-ROMS and harddisks. This will also create logical drives from
      the partition tables if needed.*/
   printf("Initializing IDE drivers...\n");
   ide_init();
   printf("[OK]\n");   

   /*Install the VGA driver*/
   printf("Loading VGA driver...");
   vga_init();
   printf("[OK]\n");   
 
   //initialize the I/O manager
   iomgr_init();

   //initialize the floppy device
   myblock = (devmgr_block_desc*)devmgr_devlist[floppy_deviceid];
   myblock->init_device();

   //initialize the file tables (Initialize the VFS)
   printf("Initializing the Virtual File System...");
   vfs_init();
   printf("[OK]\n");   

   //set the current directory of the init process to the vfs root
   current_process->workdir= vfs_root;
    
   //Initialize the task manager - a module program that monitors processes
   //for the user's convenience, as kernel thread
   printf("Initializing the task manager...");
   tm_pid=createkthread((void*)dex32_tm_updateinfo,"task_mgr",3500);
   printf("[OK]\n");   


   //create the IO manager thread which handles all I/O to and from
   //block devices like the hard disk, floppy, CD-ROM etc. see iosched.c
   printf("Initializing the disk manager...");
   createkthread((void*)iomgr_diskmgr,"disk_mgr",200000);
   printf("[OK]\n");   

   
   //Install a null block device
   printf("Initializng the null block device...");
   devfs_initnull();
   printf("[OK]\n");   
    
   printf("Initializing the filesystem driver...");
   //install and initialize the Device Filesystem driver
   devfs_init();
    
   //install and initialize the fat12 filesystem driver
   fat_register("fat");
    
   //initialize the CDFS (ISO9660/Joliet) filesystem
   iso9660_init();
   printf("[OK]\n");   

   printf("Mounting boot device %s...", boot_device_name);
   if (strcmp(boot_device_name,"fd0") == 0){
      //mount the boot device
      vfs_mount_device("fat",boot_device_name,"icsos");
   }else{
      //for livecd
      vfs_mount_device("cdfs","cds0","icsos");
   }
   printf("[OK]\n");   

   //setup the initial executable loaders (So we could run .EXEs,.b32,coff and elfs)
   printf("Initializing first module loader(s) [EXE][COFF][ELF][DEX B32]...");
   dex32_initloader();
   printf("[OK]\n");   

   /*Supposed to initialize the Advanced Power Management Interface
     so that I could do a "software" shutdown **IN PROGRESS** */
   //dex32apm_init();

   printf("Running foreground manager thread\n");
    
   //create the foreground manager
   fg_pid = createkthread((void*)fg_updateinfo,"fg_mgr",20000);
    
   if (baremode) 
      console_first++;
   printf("dex32_startup(): Running console thread\n");
    
   //Create a new console instance
   consolepid = console_new();


   /*beep the computer just in case a screen problem occured, at least
     we know it reaches this part*/
   spk=inportb(0x61);
   spk=spk|3;
   outportb(0x61,spk);
   delay(1);
   spk=inportb(0x61);
   spk=spk&252;
   outportb(0x61,spk);

   //set the console for this process
   Dex32SetProcessDDL(consoleDDL, getprocessid());
    
   /* Run the process dispatcher.
      The process dispatcher is responsible for running new modules/process.
      It is the only one that could disable paging without crashing the system since
      its stack, data and code segments are located in virtual memory that is at the
      same location as the physical memory
      see pdispatch.c/pdispatch.h for details
   */
   process_dispatcher();   // defined in kernel/process/pdispatch.c
    ;
};

void end_func()
{
};
