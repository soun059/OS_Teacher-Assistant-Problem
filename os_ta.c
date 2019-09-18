#include <pthread.h>		//Create thread of POSIX
#include <time.h>			//to make waiting time a random one
#include <unistd.h>			// to call the sleep function
#include <semaphore.h>		//to create semaphores
#include <stdlib.h>			//standard library to call malloc,calloc,etc
#include <stdio.h>			//standard library for input,output

pthread_t *Studs;		// creating a pthread variable for students for thread of arrival of students
pthread_t TA;			//	creating a pthread variable TA waiting/sleeping to help the students

int ccount = 0;        	// count of no. of chairs occupied
int ci = 0;				//storing the current index on chairs

//Declaration of mutex and semaphore variables
sem_t TA_Sleep;        	//semaphore variable to denote that TA is sleeping
sem_t ssem;				//semaphore varible for student
sem_t csem[3];			//semaphore variable for the chairs (occupied or not)
pthread_mutex_t CA;		//mutex for getting the chair access

int ns = 0;	//to get the no. of students required for check function
int *ns_done; //to check the completion status of each students' job --- 1 = work done 0 = work left to done

int check() //check function to find the completion of students' queries
{
	for(int i=0;i<ns;i++) 
	{
		if(ns_done[i]==0) 
			return 0;    //returning zero if job of that particuler is not done
	}
	return 1;  //returning zero if job of all particuler are done
}

void *TA_W() //main operation of TA which needs to be binded with the TA thread
{
	while(1)
	{
		sem_wait(&TA_Sleep);		//waiting for any student to come
		if(check()==1)
			return NULL;
		printf("####################Student awakens TA#######################\n");  //if TA is awoken by student for queries
		while(1)
		{
			pthread_mutex_lock(&CA); 		//to block any chair access
			if(ccount == 0) 
			{	
				pthread_mutex_unlock(&CA);  //once chair occupied is zero then opens all chair access and breaks the loop to sleep
				break;
			}
			sem_post(&csem[ci]); //to clear the semaphore for chair occupied at ci which is now empty : cpost clears the semaphore variable
			ccount--; //decreasing the count to get more students to get chairs
			printf("Student left his chair. Remaining Chairs %d\n", 3 - ccount); 
			ci = (ci + 1) % 3;	//finding the new current index
			pthread_mutex_unlock(&CA); //unlocking the chair access
			printf("\t TA is helping the student\n"); 
			sleep(5); 
			sem_post(&ssem); //clearing the students semaphore to done work
			usleep(1000); //make it sleep to avoid clumsiness of output
		}
	}
}
//the main operation of students to perform
void *Stud_w(void *threadID) 
{
	int PT;
	while(1)
	{
		if(ns_done[(long)threadID] == 1) //check whether work of student is done
			return NULL;
		printf("Student %ld is doing work\n", (long)threadID); //student has entered into the loop and working for assignment
		PT = rand() % 10 + 1; //creating a random time
		printf("waiting for :%d\n",PT);
		sleep(PT);		// waiting to check his task
		printf("Student %ld needs help from the TA\n", (long)threadID); //student needs queries to resolve fron TA
		pthread_mutex_lock(&CA); //to lock the chair access as he is entering
		int count = ccount; //getting the ccount into a local variable avoiding any changes in the global one
		pthread_mutex_unlock(&CA); //unlocking the access of chairs

		if(count < 3)		// if chairs still left to occupy
		{
			if(count == 0)		
				sem_post(&TA_Sleep); //resetting the TA_SLEEP as he is the first student to attend him and awakes him
			else
				printf("Student %ld sat on a chair waiting for the TA to finish. \n", (long)threadID); //students sat on the next chair to wait
			pthread_mutex_lock(&CA); //locks the chair access
			int index = (ci + ccount) % 3; //changing the current index and storing it locally
			ccount++; //increasing the total chairs occupied value
			printf("Student sat on chair for waiting .Chairs Remaining: %d\n", 3 - ccount);
			pthread_mutex_unlock(&CA); //unlocks the chair access

			sem_wait(&csem[index]);		//makes the student busy
			printf("\tTA is helping Student %ld  \n", (long)threadID); 
			sem_wait(&ssem);	//makes the semaphore of student busy so that no new student can be allowed to ask
			printf("###################@@@@@@@==> Help has been done.Student %ld left TA room.\n",(long)threadID);
			//pthread_cancel(threadID);
			ns_done[(long)threadID] = 1; //changing the status of student job
			if(check()==1) //checking if all jobs are completed or not
				exit(0);
		}
		else 
			printf("Student %ld will return at another time. \n", (long)threadID); //if all chairs are full then it comes afterwards
	}
}

int main(int argc, char* argv[])
{
	int n_s;		
	int id;
	srand(time(NULL)); //to set the pseudo random no. 

	
	sem_init(&TA_Sleep, 0, 0); //to initialize the TA_Sleep semaphore -- fyi: 0 given in pthread so that it can be applied to various threads
	sem_init(&ssem, 0, 0);  //to initialize the Student semaphore 
	for(id = 0; id < 3; ++id)			
		sem_init(&csem[id], 0, 0);  //to initialize the chair semaphore

	pthread_mutex_init(&CA, NULL);
	
	if(argc<2)
	{
		printf("Number of Students not given. Please enter the no of students:");
		scanf("%d",&n_s); //collecting the no of students if not given in cmdline argument
		ns = n_s;
	}
	else
	{
		printf("Number of Students Given. Creating %d threads.\n", n_s);
		n_s = atoi(argv[1]);
		ns = n_s;
	}
		
	Studs = (pthread_t*) malloc(sizeof(pthread_t)*n_s); //creating threads of given number of students
	ns_done = (int *)calloc(n_s,sizeof(int));  //creating a status array to find completion of program
	
	pthread_create(&TA, NULL, TA_W, NULL);	 //creating a thread link to link the TA thread variable to the TA_W function
	for(id = 0; id < n_s; id++)
		pthread_create(&Studs[id], NULL, Stud_w,(void*) (long)id); //creating a thread to link the Student threads to the Stud_w function

	
	pthread_join(TA, NULL); //now letting the TA thread to join the process system
	for(id = 0; id < n_s; id++)
		pthread_join(Studs[id], NULL); //now letting the Student thread variables to join the process system

	
	free(Studs); //freeing the studs threads.
	return 0;
}


