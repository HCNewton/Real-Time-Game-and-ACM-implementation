#include <stdio.h>
#include <termios.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#define width  40
#define height 18
#define buf_length (width*height)
#define fox_init_x (width/3*2)
#define fox_init_y (height/3*2)
#define fox_init_dir 'u'						// fox starts going up
#define rabbit_init_x (width/9*2)
#define rabbit_init_y (height/9*2)
#define rabbit_init_dir 'd'						// rabbit starts going down

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // mutual exclusion for shared resources between threads

//---- fox direction
char fox_dir = fox_init_dir;

//---- set keyboard mode --
struct termios tty_prepare ()
{
    struct termios tty_attr_old, tty_attr;
    tcgetattr (0, &tty_attr);
    tty_attr_old = tty_attr;
    tty_attr.c_lflag &= ~(ECHO | ICANON);
    tty_attr.c_cc[VMIN] = 1;
    tcsetattr (0, TCSAFLUSH, &tty_attr);
    return tty_attr_old;
}

//---- restore keyboard mode --
void tty_restore (struct termios tty_attr)
{
    tcsetattr (0, TCSAFLUSH, &tty_attr);
}

//---- keyboard thread function
void keys_thread ()
{
    unsigned char a;
    while(1)
    {
        a=getchar();
        if (a == 0x1b)
        {
            a = getchar();
            if (a == 0x5b)
            {
				pthread_mutex_lock(&mutex);
                a = getchar();
                switch (a)
                {
                case 0x41:
                    fox_dir = 'u';
                    break;
                case 0x42:
                    fox_dir = 'd';
                    break;
                case 0x43:
                    fox_dir = 'r';
                    break;
                case 0x44:
                    fox_dir = 'l';
                    break;
                }
				pthread_mutex_unlock(&mutex);
            }
        }
    }

}

//---- update x and y coord-s according to direction; used in main()
void update_coord (int *x_ptr, int *y_ptr, char dir) // call by reference to x and y
{
    switch (dir)
    {
    case 'u':
        if (*y_ptr > 1) (*y_ptr)--;
        break; // *y_ptr is called "dereference",
// which is the target pointed at by the pointer
    case 'd':
        if (*y_ptr < 18) (*y_ptr)++;
        break;
    case 'l':
        if (*x_ptr > 1) (*x_ptr)--;
        break;
    case 'r':
        if (*x_ptr < 40) (*x_ptr)++;
        break;
    }
}

//---- the program starts its execution from here
int main ()
{
    // variable declarations and initialisation
    pthread_t keyT;							// keyboard thread	
    struct termios term_back;
    term_back = tty_prepare ();				// keyboard thread defined
    pthread_create (&keyT, NULL, (void *) &keys_thread, (void *) fox_init_dir); // create the keyboard thread

	char rabbit_dir = rabbit_init_dir;	
    int fox_x = fox_init_x;
    int fox_y = fox_init_y;
    int rabbit_x = rabbit_init_x;
    int rabbit_y = rabbit_init_y;

    while (1)
    {
        usleep (250000);								// emitting data at 4 fps or 88 cps
		pthread_mutex_lock(&mutex);						// fox_dir is locked
        update_coord (&fox_x, &fox_y, fox_dir);			// fox dir is updated
		pthread_mutex_unlock(&mutex);					// fox dir is unlocked
        int r = rand() % 4; // generate the rabbit direction at random to modulo 4
        switch (r)			// select rabbit direction relating to random number
        {
        case 0:
            rabbit_dir = 'u';
            break;
        case 1:
            rabbit_dir = 'd';
            break;
        case 2:
            rabbit_dir = 'l';
            break;
        case 3:
            rabbit_dir = 'r';
            break;
		default: break;
        }

        update_coord (&rabbit_x, &rabbit_y, rabbit_dir);	// rabbit coordinates 
        printf ("\033[2J\033[%d;%dH@\033[%d;%dH*", fox_y, fox_x, rabbit_y, rabbit_x); // 22 characters
        fflush (stdout);

        if ( (fox_y == rabbit_y) && (fox_x == rabbit_x) ) break; // conditions of game termination
    }

    pthread_cancel ((void *) &keys_thread); 
    tty_restore (term_back);					// keyboard is restored

    return 0;
}

