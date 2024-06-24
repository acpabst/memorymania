/* Memory Mania
 *
 * Author: Andy Pabst
 * Date: 6/12/24 */

#include "memorymania.h"
#include "helpers.c"

int test() {
   // open buffer
   int buffer;
   char buffer_entry[2];
   buffer_entry[1] = '\n';

   buffer = open(BUFFER, O_RDWR);
   if (buffer < 0) {
        printf("Error opening buffer device.\n");
        return 1;
   }
   printf("file descriptor: %i\n", buffer);

   buffer_entry[0] = 'T';
   
   int res = write(buffer, &buffer_entry, 2);
   printf("write: %i\n", res);

   char buf[100];
   res = read(buffer, buf, 100);
   printf("errno: %i\n", errno);
   printf("num: %i, read: %s\n", res, buf);
   close(buffer);
}

void game_timer(int ppid) {
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

    while(!game_over) {
	sleep(0.01);
        if(caught_sigalrm) {
	    syslog(LOG_INFO, "Sigarlm caught.");
	    if(game_over) {
	        printf("Time is up!\n");
	    }
	    kill(ppid, SIGALRM);
	    break;
	}
	if(caught_sigusr) {
	   syslog(LOG_INFO,"Sigusr caught. Resetting timer.");
           r = setitimer(ITIMER_REAL, &timer_spec, 0);
	   caught_sigusr = false;
	}
    }
    exit(0);
}

// adapted for this game from jstest 
char getButtonPress(int fd, unsigned char *buttons) {
    
    syslog(LOG_INFO, "Getting button press");
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
    
    while (waiting) {

        if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
            syslog(LOG_ERR, "Error reading controller input");	
            return 1;
        }

        if (js.type == JS_EVENT_BUTTON && js.value == 1) {
	    waiting = false;
	    switch(js.number) {
	        case 0:
		    button = 'A';
		    break;
	        case 1:
		    button = 'B';
		    break;
	        case 2:
		    button = 'X';
		    break;
	        case 3:
		    button = 'Y';
		    break;
	        case 4:
		    button = 'L';
		    break;
	        case 5:
		    button = 'R';
		    break;
	        default:
		    printf("Invalid button pressed.\n");
		    break;
	    }
	}
    }
    return button;
}


int main (int argc, char*argv[]) {

    //test();
    //return 0;

    int controller, buffer;
    int r;
    int score = 0;

    unsigned char buttons = 2;
    int version = 0x000800;

    char buffer_entry[2];
    buffer_entry[1] = '\n';
    char test_buffer[] = {'N','N','N','N','N','N','N','N','N','N','N'};

    printf("Welcome to MEMORY MANIA!!\n");
    printf("Try to remember the sequence! The max score on today's game is %i.\n", MAX_SCORE);

    openlog(NULL, 0, LOG_USER); 
    set_signal_handling();

    if(!MEMORYMANIA_DEBUG) {
        // open controller
        syslog(LOG_INFO,"Opening device: %s", argv[argc - 1]);
        if ((controller = open(argv[argc - 1], O_RDONLY)) < 0) {
	    syslog(LOG_ERR,"Error opening device. Error: %d\n", errno);
	    exit(-1);
        }
	ioctl(controller, JSIOCGVERSION, &version);
        ioctl(controller, JSIOCGBUTTONS, &buttons);
    }

    printf("Press Enter, or any controller button to contiue.\n");
    char input = getInput(controller, &buttons);

    // begin timer
    syslog(LOG_INFO,"Creating timer . . .\n");
    int ppid = getpid();
    r = fork();
    if (r < 0) {
        syslog(LOG_ERR, "Error creating timer fork: %d\n", errno);
	exit(-1);
    } else if (r == 0) {
	game_timer(ppid);
    }

    // begin ncurses
    initscr();
    echo();

    // generate the first character
    int index_max = sizeof(supported_chars) - 1;
    srand(time(0));
    int rand_index = rand() % (index_max + 1 - 0);
    buffer_entry[0] = supported_chars[rand_index];
    buffer = open(BUFFER, O_RDWR);
    if (buffer < 0) {
        syslog(LOG_ERR, "Error opening buffer device.");
        return -1;
    }
    int numwritten = write(buffer, &buffer_entry, 2);
    close(buffer);

    char sequence[MAX_SCORE*2];
    while(!game_over) {
	buffer = open(BUFFER, O_RDWR);
        if (buffer < 0) {
            syslog(LOG_ERR, "Error opening buffer device.");
            return -1;
        }
	int numread = read(buffer, sequence, sizeof(sequence));
	for (int i = 0; i <= numread; i++) {
	    printw("%c", sequence[i]); refresh();
	    usleep(500000);
	}
	clear(); refresh();
	close(buffer);

        int j = 0;
	for (int i = 0; i <= score; i++) {
	    char input = getInput(controller, &buttons);
	    printw("%c", input);
	    refresh();
	    if (input == sequence[j]) {
		kill(r, SIGUSR1);
	    } else {
	        endwin();	    
		printf("Incorrect :(\n");
		printf("You said: %c\n", input);
                printf("Correct answer: %c\n", sequence[j]);
		game_over = true;
		kill(r, SIGALRM);
		break;
            }
	    j++;
	    j++;
	}
	if (MEMORYMANIA_DEBUG && !game_over) {
            getchar(); // get carriage return
        }
	clear(); refresh();
	usleep(250000);
	if(!game_over) {
	    score++;
	    printw("Correct!"); refresh();
	    usleep(500000);
	    clear(); refresh();
	}        
	if (score >= MAX_SCORE) {
	    endwin();
	    printf("You win!\n");
	    game_over = true;
	    break;
        }

        // generate new character
	int index_max = sizeof(supported_chars) - 1;
	int rand_index = rand() % (index_max + 1 - 0);
	buffer_entry[0] = supported_chars[rand_index];
        buffer = open(BUFFER, O_RDWR);
        if (buffer < 0) {
            syslog(LOG_ERR, "Error opening buffer device. Error: %i\n", errno);
            exit(-1);
        }
	write(buffer, &buffer_entry, 2);
	close(buffer);
    }
    printf("Game Over!\n");
    printf("Score: %i\n", score);

    close(controller);
    
    buffer = open(BUFFER, O_RDWR);
    if (buffer < 0) {
        printf("Error opening buffer device.\n");
        exit(-1);
    }
    int ioctl_try = ioctl(buffer, AESDCHAR_FLUSH);
    if(ioctl_try < 0) {
        printf("Error flushing buffer. Error: %i\n", errno);
    }
    close(buffer);
    endwin();
    exit(0);
}
