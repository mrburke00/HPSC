#include<mpi.h>     // Required for MPI
#include<stdio.h>
#include "main.h"

int main(  int argc, char *argv[] )
{

  // Begin parallel segment.
  
  MPI_Init (  &argc , &argv );

  int numPE = 4;
  int myPE;
  
  MPI_Comm_size( MPI_COMM_WORLD ,  &numPE );
  MPI_Comm_rank( MPI_COMM_WORLD ,  &myPE );
  
  cout << "Hello World!  I am PE " << myPE << endl;

  // Broadcast

  int n = 10;
  int a[n];

  for ( int i = 0 ; i < 10 ; ++i ) a[i] = 0.;
  if ( myPE == 0 ) for ( int i = 0 ; i < n ; ++i ) a[i] = 333;

  MPI_Bcast ( a , n , MPI_INT , 3 , MPI_COMM_WORLD );

  //  for ( int i = 0 ; i < 10 ; ++i ) cout << "myPE: " << myPE << " a["<< i <<"] = " << a[i] << endl;

  // Scatter

  int m = 40;
  int b[m];
  
  for ( int i = 0 ; i < m ; ++i ) b[i] = 0.;
  if ( myPE == 0 ) for ( int i = 0 ; i < m ; ++i ) b[i] = i;

  MPI_Scatter ( b , n , MPI_INT,
		a , n , MPI_INT, 0 , MPI_COMM_WORLD );

  //  for ( int i = 0 ; i < n ; ++i ) cout << "myPE: " << myPE << " a["<< i <<"] = " << a[i] << endl;

  // Gather

  for ( int i = 0 ; i < m ; ++i ) b[i] = -1111;
  for ( int i = 0 ; i < n ; ++i ) a[i] = myPE;

  MPI_Gather ( a , n , MPI_INT,
	       b , n , MPI_INT , 0 , MPI_COMM_WORLD) ;   // Here, the results will only be valid on PE0

  
  //  for ( int i = 0 ; i < m ; ++i ) cout << "myPE: " << myPE << " b["<< i <<"] = " << b[i] << endl;

  // Reduce

  int myVal = myPE;
  int myReducedVal ;

  //                                 length of data         PE who gets the answer
  //                                   |                      |
  //                                   |                      |
  MPI_Reduce( &myVal , &myReducedVal , 1 , MPI_INT, MPI_SUM , 0 , MPI_COMM_WORLD );

  //  cout << "myPE: " << myPE << " Summed value = " << myReducedVal << endl;
  
  // All Reduce

  MPI_Allreduce( &myVal , &myReducedVal , 1 , MPI_INT , MPI_SUM , MPI_COMM_WORLD);
  cout << "myPE: " << myPE << " Summed value = " << myReducedVal << endl;
  
  // End parallel segment

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Finalize();
  

}





