/* MemoryMania Helper Functions
 *
 * Author: Andy Pabst
 * Date: 6/19/24
 */

#include "memorymania.h"

static void signal_handler(int signal_number) {
    if (signal_number == SIGALRM) {
        caught_sigalrm = true;
    }
    if (signal_number == SIGUSR1) {
        caught_sigusr = true;
    }
}

void set_signal_handling() {
    struct sigaction new_action;
    memset(&new_action, 0, sizeof(struct sigaction));
    new_action.sa_handler = signal_handler;
    int sa = sigaction(SIGALRM, &new_action, NULL);
    if (sa != 0) {
        syslog(LOG_ERR, "Failed to register for SIGALRM. Error: %d", sa);
        printf("Failed to register for SIGALRM. Error: %d\n", sa);
        exit(-1);
    }
    sa = sigaction(SIGUSR1, &new_action, NULL);
    if (sa != 0) {
        syslog(LOG_ERR, "Failed to register for SIGUSR. Error: %d", sa);
        printf("Failed to register for SIGUSR. Error: %d\n", sa);
        exit(-1);
    }
    syslog(LOG_INFO, "Signal Handling set up complete.");
    printf("Signal handling set up complete.\n");
}
