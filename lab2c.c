#define _GUN_SOURCE

#include "SortedList.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <stdint.h>

#include <sys/types.h>

int lock_type = 0;

//Global variables
int num_threads = 1;
int num_it = 1;
int num_lists = 1;

int opt_yield = 0; //--yield option

//For mutex
pthread_mutex_t* mutex;
pthread_mutex_t my_mut = PTHREAD_MUTEX_INITIALIZER;

//For test and set spin lock
static int* spin;
volatile int my_spin = 0;

#define CLOCK_PRECISION 1000000000L //For clock precision

//Before thread list of elements
SortedListElement_t* the_list = NULL;

//List of keys
char** key = NULL;

//Length of the keys
int key_len = 3;

//Thread linked list. An array of pointers to sub-lists
SortedList_t** head;

//For lookup return
SortedListElement_t* pick;

//Free allocated memory after program concludes
void free_structures (void)
{
  int k;

  //Free sub-lists
  for (k = 0; k < num_lists; k++)
    {
      if (head[k] != NULL)
	free(head[k]);
    }
  
  //Free linked list header 
  if (head != NULL)
    free(head);

  //Free the_list 
  if (the_list != NULL)
    free(the_list);

  //Free allocated keys for elements
  for (k = 0; k < (num_threads*num_it); k++)
    {
      if (key[k] != NULL)
	free(key[k]);
    }

  //Destroy mutexes if m option provided
  if (lock_type == 1)
    {
      for (k = 0; k < num_lists; k++)
	pthread_mutex_destroy(&mutex[k]);
      pthread_mutex_destroy(&my_mut);
    }

  //Free spin allocated memory if s option provided
  if (lock_type == 2)
    {
      if (spin != NULL)
	free(spin);
    }
  
}


int hash (const char* key)
{
  //Hash formula: (key[0] + key[1] + ... + key[length]) % num_lists
  int i;
  int tot = 0;
  
  for (i = 0; i < 3; i++)
    tot += key[i];
  
  return tot % num_lists;
}

//create random keys of length key_len
void make_key(char* key)
{
  int k;
  int pick_int;
  const char set[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789!@#$%^&*(),.'[]{}";
  for (k = 0; k < key_len; k++)
    {
      pick_int = rand() % (int) (sizeof(set) - 1);
      key[k] = set[pick_int];
    }
}

//Create various lists
void create_lists(long threads, long iterations)
{

  head = malloc(sizeof(SortedList_t*) * num_lists);
  
  int l;
  for (l = 0; l < num_lists; l++)
    {
      head[l] = /*(SortedList_t*)*/ malloc(sizeof(SortedList_t));
      if (head[l] == NULL)
	{ 
	  fprintf(stderr, "Error allocating for thread list\n");
	  exit(1);
	}
      //Head has a key of NULL
      head[l]->key = NULL;
      head[l]->next = NULL;
      head[l]->prev = NULL;
    }

  //List of nodes to be inserted into thread list
  the_list = malloc((sizeof(SortedListElement_t) * threads * iterations));
  if (the_list == NULL)
    {
      fprintf(stderr, "Error allocating for pre-thread elements\n");
      exit(1);
    }

  //Set of keys to be inserted into the_list
  key = malloc(sizeof(char*) * threads * iterations);
  if (key == NULL)
    {
      fprintf(stderr, "Error creating keys\n");
      exit(1);
    }

  int i;
  
  for (i = 0; i < (threads*iterations); i++)
    {
      //Put random characters into keys
      key[i] = malloc(sizeof(char*) * key_len);
      make_key(key[i]);
    }

  for (i = 0; i < (threads*iterations); i++)
    {
      //Put keys into pre-thread list of elements
      the_list[i].key = key[i];
      the_list[i].prev = NULL; //Do not need to be linked
      the_list[i].next = NULL;
    }

  //Set up locking
  int m;
  
  switch (lock_type)
    {
    case 0: //Un-protected, no locks to set
      break;
    case 1: //Mutex initialization
      mutex = malloc(sizeof(pthread_mutex_t) * num_lists);
      if (mutex == NULL)
	{
	  fprintf(stderr, "Error mallocing mutexes\n");
	  exit(1);
	}
      for (m = 0; m < num_lists; m++)
	pthread_mutex_init(&mutex[m], NULL);
      break;
    case 2: //Test and Set initialization
      spin = malloc(sizeof(int) * num_lists);
      if (spin == NULL)
	{
	  fprintf(stderr, "Error mallocing spin locks\n");
	  exit(1);
	}
      for (m = 0; m < num_lists; m++)
	spin[m] = 0;
      break;
    }

  //Free everything after program exits
  atexit (free_structures);
  
  return;
}

//Main thread function
void thread_function(void* arg)
{
  int thread = (intptr_t) arg;
  int i, del;
  int length = 0;

  for (i = 0; i < num_it; i++)
    {
      //Find thread specific spot in the_list
      int spot = (thread * num_it) + i;
      //Find hash index
      int index = hash(the_list[spot].key);
      
      switch (lock_type)
	{
	case 0: //No locks
	  SortedList_insert(head[index], &the_list[spot]);
	  break;
	case 1: //Mutex
	  pthread_mutex_lock(&mutex[index]); //Lock specific mutex by index
	  SortedList_insert(head[index], &the_list[spot]);
	  pthread_mutex_unlock(&mutex[index]);
	  break;
	case 2: //Test and Set
	  while(__sync_lock_test_and_set (&spin[index], 1)); //Lock specific spin by index
	  SortedList_insert(head[index], &the_list[spot]);
	  __sync_lock_release (&spin[index]);
	  break;
	default:
	  break;
	}
    }
  
  int s;
  
  switch (lock_type)
    {
    case 0: //No locks
      for (s = 0; s < num_lists; s++)
	length += SortedList_length(head[s]);
      break;
    case 1: //Mutex
      for (s = 0; s < num_lists; s++) //Lock all sub-lists first
	pthread_mutex_lock(&mutex[s]);
      for (s = 0; s < num_lists; s++)
	length += SortedList_length(head[s]); //Add lengths
      for (s = 0; s < num_lists; s++)
	pthread_mutex_unlock(&mutex[s]); //Unlock all sub-lists
      break;
    case 2: //Test and Set
      for (s = 0; s < num_lists; s++)
	while(__sync_lock_test_and_set (&spin[s], 1));  //Lock all sub-lists
      for (s = 0; s < num_lists; s++)
	length += SortedList_length(head[s]); //Add lengths
      for (s = 0; s < num_lists; s++)
	__sync_lock_release (&spin[s]); //Unlock all sub-lists
      break;
    default:
      break;
    }

  for (i = 0; i < num_it; i++)
    {
      //Find thread specific spot in the_list
      int spot = (thread * num_it) + i;
      //Find hash index
      int index = hash(the_list[spot].key);
      
      switch (lock_type)
	{
	case 0: //No locks
	  pick = SortedList_lookup(head[index], key[spot]);
	  del = SortedList_delete(pick);
	  break;
	case 1: //Mutex
	  pthread_mutex_lock(&my_mut);
	  pthread_mutex_lock(&mutex[index]); //Lock specific mutex by index
	  pick = SortedList_lookup(head[index], key[spot]);
	  del = SortedList_delete(pick);
	  pthread_mutex_unlock(&mutex[index]);
	  pthread_mutex_unlock(&my_mut);
	  break;
	case 2: //Test and Set
	  while(__sync_lock_test_and_set (&my_spin, 1));
	  while(__sync_lock_test_and_set (&spin[index], 1)); //Lock specific mutex by index
	  pick = SortedList_lookup(head[index], key[spot]);
	  del = SortedList_delete(pick);
	  __sync_lock_release (&spin[index]);
	  __sync_lock_release (&my_spin);
	  break;
	default:
	  break;
	} 
    }
}


//Main function
int main(int argc, char* argv[])
{

  srand (time(0)); //For rand()
  int c;
  int j, con = 0;
  int sync = 0;
  char* a = NULL;
  char* y = NULL;
  
  while (1) //Getopt for options
    {
      static struct option long_options[] =
	{
	  {"threads", required_argument, NULL, 't'},
	  {"iterations", required_argument, NULL, 'i'},
	  {"sync", required_argument, NULL, 's'},
	  {"yield", required_argument, NULL, 'y'},
	  {"lists", required_argument, NULL, 'l'},
	  {0, 0, 0, 0}
	};

      int option_index = 0;
      c = getopt_long(argc, argv, "", long_options, &option_index);

      if (c == -1) //No more options, break
	break;

      if (optopt) //If missing argument, optopt will be non-zero
	exit(1);
	  

      switch (c)
	{
	case 't': //Threads
	  num_threads = atoi(optarg);
	  if (!num_threads)
	    {
	      fprintf(stderr, "Error, missing argument for threads\n");
	      exit(1);
	    }
	  break;
	case 'i': //Iterations
	  num_it = atoi(optarg);
	  if (!num_it)
	    {
	      fprintf(stderr, "Error, missing argument for iterations\n");
	      exit(1);
	    }
	  break;
	case 's': //Sync options
	  if (sync) //If sync option already provided, error
	    {
	      fprintf(stderr, "Error, two sync modes provided\n");
	      exit(1);
	    }
	  a = optarg;
	  if (*a == '\0')
	    {
	      fprintf(stderr, "Error, missing argument for sync\n");
	      exit(1);
	    }
	  if (*a == 'm')
	    lock_type = 1;
	  else if (*a == 's')
	    lock_type = 2;
	  sync = 1; //Do not allow another sync option
	  break;
	case 'y': //Yield option
	  y = optarg;
	  if (*y == '\0')
	    {
	      fprintf(stderr, "Error, missing argument for yield\n");
	      exit(1);
	    }
	  con = 1;
	  while (con)
	    {
	      switch(y[j])
		{
		case 'i':
		  opt_yield |= INSERT_YIELD;
		  break;
		case 'd':
		  opt_yield |= DELETE_YIELD;
		  break;
		case 's':
		  opt_yield |= SEARCH_YIELD;
		  break;
		default:
		  con = 0; //No more arguments, end loop
		}
	      j++;
	    }
	  break;
	case 'l': //Lists option
	  num_lists = atoi(optarg);
	  if (!num_lists)
	    {
	      fprintf(stderr, "Error, missing argument for lists\n");
	      exit(1);
	    }
	  break;
	default:
	  break;
	}
    }

  create_lists(num_threads, num_it); //Create empty linked list

  struct timespec start, stop;
  uint64_t t_time; //Total time from start to stop

  if ( clock_gettime(CLOCK_MONOTONIC, &start) == -1) //Start clock timer
    {
      fprintf(stderr, "Clock gettime error at start");
      exit(1);
    }

  int i, threads;
  
  pthread_t tid[num_threads]; //Thread array

  for (i = 0; i < num_threads; i++)
    {
      threads = pthread_create(&(tid[i]), NULL, (void*) &thread_function, (void*) (intptr_t) i);
      if (threads)
	{
	  fprintf(stderr, "Error creating thread number: %d\n", i);
	  exit(1);
	}
    }

  for (i = 0; i < num_threads; i++)
    {
      threads = pthread_join((tid[i]), NULL);
      if (threads)
	{
	  fprintf(stderr, "Error joining thread number: %d\n", i);
	  exit(1);
	}
    }

  if ( clock_gettime(CLOCK_MONOTONIC, &stop) == -1) //Stop clock timer
    {
      fprintf(stderr, "Clock gettime error at end");
      exit(1);
    }

  //Final time for program
  t_time = ((CLOCK_PRECISION * (stop.tv_sec - start.tv_sec)) + (stop.tv_nsec - start.tv_nsec));

  int ops = 0;
  ops = (num_threads * num_it) * 2; //Number of operations
  
  int per_op = 0;
  per_op = t_time / ops; //Cost/time per operation

  
  //Corrected cost per operation for graph
  //per_op /= ((num_threads * num_it) / num_lists);
  
  //Report final data
  printf("%d threads x %d iterations x (insert + lookup/delete) = %d operations\n", num_threads, num_it, ops);
  int len = 0;
  int z;
  for (z = 0; z < num_lists; z++)
    len += SortedList_length(head[z]);
  if (len != 0)
    fprintf(stderr, "ERROR: final length = %d\n", len);
  printf("elapsed time: %lluns\n", (long long unsigned int) t_time);
  printf("per operation: %dns\n", per_op);
  
  if (len != 0)
    exit(1); //Exit with non-zero status if error in list length

  exit(0); //No errors with length
}
      
  
	    
	    
      
  
  
