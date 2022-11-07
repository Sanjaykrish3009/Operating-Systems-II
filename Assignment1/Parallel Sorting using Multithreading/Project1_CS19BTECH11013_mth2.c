#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<pthread.h>   //header file for using threads
#include<sys/time.h>   //header file for using 'gettimeofday' function


int seg=0,seg2=0,p=1;   //initialising global variables

struct thread //This struct is for passing parameters while thread creation
{
	long*arr;       //array which is to be sorted
	
	int seg_size;  //size of the segment
};

//Declaration of functions
	
int part(long* Arr, int low, int high);

void quick_sort(long* Array,int low,int high);

int power(int base,int exp);

void Merge(long*Array,int low,int mid,int high);

void* Merge_sort(void*arg);

void* Quick_sort(void*arg);


int main()
{
	
  int size,n;
  
  FILE *fp1;
	
  fp1 = fopen("inp.txt","r");
	
  if(fp1==NULL)
  {
	 printf("\nCan't open file for reading");	
  }
  else
  {
	fscanf(fp1,"%d",&size);
	
	fscanf(fp1,"%d",&n);	
	
	if(n<0||size<0) //ERROR condition
	{
		fprintf(fp1,"\nERROR");
	}
	else
	{
	    int SIZE=power(2,size);  //SIZE is size of array
	
	    int N=power(2,n);  //N is number of threads to be created
	
	    int seg_size=SIZE/N;   //seg_size is size of each segment

	    srand(time(NULL));     
	
	    struct timeval start,end;   //for start time and end time using gettimeofday
	
	    struct thread*k;   
	
	    k=(struct thread*)malloc(sizeof(struct thread)); //
	
	    k->arr=(long*)malloc(SIZE*sizeof(long));  //dynamic allocation of memory for array
	
	    k->seg_size=seg_size;
	
		for(int i=0;i<SIZE;i++)
		{
			k->arr[i]= rand();
		}	
	    
	    FILE *fp2;
	    
	    fp2=fopen("output.txt","w");
	    
		fprintf(fp2,"\nUnsorted Array: ");
	
		for(int i=0;i<SIZE;i++)           //printing the array before sorting
		{
			fprintf(fp2,"%ld ",k->arr[i]);
		}	
	
		gettimeofday(&start,NULL);    //calling 'gettimeofday' function to know current elapsation time
	
		if(n>size) //ERROR condition if number of threads is greater than size of array
		{
			fprintf(fp2,"\nThread creation was unsuccessful");
		}
		else
		{
			pthread_t threads[N];
			
			
	        //for segment sorting
			for(int i=0;i<N;i++)
			{    
				pthread_create(&threads[i],NULL,Quick_sort,k);   
				//calling 'pthread_create' function to create threads using 'Quick_sort' function and passing pointer 'k' as arguement
			}
	
			for(int i=0;i<N;i++)
			{    
				pthread_join(threads[i],NULL);   //calling 'pthread_join' to join all threads after execution
			}	
	
			int y=N;
	
			while(y>1)
			{
				int num=y/2;
		
				pthread_t Threads[num];
		
		        //for merging segments
				for(int i=0;i<num;i++)
				{    
					pthread_create(&Threads[i],NULL,Merge_sort,k);
					//calling 'pthread_create' function to create threads using 'Merge_sort' function and passing pointer 'k' as arguement
				}
	    
				for(int i=0;i<num;i++)
				{    
					pthread_join(Threads[i],NULL);    //calling 'pthread_join' to join all threads after execution
				}	
	    
				seg2=0;
	    
				p++;
	    
				y=y/2;   //decrementing the number of threads to be created to half the value
	    
			}
			
			gettimeofday(&end,NULL);   //calling 'gettimeofday' function to know current elapsation time
	
			fprintf(fp2,"\nSorted Array: ");
	
			for(int i=0;i<SIZE;i++)
			{
				fprintf(fp2,"%ld ",k->arr[i]);   //printing the array after sorting
			}
	
			long Time=(end.tv_sec-start.tv_sec)*1000000+ end.tv_usec - start.tv_usec;
	
			fprintf(fp2,"\nTime elapsed: %ld micro seconds",Time);   //printing time elapsed to sort the array
	
		}
	}
  }
  
  return 0;		
}


//defining the function 'part' 
//This function helps to find pivot of the array
int part(long* Arr, int low, int high)
{
	
	int x = low - 1;
	
	for (int i=low;i<high; ++i)
	{
		if (Arr[i] < Arr[high])
		{
			long swap = Arr[x+1];
			
			Arr[x+1] = Arr[i];
			
			Arr[i] = swap;
			
			x++;
		}
	}
	
	long temp = Arr[x+1];
	
	Arr[x+1] = Arr[high];
	
	Arr[high] = temp;
	
	return x+1;
}

//defining the function 'quick_sort'
//This function helps to sort the array using recursion
void quick_sort(long* Array,int low,int high)
{
	
	if (low<high)
	{
		int pivot = part(Array,low,high);   //calling the function 'part'
		
		quick_sort(Array, low, pivot-1);  //recursive call
		
		quick_sort(Array, pivot+1, high);  //recursive call
	}
}

//defining the function 'power'
//This function helps to power of base to given exponent
int power(int base,int exp)
{
	int x=1;
	
	for(int i=0;i<exp;i++)
	{
		x=x*base;
	}
	
	return x;
}

//defining the function 'Quick_sort' with void* as return type
//The threads created with this function will execute this function basing on the arguements passed
//This function helps for segment sorting using quick sort
void* Quick_sort(void*arg)
{
	struct thread*P=arg;
	
	int low= seg*P->seg_size;
	
	int high= (seg+1)*P->seg_size - 1;
	
	seg++;
	
	quick_sort(P->arr,low,high);  //calling the function 'quick_sort'
	
	return NULL;
}

//defining the function 'Merge'
//This function helps to merge any two sorted segments in sorted manner
void Merge(long*Array,int low,int mid,int high)
{
	int size1=mid-low+1;
	
	int size2=high-mid;
	
	long Left[size1],Right[size2];
	
	for(int i=0;i<size1;i++)
	{
		Left[i]=Array[low+i];
	}
	
	for(int i=0;i<size2;i++)
	{
		Right[i]=Array[mid+1+i];
	}
	
	int x=low;
	
	int i=0,j=0;
	
	while(i<size1 && j<size2)
	{
		if(Left[i]<=Right[j])
		{
			Array[x++]=Left[i++];
		}
		else
		{
			Array[x++]=Right[j++];
		}
	}
	
	while(i<size1)
	{
	    Array[x++]=Left[i++];	
	}
	
	while(j<size2)
	{
	    Array[x++]=Right[j++];	
	}
	
}

//defining the function 'Quick_sort' with void* as return type
//The threads created with this function will execute this function basing on the arguements passed
//This function helps for merging any two segments
void* Merge_sort(void*arg)
{
	struct thread*P=arg;
	
	int left=seg2*P->seg_size;
			
	int right=(seg2+power(2,p))*P->seg_size -1;
			
	int mid=left + (right-left)/2;
			
	Merge(P->arr,left,mid,right);    //calling the function 'Merge'
	
	seg2=seg2+power(2,p);
	
	return NULL;
}





