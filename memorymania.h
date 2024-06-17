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

// Supported Characters
char supported_chars[] = {'A', 'B', 'X', 'Y', 'L', 'R'};

#define MAX_SCORE 10

