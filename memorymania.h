/* Memory Mania Header
 *
 * Author: Andy Pabst
 * Date 6/12/24 */

#ifndef MEMORYMANIA_H_
#define MEMORYMANIA_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/joystick.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <syslog.h>
#include <string.h>

#include "assignments-3-and-later-acpabst/aesd-char-driver/aesd_ioctl.h"

// Supported Characters
char supported_chars[] = {'A', 'B', 'X', 'Y', 'L', 'R'};

#define MEMORYMANIA_DEBUG 0 

#if MEMORYMANIA_DEBUG
   /* This one for keyboard input */
#  define getInput(args...) fgetc(stdin)
#else
    /* This one for XBOX Controller input*/
#  define getInput(fd, buttons) getButtonPress(fd, buttons)
#endif

#define MAX_SCORE 10
#define BUFFER "/dev/aesdchar"

// global veriables
bool game_over = false;
bool caught_sigalrm = false;
bool caught_sigusr = false;

int buffer_open();
void set_signal_handling();

#endif
