/* mpi_bcast.c */
#include <stdio.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
    int i,myid, numprocs;
    int source,count;
    int buffer[4];
    
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    
    source=0;
    count=4;
    if(myid == source){
        for(i=0;i<count;i++)
            buffer[i]=i;
    }
    if ( myid == source){
     printf("Before Broadcast \n");
     for(i=0 ; i<count ; i++) 
        printf("(%d) \t buf[%d]=%d \n",myid,i,buffer[i]);
    
     printf("After Broadcast \n");
    }
    MPI_Bcast(buffer,count,MPI_INT,source,MPI_COMM_WORLD);
    
    for(i=0;i<count;i++)
        printf("(%d) \t buf[%d]=%d \n",myid,i,buffer[i]);
    MPI_Finalize();
}
