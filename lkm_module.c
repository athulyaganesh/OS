// Here begins "code block one" as defined in this lab writeup. Here we have #includes
// for headers and #defines to set some constants that we'll use elsewhere in the
// source code

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define DEVICE_NAME "lkm_module"
#define EXAMPLE_MSG "Hello from the lkm_module!\n"
#define MSG_BUFFER_LEN 40


// Here begins "code block two" as defined in the lab writeup.  Here we make a bunch
// of calls to documentation macros to embed information into the module that can
// be interrogated by those who require information.  The original module is GPL,
// so our modifying it and giving full credit to the original author is totally
// cool.

MODULE_AUTHOR("Original Author: Robert W. Oliver II - modified by jcg");
MODULE_DESCRIPTION("A simple example character driver that does only one thing");
MODULE_VERSION("0.02");
MODULE_LICENSE("GPL");

// Here begins "code block three" as defined in the lab writeup.  Here we're actually
// defining executable code that will extend kernel functionality.

// Let's start with function prototypes.  The next four prototype definitions are
// forward references to "handlers" that we'll associate with kernel attempts to
// deal with the device we are creating.  These four handlers will represent what
// we want to happen whenever the kernel does any one of these four standard
// operations on the device we'll create (open, release, read,  and write)
// The forms of these functions and what parameters are expected are defined by
// kernel conventions.  We'll come back to this in more detail in a later lab.


static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static int major_num;

// Now let's move on to variable and struct declarations

static int device_open_count = 0;           // We will use this to keep track of devices
                                            // opened by this driver.  Really, this
                                            // module will only ever open one device,
                                            // but SOME drivers might open more than one
                                            // The point of keeping the count is so that
                                            // the driver knows when it should ignore
                                            // requests to "open more devices"

static char msg_buffer[MSG_BUFFER_LEN];     // We will use this to hold a message we want
                                            // this module to pass out whenever the
                                            // device it creates is read

static char *msg_ptr;                       // This is a pointer that we will set to
                                            // point to memory where the message is
                                            // located.

// struct file_operations is defined in linux/fs.h and is intended to be a repository
// of pointers to "service code that the kernel should use whenever any process attempts
// access to a device that is being defined by this module. When creating a device,
// the module programmer would write a handler for each of these standard operations
// on a device.  The module will modify the kernel data structures to make it "aware"
// of a new device that should be handled with these provided functions.  This
// code sets up the block of pointers.  Note that each field is associated with a
// handler functions we define IN this file.

static struct file_operations file_ops =
   { .read = device_read,
     .write = device_write,
     .open = device_open,
     .release = device_release
   };

// Now we begin to define each of the device driver handlers....

// The read handler device_read() moves the data held locally in the kernel memory
// pointed to by msg_ptr "into" the device that was created when this module was
// loaded.  We'll come back to the nuts and bolts of this in a later lab, but for
// now, we'll at least outline the meanings of the major steps.

static int count = 0; // STATIC VARIABLE COUNT SET TO ZERO
 
static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset)
  { int bytes_read = 0;
     // If we’re at the end of the local copy, loop back to the beginning
        if (*msg_ptr == 0 && count < 10) // ADDING THE COUNT TO THE IF STATEMENT. INSTEAD OF SENDING THE MESSAGE BUFFER EVERYTIME, IT DOES IT ONLY WHILE THE COUNT IS LESSER THAN 10 
	{
	msg_ptr = msg_buffer;
	count = count + 1 ; // INCREMENTING THE COUNTER VARIABLE AFTER EACH TIME THE MSG_PTR HAS A VALUE TO SEND TO THE NEXT WHILE LOOP. 
	}
	else
	{
	count = 0; //ONCE IT REACHES 10, THE STATIC VARIABLE IS RESET TO 0, SO THAT FOR THE NEXT CAT CALL, IT PRINTS ANOTHER 10 TIMES ONLY. 
	}

     // Send the data to the user via the device.  There's a little weirdness here.
     // The message is held in kernel memory, but the process that is accessing the
     // device lives in its own "virtual memory" space (more no that later too).
     // If we send things out to processes, we need kernel support to "get it out"
     // to a place where the process is even allowed to see it.  Remember, processes
     // NEVER get direct access to kernel memory.  For this purpose, we'll use the
     // put_user() service to copy a character from our internal kernel memory to
     // the place (device driver handle) that the requester can see it.  Again,
     // we will come back to this in more detail in a later lab.

        while (len && *msg_ptr) // USUALLY, THE MSG_PTR VARIABLE IS FILLED WITH INFORMATION, BUT WILL NOT HAVE INFORMATION TO ENTER THE WHILE LOOP AFTER 10 TIMES BECAUSE OF OUR COUNT VARIABLE.
           { put_user(*(msg_ptr++), buffer++);
             len--;
             bytes_read++;
           }
        return bytes_read;
   }

// The read handler device_write() does VERY little for this module.  We are making a
// device that is "one way" in that it only spits out data through the device it creates
// In this case, then, the "write handler" really only need to refuse to do anything
// and then put an error message in the system log.

static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset)
  { printk(KERN_ALERT "Write operation is not supported in module lkm_example.\n");
    return -EINVAL;
   }

// device_open() will be called whenever anyone tries to open the device from a user
// process.  We are allowing this device to be opened by ONLY one process at a time.
// You can see that in the behavior that's coded if the device count is not zero.
// The module basically returns a "I'm busy" to the kernel... which presumably will
// then do something that makes sense.

static int device_open(struct inode *inode, struct file *file)
   { /* If device is open, return busy */
     if (device_open_count) return -EBUSY;
     device_open_count++;
     try_module_get(THIS_MODULE);
     return 0;
   }

// device_release() is what is to be done when a process is done with the device
// governed by this module.  The most salient observation for now is that it
// decrements the "devices open" count, presumably freeing up the device to
// service any other requests that might be pending.

static int device_release(struct inode *inode, struct file *file)
  { device_open_count--;
    module_put(THIS_MODULE);
    return 0;
  }


// Here we define the handlers for "module loads" and "module unloads".  Again,
// this defines the code that modifies the kernel (or its operation) and the second
// defines how we put the kernel back to the way it was.  This is NOT in principle
// and different than what we did in the previous example.  It's just that we're doing
// more stuff ;)

// When the module is loaded, we will do all the things in lkm_example_init()...
// so here we go!

static int __init lkm_example_init(void)
   { // First, fill the buffer in the module/kernel memory with the message we'll send
     strncpy(msg_buffer, EXAMPLE_MSG, MSG_BUFFER_LEN);

     // Set the msg_ptr to the beginning of the kernel memory message buffer
     msg_ptr = msg_buffer;

     // Here's is where we're CLEARLY a "module type a" as defined in the lab writeup.
     // Here we're requesting the kernel to "register" a kernel standard character
     // device.  Registration entails modifying kernel data structures to give it
     // the additional responsibility of, in a way standard to character devices,
     // monitor and manage a new device.  register_chrdev() is a kernel function
     // that does this registration.  It takes three parameters...  a major device
     // number, a name for the device to be created, and the structure that contains
     // all the pointers to the standard handlers.  Note, that as a "type a" kernel
     // module, we're not changing the kernel as much as we're using its standard
     // internal services to ask it to take responsibility for a new device and
     // use our provided code to define those responsibilities.

     major_num = register_chrdev(0, "lkm_example", &file_ops);

     if (major_num < 0)
        { printk(KERN_ALERT "Could not register device: %d\n", major_num);
          return major_num; }
     else
        { printk(KERN_ALERT "lkm_module loaded with device major number %d\n",
                 major_num);
          return 0;
        }
   }

// When the module is unloaded, we're responsible for putting things back the way
// we found them before the module loaded.  In this case, it's REALLY simple.  All we
// need to do is unregister the handler using the standard kernel call for that.  Of
// course, if we had done a more "free form" modification, our unloading process might
// be way more complex.

static void __exit lkm_example_exit(void)
  { unregister_chrdev(major_num, DEVICE_NAME);
    printk(KERN_ALERT "the lkm_module has left the building...\n");
  }

// Here begins "code block four"...  where we just tell the kernel where to find
// the "inti/load" and "exit/unload" functions.  Note we're just passing the pointers
// to the functions we wrote in block three to provide these capabilities

module_init(lkm_example_init);
module_exit(lkm_example_exit);
