/* Memory Mania Header
 *
 * Author: Andy Pabst
 * Date 6/12/24 */

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

