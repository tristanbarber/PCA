/* mpi_reduce.c */
#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[])
{
    int i,myid, numprocs;
    int dest,count;
    int my_value, reduced_value;
    
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    
    my_value = myid;
    
    dest=0;
    count=1;
    
    printf("(%d) my_value = %d\n",myid, my_value);
    
    MPI_Reduce(&my_value,&reduced_value,count,MPI_INT,MPI_SUM,dest,MPI_COMM_WORLD);
    
    if (myid == dest) {
        printf("(%d) reduced value = %d \n",myid,reduced_value);
    }
    MPI_Finalize();
}
