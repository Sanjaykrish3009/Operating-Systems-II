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

void threadFunc2(vector<int>array);


//global variables

bool **Mat,*type;  

int*colour;        

int V,Max=0;


//semaphores

sem_t *Mutex;   //for fine grain locking (one lock for each vertex)



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
	
    //FINE GRAIN LOCKING
    
    Mutex=(sem_t*)malloc(V*sizeof(sem_t));  //Dynamically allocating memory for Mutex locks
    
    for(int i=0;i<V;i++)
	{
		colour[i]=0;    //uncolouring all the vertices
		
		int ret=sem_init(&Mutex[i],0,1);   //Intialising Mutex[i] with '1'
    
		if(ret!=0)
		{
			Outfile << "Unsuccessful Mutex lock " << i+1 << " initialisation" << endl;
		}
	}
	
	Max=0;
    
    struct timeval start2,end2;
	
	gettimeofday(&start2,NULL);   //calling the function 'gettimeofday' to know time before thread creation
	
    std::thread threads2[P];
    
    for(int i=0;i<P;i++)
    {
		threads2[i]=std::thread(threadFunc2,part[i]);
	}
	
	for(int i=0;i<P;i++)
    {
		threads2[i].join();
	}
	
	gettimeofday(&end2,NULL);    //calling the function 'gettimeofday' to know time after threads execution
	
	//Output for coarsegrain locking
	
	Outfile << "Fine-grained Lock:" << endl;
	
	Outfile << "Number of colours used: " << Max << endl;
    
    double time2= end2.tv_sec-start2.tv_sec +(end2.tv_usec-start2.tv_usec)*0.001;
    
    Outfile << "Time taken by the algorithm using fine-grained lock: " << time2 << " Milliseconds" << endl;
    
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


void threadFunc2(vector<int>array)
{
	bool Arr[V];    //Assuming a maximum of V colours are present
	
	for(int i=0;i<V;i++)
	{
		Arr[i]=true;    //initially making all colours available (i.e, true)
	}
	
	int count=array.size();
	
	//coloring each vertex present in the given partition sequentially
    for(int i=0;i<count;i++)
    { 
        int ver=array[i];
         
		if(type[ver]==0)    //if the vertex is internal
		{
			for(int j=0;j<V;j++)
			{
				if(Mat[ver][j]==1&&colour[j]!=0)
				{
					Arr[colour[j]-1]=false;    //the colours present in its adjacent vertices are made unavailble 
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
					   Max++;    //Max contains the number of colours used till now
					}
					 
					break;
				}
				 
				
		    }
		    
			for(int j=0;j<V;j++)
			{
				if(Mat[ver][j]==1&&colour[j]!=0)
				{
					Arr[colour[j]-1]=true;    //Now, making the colours present in its adjacent vertices available for next vertex.
				}
			}	 
			
		}
		else if(type[ver]==1)
		{
			
			for(int j=0;j<V;j++)
		    {
				 if(Mat[ver][j]==1||ver==j)
				 {
					 sem_wait(&Mutex[j]);    //Locking all the adjacent vertices locks along with itself in increasing order of vertex IDs
					 
					 if(ver!=j&&colour[j]!=0)
					 {
						Arr[colour[j]-1]=false;    //the colours present in its adjacent vertices are made unavailble 
					 }
					 
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
						 Max++;    //Max contains the number of colours used till now
					 }
					 
					 break;
				 } 
			 }
			 
		     for(int j=0;j<V;j++)
		     {
				 if(Mat[ver][j]==1||ver==j)
				 {
					 if(ver!=j&&colour[j]!=0)
					 {
						Arr[colour[j]-1]=true;     //Now, making the colours present in its adjacent vertices available for next vertex.
					 }
					 
					 sem_post(&Mutex[j]);   //Unlocking all the adjacent vertices locks along with itself in increasing order of vertex IDs
					 
				 }
			 }
		}
	}	
}



