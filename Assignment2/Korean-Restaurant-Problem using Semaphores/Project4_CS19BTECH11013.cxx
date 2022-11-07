//header files
#include <iostream>
#include <thread>
#include <random>
#include <fstream>
#include <semaphore.h>
#include <unistd.h>
#include <sys/time.h>


using namespace std;


//declaration of functions
double expo_dist(double lam);

int uniform(int s);

void threadFunc(int i,int X,float T);   //Thread function


//global variables
int eating=0;    //for counting number of customers eating at the table

int waiting=0;   //for counting number of customers waiting

double waiting_time=0;   //for calculating total waiting time of each customer

sem_t mutex,block,print;  //semaphores

bool must_wait=false;   //To know if any group has been formed

ofstream Outfile;


//main function

int main()
{
    int N,X;
    
    double r,lam,T;
    
    ifstream Infile;             //File pointers
	
	Infile.open("input.txt");    //opening 'inp-params.txt' file in read mode
	
	if(!Infile) //ERROR condition 
	{
		cerr << "ERROR: File could not be opened" << endl;
		
		exit(1);
	}
	
	Outfile.open("Logfile.txt");     //opening 'EDF-Log.txt' file in write mode
	
	if(!Outfile)   //ERROR condition 
	{
		cerr << "ERROR: File could not be opened" << endl;
		
		exit(1);
	}
    
    Infile >> N >> X >> lam >> r >> T;   
    
    int i=0;
    
    int ret1=sem_init(&mutex,0,1);   //Intialising mutex with '1'
    
    if(ret1!=0)
    {
		cout << "Unsuccessful" << endl;
	}
	
	int ret2=sem_init(&block,0,0);   //Initialising block with'0'
    
    if(ret2!=0)
    {
		cout << "Unsuccessful" << endl;
	}
	
	int ret3=sem_init(&print,0,1);   //Initialising block with'0'
    
    if(ret3!=0)
    {
		cout << "Unsuccessful" << endl;
	}
	
    std::thread threads[N];   //N threads (one thread for each customer)
    
    while(i<N)
    {
		int max=r*X;    

		int set=uniform(max);   //calling the function 'uniform' to uniformly select a set from 1 to r*X (X is no of seats at the table)
	
		if(set>N-i)
		{
			set=N-i;
		}
		
		for(int j=0;j<set;j++)
		{
			threads[i]=std::thread(threadFunc,i,X,T);//thread creation
			i++;
		}
		
		//exponential dealy between set of customers
		double delay=expo_dist(lam)/1000;   //(in milliseconds)
		
		sleep(delay);
		
	}
	
	for(int j=0;j<N;j++)
	{
		threads[j].join();
	}
	
	double Avg_wait=(float)waiting_time*1000000/N;
	Outfile << "Average wait time for each customer is " << Avg_wait  << " micro seconds" << endl;
	
	return 0;
}


//This function is to find exponential delay with lam as parameter
double expo_dist(double lam)
{
	unsigned temp = chrono::system_clock::now().time_since_epoch().count();
	
	default_random_engine generator(temp);
	
    exponential_distribution <double> distribution(lam);
    
    return distribution(generator);
}

//This function is selected a set uniformly from 1 to 'r*X'
int uniform(int s)
{
	unsigned temp = chrono::system_clock::now().time_since_epoch().count();
	
	default_random_engine generator(temp);
	
    uniform_int_distribution <int> distribution(1,s);
    
    return distribution(generator);
}


//Thread function
void threadFunc(int i,int X,float T)
{
	
	struct tm*reqTime,*accessTime;
	
	struct timeval start,end;
	
	using namespace chrono;
		
	auto now = system_clock::now();
		
	auto milli=duration_cast<milliseconds>(now.time_since_epoch())%1000;
		
	auto timer= system_clock::to_time_t(now);
		
	reqTime=localtime(&timer);
	
	//Customer requesting access to a seat as soon as he enters the restaurant
	//printing time in HH:MM:SS:mmm
	 sem_wait(&print);   
	 
	Outfile <<  i+1 << "th Customer access request at " << reqTime->tm_hour << ":" << reqTime->tm_min << ":" << reqTime->tm_sec  << ":" << milli.count() <<endl;
	
	gettimeofday(&start,NULL);
	
	sem_post(&print);
	
	sem_wait(&mutex);   
	
	//If a group is present
	if(must_wait==true|| eating+1>X)
	{
		waiting++;   //customer waiting
		
		must_wait=true;
		
		sem_post(&mutex);
		
		sem_wait(&block);   //customer waiting untill all other customers in the group completes eating
	}
	else
	{
		eating++;   //customer starts eating
	
		if( waiting>0 && eating == X)
		{
			must_wait=true;
		}
		else
		{
			must_wait=false;
		}
		
		sem_post(&mutex);
	}
	
		
    //Customer eats at the table
    sem_wait(&print);   
    
	now = system_clock::now();
		
	milli=duration_cast<milliseconds>(now.time_since_epoch())%1000;
		
	timer= system_clock::to_time_t(now);
		
	accessTime=localtime(&timer);
    		
	//printing time in HH:MM:SS:mmm
	Outfile <<  i+1 << "th Customer given access at " << accessTime->tm_hour << ":" << accessTime->tm_min << ":" << accessTime->tm_sec  << ":" << milli.count() << endl;
	
	gettimeofday(&end,NULL);
		
	double wait= end.tv_sec-start.tv_sec +(end.tv_usec-start.tv_usec)*0.000001;
		
	waiting_time=waiting_time+wait;  //calculating waiting time of the customer
	
	sem_post(&print);
	
	double lambda=1/T;
	
	//eating time for the customer
	double eat_time=expo_dist(lambda)/1000;   //(in milliseconds)
	sleep(eat_time);
	
    sem_wait(&mutex);
    
    //Finishes eating
	eating--;
	
	/*
	now = system_clock::now();
		
    milli=duration_cast<milliseconds>(now.time_since_epoch())%1000;
		
	timer= system_clock::to_time_t(now);
	
	struct tm*ExitTime;
		
	ExitTime=localtime(&timer);
		
	//printing time in HH:MM:SS:mmm
	Outfile <<  i+1 << "th Customer exit at " << ExitTime->tm_hour << ":" << ExitTime->tm_min << ":" << ExitTime->tm_sec  << ":" << milli.count() << endl;
	*/
	
	//After a group completes eating
	if(eating == 0)
	{
		int k = min (X , waiting );   //choosing number of customers to be seated at the table
		
		waiting -= k;
		
        eating += k;
        
        //Finding if any group has formed or not
        if( waiting>0 && eating == X)
		{
			must_wait=true;
		}
		else
		{
			must_wait=false;
		}
	    
	    for(int i=0;i<k;i++)
	    {
			sem_post(&block);
		}	
	}

    sem_post(&mutex);
    
   
}
