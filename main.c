/* Memory Mania
 *
 * Author: Andy Pabst
 * Date: 6/12/24 */

#include "memorymania.h"

bool game_over = false;
bool caught_sigalrm = false;
bool caught_sigusr = false;

static void signal_handler(int signal_number) {
    if (signal_number == SIGALRM) {
	caught_sigalrm = true;
	game_over = true;
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

void game_timer() {
    time_t t;
    int r;
    int num_char;
    struct itimerval timer_spec;

    timer_spec.it_interval.tv_sec = 10;
    timer_spec.it_interval.tv_usec = 0;
    timer_spec.it_value.tv_sec = 10;
    timer_spec.it_value.tv_usec = 0;
    
    r = setitimer(ITIMER_REAL, &timer_spec, 0);
    if (r != 0 ) {
        syslog(LOG_ERR, "Failed to create timer. Error: %d", errno);
	printf("Failed to create timer. Error: %d\n", errno);
	exit(-1);
    }

    printf("In timer. Alarm should be set.\n");
    
    while(!game_over) {
	sleep(0.01);
        if(caught_sigalrm) {
	    printf("Sigalarm caught. Time is up!\n");
	    game_over = true;
	    break;
	}
	if(caught_sigusr) {
	   printf("Sigusr caught. Resetting timer.\n");
           r = setitimer(ITIMER_REAL, &timer_spec, 0);
	   caught_sigusr = false;
	}
    }
    printf("Exiting Timer Process. . . \n");
    exit(0);
}

// adapted for this game from jstest 
char getButtonPress(int fd, unsigned char *buttons) {
    
    printf("Getting button press\n");
    char button;
    //int i;
    bool waiting = true;
    struct js_event js;
    js.type = JS_EVENT_INIT;
    js.number = 0;
    js.time = 0;
    js.value = 0;

    if (caught_sigalrm) {
        // time is up, return dummy character
        printf("Time is up, from button press\n");
    }
    
    //while (waiting) {

        if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
            printf("Something went wrong");	
            return 1;
        }
	if (caught_sigalrm) {
	    // time is up, return dummy character
	    printf("Time is up, from button press\n");
            return 'N';
	}
        if (js.type == JS_EVENT_BUTTON && js.value == 1) {
	    waiting = false;
	    switch(js.number) {
	        case 0:
                    printf("Pressed A\n");
		    button = 'A';
		    break;
	        case 1:
		    printf("Pressed B\n");
		    button = 'B';
		    break;
	        case 2:
		    printf("Pressed X\n");
		    button = 'X';
		    break;
	        case 3:
		    printf("Pressed Y\n");
		    button = 'Y';
		    break;
	        case 4:
		    printf("Pressed L\n");
		    button = 'L';
		    break;
	        case 5:
		    printf("Pressed R\n");
		    button = 'R';
		    break;
	        default:
		    printf("Invalid button pressed.\n");
		    break;
	    }
	}
    //}
    return button;
}


int main (int argc, char*argv[]) {

    int fd;
    int r;
    int score = 0;

    unsigned char buttons = 2;
    int version = 0x000800;

    char test_buffer[] = {'N','N','N','N','N','N','N','N','N','N','N'};

    openlog(NULL, 0, LOG_USER); 
    set_signal_handling();

    // open controller
    printf("Opening device: %s\n", argv[argc - 1]);
    if ((fd = open(argv[argc - 1], O_RDONLY)) < 0) {
	printf("Error opening device.");
	return 1;
    }
    printf("Welcome to MEMORY MANIA!!\n");
    printf("Try to remember the sequence! The max score on today's game is %i.\n", MAX_SCORE);
    ioctl(fd, JSIOCGVERSION, &version);
    ioctl(fd, JSIOCGBUTTONS, &buttons);

    // begin timer
    printf("Creating timer . . .\n");
    r = fork();
    if (r < 0) {
        printf("Error creating timer fork: %d\n", errno);
	exit(-1);
    } else if (r == 0) {
	game_timer();
    }

    // generate the first character
    int index_max = sizeof(supported_chars) - 1;
    int rand_index = rand() % (index_max + 1 - 0);
    char rand_char = supported_chars[rand_index];
    printf("%c\n", rand_char);
    test_buffer[0] = rand_char;
    printf("Begin Sequence. Type this letter\n");

    while(!game_over) {
	printf("score: %i\n", score);
	for (int i = 0; i <= score; i++) {
	    printf("i: %i\n", i);
	    char input = getInput(fd, &buttons);
	    if (caught_sigalrm) {
		    printf("Something is happening. . .\n");
	    }
	    printf("You said: %c\n", input);
	    if (input == test_buffer[i]) {
	        printf("Correct!\n");
		kill(r, SIGUSR1);
	    } else {
		printf("Incorrect :(\n");
		game_over = true;
		kill(r, SIGALRM);
		break;
            }
	}
	score++;
	if (score >= MAX_SCORE) {
	    printf("You win!\n");
	    game_over = true;
	    break;
        }

	if (MEMORYMANIA_DEBUG) {
	    getchar(); // get carriage return
	}
        
        // generate new character
	int index_max = sizeof(supported_chars) - 1;
	int rand_index = rand() % (index_max + 1 - 0);
	char rand_char = supported_chars[rand_index];
	printf("Random character selected: %c (index: %i)\n", rand_char, rand_index);
        test_buffer[score] = rand_char;
    }
    printf("Game Over!\n");
    printf("Score: %i\n", score);

    exit(0);
}
