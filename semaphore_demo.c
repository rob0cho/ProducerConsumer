/*
 * unnamed_semaphores.c
 * Demonstration of POSIX unnamed semaphores
 *
 * This program creates two threads.
 *	thread 0 - increments a common variable N_0 times
 *	thread 1 - decrements the same variable N_1 times  (ooh exciting!)
 *
 * Compilation:  gcc -o semaphore_demo semaphore_demo.c -lpthread -lrt
 *
 * Libraries:
 * rt - POSIX Real Time, required for semaphores and thread yield
 * pthread - POSIX thread library
 *
 * Usage:  semaphore_demo [-h] [-i N] [-d N]
 *	-h : report usage (this message)
 *	-i N :  N is the number of times to increment
 *	-d N :  N is the number of times to decrement 
 *	
 */



// g++ mizzo.cpp -o mizzo
//  g++ mizzo.cpp producer.cpp consumer.cpp -o mizzo
// g++ mizzo.cpp producer.cpp consumer.cpp -o mizzo -lpthread -lrt
// ./mizzo -E 1 -L 2 -f 3 -e 4
// ./mizzo -E 1 -L 1 -f 999 -e 999

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>	/* POSIX semaphores prototypes & defns */
#include <pthread.h>	/* POSIX threads prototypes & defns */
#include <unistd.h>

/* usage string for pilot error */
char USAGE[] = " [-i N (default %d)] [-d N (default %d)]\n" 
	"\t-h : report usage (this message)\n"
	"\t-i N :  N is the number of times to increment\n"
	"\t-d N :  N is the number of times to decrement\n";

/* program exit codes */
#define EXT_NORMAL	 0
#define EXT_USAGE	 5
#define EXT_SEMAPHORE	10
#define EXT_THREAD	15

#define	INCREMENT_TIMES_DEFAULT	50
#define DECREMENT_TIMES_DEFAULT	50

/* Types of operations threads can perform */
typedef enum {
  INCREMENT,
  DECREMENT
} OPERATION;
  
  
/* Structure for communicating data to increment/decrement threads */
typedef struct {
  OPERATION	Operation;	/* Specify what should be done */
  char		*Name;		/* Human readable name of operation */
  int		N;		    /* Number of times to perform operation */

  sem_t		*MutexPtr;	/* pointer to critical region semaphore */

  int		*ValuePtr;	/* pointer to shared data */
} THREAD_DATA;

/*
 * operate - Expects a pointer to THREAD_DATA with information
 * on what operation to perform and how many times to do it, along
 * with the data to be operated on and a semaphore for accessing
 * the shared data in a critical region.
 */
void * operate(void * VoidPtr) {
  int	i, Delta;

  /* Typecast into a pointer of the expected type. */
  THREAD_DATA	*DataPtr = (THREAD_DATA *) VoidPtr;

  switch (DataPtr->Operation) {
  case INCREMENT:
    Delta = 1;
    break;
  case DECREMENT:
    Delta = -1;
    break;
  default:
    /* Bad input, return without any operations */
    return NULL;
  }
  
  for (i=0; i < DataPtr->N; i++) {
    sem_wait(DataPtr->MutexPtr);	/* entry */
    sleep(1);

    /* critical region */
    *(DataPtr->ValuePtr) = *(DataPtr->ValuePtr) + Delta;
    printf("After %s --> %5d\n", DataPtr->Name, *(DataPtr->ValuePtr));
    fflush(stdout);
    
    sem_post(DataPtr->MutexPtr);	/* exit */

  }

  return NULL;
}

/*
 * program entry point
 */
int main(int argc, char *argv[]) {
  
  THREAD_DATA	DecrementData;	/* thread declarations & their data */
  pthread_t	DecrementThread;
  THREAD_DATA	IncrementData;
  pthread_t	IncrementThread;
  sem_t		Mutex;		/* critical region semaphore */
  int		Value;		/* data to be shared between threads */
  int		Option;		/* command line switch */
  extern char	*optarg;	/* global value set by getopt */
  void		*ThreadResultPtr;
  
  Value = 0;
  
  /* Initialize data structures -------------------- */
  DecrementData.ValuePtr = &Value;	/* set up shared data pointer */
  DecrementData.MutexPtr = &Mutex;
  DecrementData.Operation = DECREMENT;
  /* Decrement name has leading space so it is easy to see
   * the difference between where inc/dec has occurred in output
   */
  DecrementData.Name = " -";	
  DecrementData.N = DECREMENT_TIMES_DEFAULT;	/* set defaults */

  IncrementData.ValuePtr = &Value;	/* similar */
  IncrementData.MutexPtr = &Mutex;
  IncrementData.Operation = INCREMENT;
  IncrementData.Name = "+ "; 
  IncrementData.N = INCREMENT_TIMES_DEFAULT;

  /* Process command line arguments --------------------
   * man -s 3c getopt for details
   */
  while ( (Option = getopt(argc, argv, "i:d:h")) != -1) {
    switch (Option) {
    case 'i':	/* increment N times */
      IncrementData.N = atoi(optarg);	/* Get value from string */
      break;
    case 'd':	/* decrement N times */
      DecrementData.N = atoi(optarg);
      break;

    default:	/* Handle help & illegal args */
      /* argv[0] - has executable name */
      printf("Usage: %s : ", argv[0]);
      printf(USAGE, INCREMENT_TIMES_DEFAULT, DECREMENT_TIMES_DEFAULT);
      exit(EXT_USAGE);	/* exit program */
    }
  }

  /* Create the semaphore --------------------
   * arg 1 - semaphore handle
   * arg 2 - Always zero for unnamed semphores
   * arg 3 - Initial value
   */
  if (sem_init(&Mutex, 0, 1) == -1) {
    fprintf(stderr, "Unable to initialize Mutex semaphore\n");
    exit(EXT_SEMAPHORE);
  }

  /* Finally all set up - let's go -------------------- */

  if (pthread_create(&IncrementThread, NULL, operate, &IncrementData)) {
    fprintf(stderr, "Unable to create increment thread\n");
    exit(EXT_THREAD);
  }

  if (pthread_create(&DecrementThread, NULL, operate, &DecrementData)) {
    fprintf(stderr, "Unable to create decrement thread\n");
    exit(EXT_THREAD);
  }

  /* wait for threads to exit --------------------
   * Note that these threads always return a NULL result pointer
   * so we will not be checking the ThreadResultPtr, but they
   * could return something using the same mechanisms that we used 
   * to pass data in to the thread.
   */

  
  if (pthread_join(IncrementThread, &ThreadResultPtr)) {
    fprintf(stderr, "Thread join error\n");
    exit(EXT_THREAD);
  }
  
  if (pthread_join(DecrementThread, &ThreadResultPtr))  {
    fprintf(stderr, "Thread join error\n");
    exit(EXT_THREAD);
  }

  /* Display final value.  No need for critical regions as we
   * are the only thread running.
   */
  printf("Final value:  %d\n", Value);
  
  exit(EXT_NORMAL);
}