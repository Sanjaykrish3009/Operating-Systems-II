//header files
#include <iostream>
#include <thread>
#include <random>
#include <time.h>
#include <semaphore.h>
#include <sys/time.h>
#include <vector>
#include <fstream>

using namespace std;


//declaration of functions

int present(vector<int>arr,int n);

void Type(vector<int>array);

void threadFunc(vector<int>array);

//global variables

bool **Mat,*type;  

int*colour;        

int V,Max=0;


//semaphores

sem_t mutex;   //for coarse grain locking (one global lock)



//main function

int main()
{	
	int P;
	
	ifstream Infile;             //File pointers
	
	ofstream Outfile;
	
	Infile.open("input_params.txt");    //opening 'input_params.txt' file in read mode
	
	if(!Infile) //ERROR condition 
	{
		cerr << "ERROR: File could not be opened" << endl;
		
		exit(1);
	}
	
	Outfile.open("output.txt");     //opening 'output.txt' file in write mode
	
	if(!Outfile)   //ERROR condition 
	{
		cerr << "ERROR: File could not be opened" << endl;
		
		exit(1);
	}
	
	Infile >> P >> V;   //reading number of partitions and number of vertices from input file
	
	
	Mat=(bool**)malloc(V*sizeof(bool*));  //Dynamically allocating memory for adjacency matrix
	
	colour=(int*)malloc(V*sizeof(int));   //Dynamically allocating memory for colour array 
	
	type=(bool*)malloc(V*sizeof(bool));   //Dynamically allocating memory for type array
	
	vector<vector<int>> part(P);   //part - for P partitions 

	srand(time(NULL));
	
	for(int i=0;i<V;i++)
	{
		Mat[i]=(bool*)malloc(V*sizeof(bool));
		
		colour[i]=0;    //Initially all vertices are uncoloured - '0'
	
	    int ran=rand()%P;   //Vertex randomly choosing a partition
		
		part[ran].push_back(i);
		
		for(int j=0;j<V;j++)
		{
			Infile >> Mat[i][j];    //reading adjacency matrix from input file
		    
		}
	}
	
	for(int i=0;i<P;i++)
	{
		Type(part[i]);    //calling the function 'Type' 
	}
	
    int ret1=sem_init(&mutex,0,1);   //Intialising mutex lock with '1'
    
    if(ret1!=0)
    {
		Outfile << "Unsuccessful mutex lock initialisation" << endl;
	}
	
	struct timeval start,end;
	
	gettimeofday(&start,NULL);   //calling the function 'gettimeofday' to know time before thread creation
	
	//COARSE GRAIN LOCKING
	
    std::thread threads[P];
    
    for(int i=0;i<P;i++)
    {
		threads[i]=std::thread(threadFunc,part[i]);
	}
	
	for(int i=0;i<P;i++)
    {
		threads[i].join();
	}
	
	gettimeofday(&end,NULL);   //calling the function 'gettimeofday' to know time after threads execution
	
	//Output for coarsegrain locking
	
	Outfile << "Coarse-grained Lock:" << endl;
	
	Outfile << "Number of colours used: " << Max << endl;
	
    double time= end.tv_sec-start.tv_sec +(end.tv_usec-start.tv_usec)*0.001;
    
    Outfile << "Time taken by the algorithm using coarse-grained lock: " << time << " Milliseconds" << endl;
    
    Outfile << "Colours:" << endl;
    
    for(int i=0;i<V;i++)
    {
		if(i!=V-1)
		{
			Outfile << "v" << i+1 << " - " << colour[i] << "," ;
		}
		else if(i==V-1)
		{
			Outfile << "v" << i+1 << " - " << colour[i] << endl ;
		}
	}
	
	Outfile << endl;
    
   
    
	return 0;
}


//defining the function 'present'
//This function is to know whether any integer is present in vector (of int) or not
int present(vector<int>arr,int n)
{
	int count=arr.size();
	
    for(int j=0;j<count;j++)
	{
		if(arr[j]==n)
		{
		    return 1;	//returning '1' if n is found
		}
	}
	
	return 0;   //returning '0' if not found
}

//defining the function 'Type'
//This function is to know whether a vertex is internal or external in its partition
void Type(vector<int>array)
{
    int count=array.size();
    
    for(int i=0;i<count;i++)
	{
		int j=0;
		
		while(j<V)
		{
			if(Mat[array[i]][j]==1)
			{
				if(present(array,j)==0)
				{
					break;
				}
			}
			j++;
		}
		
		if(j==V)
		{
			type[array[i]]=0;   //if type is '0' , it is internal vertex
		}
		else
		{  
			type[array[i]]=1;   //if type is '1' , it is external vertex
		}
	}
}

//defining the function 'threadFunc'
//This is a thread function for colouring vertices using coarse grain locking
void threadFunc(vector<int>array)
{
	bool Arr[V];   //Assuming a maximum of V colours are present
	
	for(int i=0;i<V;i++)
	{
		Arr[i]=true;   //initially making all colours available (i.e, true)
	}
	
	int count=array.size();
	
	//coloring each vertex present in the given partition sequentially
    for(int i=0;i<count;i++)
    { 
        int ver=array[i];
         
		if(type[ver]==0)   //if the vertex is internal
		{
			for(int j=0;j<V;j++)
			{
				if(Mat[ver][j]==1&&colour[j]!=0)
				{
					Arr[colour[j]-1]=false;   //the colours present in its adjacent vertices are made unavailble 
				}
			}
		     
		    //searching for colouring which is available
			for(int k=1;k<=V;k++)
		    {
			    if(Arr[k-1]==true)  
				{
				    colour[ver]=k;
					 
					if(k>Max)
					{
					   Max++;   //Max contains the number of colours used till now
					}
					 
					break;
				}
				 
				
		    }
		    
			for(int j=0;j<V;j++)
			{
				if(Mat[ver][j]==1&&colour[j]!=0)
				{
					Arr[colour[j]-1]=true;   //Now, making the colours present in its adjacent vertices available for next vertex.
				}
			}	 
			
		}
		else if(type[ver]==1)  //if the vertex is external
		{
			
			sem_wait(&mutex);   //locking with the help of a semaphore 
			                    //waits until the semaphore is available
			  
			for(int j=0;j<V;j++)
		    {
				if(Mat[ver][j]==1&&colour[j]!=0)
				{
					Arr[colour[j]-1]=false;    //the colours present in its adjacent vertices are made unavailble 
				}
			}
		     
			for(int k=1;k<=V;k++)
			{
				 if(Arr[k-1]==true)
				 {
					 colour[ver]=k;
					 
					 if(k>Max)
					 {
						 Max++;   //Max contains the number of colours used till now
					 }
					 
					 break;
				 } 
			 }
			 
			 for(int j=0;j<V;j++)
			 {
				if(Mat[ver][j]==1&&colour[j]!=0)
				{
					Arr[colour[j]-1]=true;   //Now, making the colours present in its adjacent vertices available for next vertex.
				}
			 }
			
		     sem_post(&mutex);  //unlocking the semaphore
		}
	}	
}

