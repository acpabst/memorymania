/* Memory Mania
 *
 * Author: Andy Pabst
 * Date: 6/12/24 */

#include "memorymania.h"

// adapted for this game from jstest 
char getButtonPress(int fd, unsigned char *buttons) {

    char *button;
    int i;
    struct js_event js;

    button = calloc(buttons, sizeof(char));

    if (read(fd, &js, sizeof(struct js_event)) != sizeof(struct js_event)) {
        printf("Something went wrong");	
        return 1;
    }

    if ((js.type & ~JS_EVENT_INIT) == JS_EVENT_BUTTON) {
        button[js.number] = js.value;
    }
    // TODO change this so the button range only works for the supported buttons
    for (int i = 0; i < buttons; i++) {
        if(button[i]) {
	     printf("Character pressed: %i\n", i);
	     printf("Corresponding letter: %c\n", supported_chars[i]);
	}
    }
    return supported_chars[i];
}


int main (int argc, char*argv[]) {

    int fd;
    bool game_over = false;
    int score = 0;

    unsigned char buttons = 2;
    int version = 0x000800;

    char test_buffer[] = {'N','N'};

    // open controller
    printf("Opening device: %s\n", argv[argc - 1]);
    if ((fd = open(argv[argc - 1], O_RDONLY)) < 0) {
	printf("Error opening device.");
	return 1;
    }
    printf("Made it here\n");
    ioctl(fd, JSIOCGVERSION, &version);
    ioctl(fd, JSIOCGBUTTONS, &buttons);

    // generate the first character
    int index_max = sizeof(supported_chars) - 1;
    int rand_index = rand() % (index_max + 1 - 0);
    char rand_char = supported_chars[rand_index];
    printf("%c\n", rand_char);
    test_buffer[0] = rand_char;
    printf("Begin Sequence. Type this letter\n");

    while(!game_over) {
	 for (int i = 0; i < score + 1; i++) {
	    //char input = getchar();
	    char input = getButtonPress(fd, &buttons);
	    printf("You said: %c\n", input);
	    if (input == test_buffer[i]) {
	        printf("Correct!\n");
	    } else {
		printf("Incorrect :(\n");
		game_over = true;
		break;
            }
	    if (i == score) {
	        input = getchar(); // get the enter press. don't do anything with it
	        printf("Extra character: %i\n", input);
	    }
	    // logic for if the strong is correct or not
	 }
	 score++;
	 if (score > MAX_SCORE) {
	    printf("You win!\n");
	    game_over = true;
	    break;
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
