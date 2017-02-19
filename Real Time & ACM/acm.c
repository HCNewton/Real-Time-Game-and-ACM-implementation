#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// decide what type ACM to use, look in the handout for pseudo-code

char slots[3][22]; // 3 slots each 22 chars long
const int LUT[3][3] = { { 1, 2 , 1}, { 2, 0 , 0}, { 1, 0 , 1}};
int w= 0;	// write frame
int r= 1;	// read frame
int l= 2; 	// last frame
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 

void
reader_thread ()
{
  while(1)  // reader loop; includes a delay to reduce the output data rate
    {
		pthread_mutex_lock(&mutex);
		if(l != r){
		  r = l;
		  pthread_mutex_unlock(&mutex);
		  printf ("%s", slots[r]); // access slot; slots[i] is a pointer to slots[i][0]
		  fflush(stdout);
		  usleep(450000);    				// outputting 49 characters per second
		  } else {
		  pthread_mutex_unlock(&mutex);
	  }
    }
}

char input() // getchar() wrapper which checks for End Of File EOF
{
  char c = getchar();		// get char from stdin
  if(c==EOF) exit(0); 		// exit the whole process if input ends
  return c;
}

int
main ()
{
	pthread_t readerTh;
	pthread_create(&readerTh, NULL, (void*) &reader_thread, NULL);	// creates reader thread

  while (1)  // writer loop
	{
		int j = 0;	// writes into slot j
		while ((slots[w][j++] = input()) != '*'); // the actual computation takes place inside the condition
		slots[w][j] = 0; // append the terminating symbol to the string

		pthread_mutex_lock(&mutex);
		l = w;						// update last write slot
		w = LUT[r][l];				// update written slot
		pthread_mutex_unlock(&mutex);
    }

  return 0;
}
