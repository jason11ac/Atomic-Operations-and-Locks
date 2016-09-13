# Atomic-Operations-and-Locks
Project for my CS 111 Class

~~~~~~~                 	                            Jason Alvarez-Cohen
README                          	                    UID: 504487052
~~~~~~~

This tarball contains 6 items:

     1. The header file SortedList.h that contains interfaces for each linked list operation
     2. The C source file SortedList.c that implements the interfaces in SortedList.h
     3. The C file lab2c.c that compiles and creates the executable lab2c
     4. The makefile that can be used with certain options to complete a number of tasks:
          - make:       Makes the lab2c executable using SortedList.c and lab2c.c
          - make clean: Removes the makefile created executable, lab2c
          - make dist:  Build the tarball that contains all 7 items described
     5. A graph that shows:
          - 2C_Graph1: (corrected) average time per operation (for none, mutex, and spin-lock)
	      	       vs the ratio of threads per list.
     6. This README file, which describes what everything in this final
        tarball is and contains answers to the project questions below.


The executable created by the makefile, lab2c, can be used with specific arguments:

        --threads=<numberofthreads>: This option takes a required parameter for the number of parallel
                                     threads. The default if no threads option is given is one.

        --iterations=<numberofiterations>: This option takes a required parameter for the number of
                                           iterations. The default if no iterations option is given
                                           is one.

        --lists=<numberoflists>: This option breaks the single sorted list into the specified
	                         number of sub-lists. (each with its own list header and synchronization
				 object) The default if no lists option is given is one. 

        --yield: This option takes any combination of the following three parameters, which will
		 enable critical section pthread_yield()s in SortedList functions:
		 'i': Enables yields during the insert critical section
		 'd': Enables yields during the delete critical section
		 's': Enables yields during the lookup and length critical sections

        --sync=<lock_type>: This option takes only one of two parameters:
                            - 'm': Protect SortedList functions using a pthread_mutex lock per list
                            - 's': Protect SortedList functions using a spin-lock per list. This uses the
			      	   builtin __sync_lock_test_and_set and __sync_lock_release functions

The executable can be used with any combination of the options above. If more than one sync options
are provided, an error occurs.

QUESTIONS:

2C.1A:
	To study the performance of the three methods as a function of the number of threads per list,
	we can derive an equation for the correct average cost per operation for each. First, we can
	calculate the non-corrected cost per operation by dividing the total time of the program by the
	number of operations (threads * iterations * 2). To calculate the correct cost per operation we
	use the equations below:

	    Avg_Cost_per_Op(Un) = (Thread_Creation + (Time_Thread * Num_of_Ops)) / Num_of_Ops ->
	    Avg_Cost_per_Op(Un) = (Thread_Creation / Num_of_Ops) + Time_Thread

	    	where Time_Thread is the total time for each thread to insert, find length, lookup
		and delete each node assigned to itself into Num_of_lists

	    Avg_Cost_per_Op(Cor) = Avg_Cost_per_Op(Un) / list_size
	    
		where (list_size == Num_of_elements / Num_of_lists)

	With the single thread unprotected case, changing the number of lists should not have a
	big performance effect as no thread/context switching is occuring. Although, there might be
	a slight increase in performance as the thread does not have to iterate through one big list
	to reach the end, it now has a number of shorter lists to add to. With the mutex and spin lock
	methods, if the thread to list ratio is relatively small, there are less collisions and conflicts
	as there are less threads updating a certain list at the same time. Less collisions and
	less locking/spinning means increased performance.


2C.1B:
	For measuring performance in this particular case, threads per list is more interesting than
	just threads because the added option of having sub-lists allows for less thread collisions
	as all the threads are spread between a number of lists rather than just one like in 2B. This
	decrease in collisions improves performance as there is less locking/spinning/blocking.

GPROF:

Single list, single thread, 10k - 100k iterations:

	--threads=1 --iterations=10000 --lists=1 :

	 %   cumulative   self              self     total
	  time   seconds   seconds    calls  us/call  us/call  name
	  53.92      0.07     0.07    10000     7.01     7.01  SortedList_insert
	  46.22      0.13     0.06    10000     6.01     6.01  SortedList_lookup
	  0.00       0.13     0.00    20000     0.00     0.00  hash
	  0.00       0.13     0.00    10000     0.00     0.00  SortedList_delete
          0.00       0.13     0.00    10000     0.00     0.00  make_key
	  0.00       0.13     0.00        2     0.00     0.00  SortedList_lengh
          0.00       0.13     0.00        1     0.00     0.00  create_lists

	--threads=1 --iterations=20000 --lists=1 :

	 %   cumulative   self              self     total
	  time   seconds   seconds    calls  us/call  us/call  name
	  54.79      0.29     0.29    20000    14.52    14.52  SortedList_insert
	  43.45      0.52     0.23    20000    11.52    11.52  SortedList_lookup
	  1.89       0.53     0.01                             thread_function
	  0.00       0.53     0.00    40000     0.00     0.00  hash
	  0.00       0.53     0.00    20000     0.00     0.00  SortedList_delete
	  0.00       0.53     0.00    20000     0.00     0.00  make_key
	  0.00       0.53     0.00        2     0.00     0.00  SortedList_length
	  0.00       0.53     0.00        1     0.00     0.00  create_lists

	--threads=1 --iterations=50000 --lists=1 :
	
	  %   cumulative   self              self     total
	   time   seconds   seconds    calls  us/call  us/call  name
	   49.72      1.45     1.45    50000    29.04    29.04  SortedList_insert
	   49.72      2.90     1.45    50000    29.04    29.04  SortedList_lookup
	   0.69       2.92     0.02                             thread_function
	   0.00       2.92     0.00   100000     0.00     0.00  hash
           0.00       2.92     0.00    50000     0.00     0.00  SortedList_delete
	   0.00       2.92     0.00    50000     0.00     0.00  make_key
	   0.00       2.92     0.00        2     0.00     0.00  SortedList_length
	   0.00       2.92     0.00        1     0.00     0.00  create_lists

	--threads=1 --iterations=100000 --lists=1 :

	  %   cumulative   self              self     total
	   time   seconds   seconds    calls  us/call  us/call  name
	   51.58      7.50     7.50   100000    75.00    75.00  SortedList_lookup
	   48.28     14.52     7.02   100000    70.19    70.19  SortedList_insert
	   0.14      14.54     0.02   200000     0.10     0.10  hash
	   0.14      14.56     0.02                             thread_function
	   0.00      14.56     0.00   100000     0.00     0.00  SortedList_delete
	   0.00      14.56     0.00   100000     0.00     0.00  make_key
	   0.00      14.56     0.00        2     0.00     0.00  SortedList_length
	   0.00      14.56     0.00        1     0.00     0.00  create_lists

	    - As we can see in all three runs, most of the time is spent in SortedList_lookup
	      and SortedList_insert as these functions need to iterate through the lists to
	      complete their tasks, which takes time with a single list.
	    - As expected the number of calls for each sorted list function is threads * iterations
	      with length being an exception as that is called once per list number, and then again
	      at the end of the program for error checking. Create_lists is only called once to
	      create all the necessary data structures.
	    - Time spent on each function per call is shown above under the self us/call column. It
	      seems that as the number of iterations increases, so does the self us/call. 

8 threads, 10k iterations, 64 - 1 lists, mutex lock:

  	--threads=8 --iterations=10000 --lists=64 --sync=m :

	  %   cumulative   self              self     total
	   time   seconds   seconds    calls  ms/call  ms/call  name
	   55.63      0.10     0.10    61762     0.00     0.00  SortedList_insert
	   33.38      0.16     0.06    79933     0.00     0.00  SortedList_lookup
	   5.56       0.17     0.01      576     0.02     0.02  SortedList_length
	   5.56       0.18     0.01        1    10.01    10.01  create_lists
	   0.00       0.18     0.00   140696     0.00     0.00  hash
	   0.00       0.18     0.00    80000     0.00     0.00  make_key
	   0.00       0.18     0.00    79809     0.00     0.00  SortedList_delete

	--threads=8 --iterations=10000 --lists=16 --sync=m :

	  %   cumulative   self              self     total
	   time   seconds   seconds    calls  ms/call  ms/call  name
	   55.48      0.41     0.41    74835     0.01     0.01  SortedList_insert
	   41.95      0.72     0.31    79977     0.00     0.00  SortedList_lookup
	   1.35       0.73     0.01        1    10.01    10.01  create_lists
	   1.35       0.74     0.01                             thread_function
	   0.00       0.74     0.00   154294     0.00     0.00  hash
	   0.00       0.74     0.00    80000     0.00     0.00  make_key
	   0.00       0.74     0.00    79978     0.00     0.00  SortedList_delete
	   0.00       0.74     0.00      144     0.00     0.00  SortedList_length

	--threads=8 --iterations=10000 --lists=4 --sync=m :

	  %   cumulative   self              self     total
	   time   seconds   seconds    calls  us/call  us/call  name
	   53.24      1.26     1.26    79121    15.95    15.95  SortedList_insert
	   46.05      2.35     1.09    80000    13.64    13.64  SortedList_lookup
	   0.42       2.36     0.01       36   278.15   278.15  SortedList_length
	   0.42       2.37     0.01                             thread_function
	   0.00       2.37     0.00   159065     0.00     0.00  hash
	   0.00       2.37     0.00    80000     0.00     0.00  SortedList_delete
	   0.00       2.37     0.00    80000     0.00     0.00  make_key
	   0.00       2.37     0.00        1     0.00     0.00  create_lists

	--threads=8 --iterations=10000 --lists=1 --sync=m :

	  %   cumulative   self              self     total
	   time   seconds   seconds    calls  us/call  us/call  name
	   50.07      4.16     4.16    79990    51.95    51.95  SortedList_insert
	   50.07      8.31     4.16    80000    51.94    51.94  SortedList_lookup
	   0.00       8.31     0.00   159970     0.00     0.00  hash
	   0.00       8.31     0.00    80000     0.00     0.00  SortedList_delete
	   0.00       8.31     0.00    80000     0.00     0.00  make_key
	   0.00       8.31     0.00        9     0.00     0.00  SortedList_length
	   0.00       8.31     0.00        1     0.00     0.00  create_lists

	   - Like in the --lists=1 tests, the sorted list insert and lookup functions
	     take up most of the time of the program, but if we look at the tests with
	     a high number of lists (e.g. 64, 16, 8) we can see that SortedList_length
	     and create_lists both take a significantly larger amount of the total program
	     time. This is because length is being called on every list, making it take
	     more time and create_lists must create all n number of lists, which also
	     takes an increased amount of time.
	   - Appearently, the gprof call counter is not thread safe, so the number of calls
	     for each function in these tests are incorrect.
	   - Once again, the time on each function per call is listed under self us/call.
	     But because the number of calls is inaccurate, this value is also inaccurate as
	     it is calculated by dividing total self seconds by the number of calls. 

8 threads, 10k iterations, 64 - 1 lists, spin-lock:

        --threads=8 --iterations=10000 --lists=64 --sync=s : 

	  %   cumulative   self              self     total
	   time   seconds   seconds    calls  ms/call  ms/call  name
	   93.10      4.38     4.38                             thread_function
	   4.05       4.57     0.19    79948     0.00     0.00  SortedList_lookup
	   2.13       4.67     0.10    56885     0.00     0.00  SortedList_insert
	   0.21       4.68     0.01   140187     0.00     0.00  hash
	   0.21       4.69     0.01    79985     0.00     0.00  SortedList_delete
	   0.21       4.70     0.01      576     0.02     0.02  SortedList_length
	   0.21       4.71     0.01        1    10.01    10.01  create_lists
	   0.00       4.71     0.00    80000     0.00     0.00  make_key
	
	--threads=8 --iterations=10000 --lists=16 --sync=s :

	  %   cumulative   self              self     total
	   time   seconds   seconds    calls  us/call  us/call  name
	   93.81     17.09    17.09                             thread_function
	   3.90      17.80     0.71    79931     8.89     8.89  SortedList_lookup
	   2.31      18.22     0.42    61051     6.89     6.89  SortedList_insert
	   0.05      18.23     0.01    79996     0.13     0.13  SortedList_delete
	   0.05      18.24     0.01      144    69.54    69.54  SortedList_length
	   0.00      18.24     0.00   152745     0.00     0.00  hash
	   0.00      18.24     0.00    80000     0.00     0.00  make_key
	   0.00      18.24     0.00        1     0.00     0.00  create_lists
	
	--threads=8 --iterations=10000 --lists=4 --sync=s :

	  %   cumulative   self              self     total
	   time   seconds   seconds    calls  us/call  us/call  name
	   97.61     90.70    90.70                             thread_function
	   1.51      92.10     1.40    73521    19.07    19.07  SortedList_insert
	   0.99      93.02     0.92    79939    11.52    11.52  SortedList_lookup
	   0.02      93.04     0.02    79998     0.25     0.25  SortedList_delete
	   0.00      93.04     0.00   158677     0.00     0.00  hash
	   0.00      93.04     0.00    80000     0.00     0.00  make_key
	   0.00      93.04     0.00       36     0.00     0.00  SortedList_length
	   0.00      93.04     0.00        1     0.00     0.00  create_lists
	
	--threads=8 --iterations=10000 --lists=1 --sync=s :

	  %   cumulative   self              self     total
	   time   seconds   seconds    calls  us/call  us/call  name
	   99.76    652.87   652.87                             thread_function
	   0.24     654.44     1.57    78896    19.93    19.93  SortedList_insert
	   0.14     655.33     0.89    79900    11.15    11.15  SortedList_lookup
	   0.00     655.33     0.00   159888     0.00     0.00  hash
	   0.00     655.33     0.00    80000     0.00     0.00  SortedList_delete
	   0.00     655.33     0.00    80000     0.00     0.00  make_key
	   0.00     655.33     0.00        9     0.00     0.00  SortedList_length
	   0.00     655.33     0.00        1     0.00     0.00  create_lists

	   - With spin locks, thread_function all of a sudden becomes the majority
	     of the program time, no matter how many lists are used. This is because
	     spin-locks force the CPU to spin while it waits to complete tasks, which
	     in turn makes thread_function take a lot more CPU and wall time. After
	     thread_function, the insert and lookup functions are 2nd and 3rd in time
	     percentage.
	   - Again, the counter for calls is not thread safe, making the number of
	     calls per function inaccurate.
	   - Once again, the time on each function per call is listed under self us/call.
	     But because the number of calls is inaccurate, this value is also inaccurate as
	     it is calculated by dividing total self seconds by the number of calls.


2C.2A:
	Looking at the gprof data above, as the number of lists increase, the self time of
	each operation and total program time decreases almost exponentially. This can be
	seen with both spin and mutex locking. Increasing the number of lists improves
	speed and performance because it allows for less thread blocking/conflicts and
	shorter lists overall. Shorter lists means less iterations for the insert and
	lookup sorted list functions, which are usually the main time hogs of the program.

2C.2B:
	If we compare the times for mutex vs. spin-lock we can see that clearly using a mutex
	lock is substantially faster, no matter how many lists are being used. The reason for
	this is thread_function becomes a huge time hog with spin-locks because of all the
	spinning and waiting that happens with threads.

2C.3A:
	The mutex must be held when pthread_cond_wait is called because if there is no mutex,
	then an atomic call to wait can not be guaranteed. Not having a mutex could cause
	the thread to eternally sleep as the condition variable could be changed right before
	the pthread_cond_wait puts the thread to sleep. If this happens, the thread is waiting
	to be woken up by a certain signal that has already been sent right before the thread
	was put to sleep.

2C.3B:
	If the waiting thread becomes blocked and does not release the mutex, the blocked thread
	will be put to sleep while still holding the mutex, which does not allow other threads
	to use whatever variables/data/memory that is in the locked mutex section. This could lead
	to program deadlock as the sleeping thread is waiting for a certain event, but other threads
	can not proceed as the sleeping thread still holds the mutex which would allow thread
	progress.

2C.3C:
	The mutex must be reacquired because once pthread_cond_wait returns, we could still be
	executing instructions in a mutually exclusive section that requires locking to avoid
	race conditions. This is usually the case. 

2C.3D:
	If the mutex were to be released before calling pthread_cond_wait, another thread could acquire
	the mutex in between the mutex release and the call to wait. This could lead to horrible outcomes
	such as deadlock as the pthread_cond_wait may be waiting on a condition variable, which could be
	updated right before pthread_cond_wait is called. In this case, the condition variable is changed
	and a signal occurs before wait is called, which would put the waiting thread to sleep forever.
	The releasing of the mutex and call to wait must be atomic, which is why these tasks are done with
	a single line of code.

2C.3E:
	Pthread_cond_wait can not be implemented in user mode because preemption can not be prevented in
	user mode. For the release/sleep/lock actions to be atomic, there needs to be no preemption, which
	can only be done in kernel mode. 
	
	
	     

	
