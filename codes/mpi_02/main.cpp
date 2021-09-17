#include<omp.h>     // Required for OpenMP
#include<mpi.h>     // Required for MPI
#include<stdio.h>
#include<chrono>
#include<ctime>
#include "main.h"

int main(  int argc, char *argv[] )
{
  MPI_Init     (&argc         , &argv       );

  int numPE;      
  int myPE;
  MPI_Comm_size(MPI_COMM_WORLD,  &numPE);  
  MPI_Comm_rank(MPI_COMM_WORLD, &myPE  );  

  // =====================================================================================
  // Cartesian Communicator
  //
  // Adapted from examples at University of Texas, TACC, website
  //
  // =====================================================================================
  
  // Preparing to use University of Texas, TACC code
  
  int ndim = 2; int periodic[2]; int dimensions[2]; int coord_2d[2];
  int idim = 3;
  int jdim = 3;
  int procno = myPE;
  int rank_2d;
  MPI_Comm comm = MPI_COMM_WORLD;
  
  MPI_Comm comm2d;
  periodic  [0] =       periodic[1]   = 0   ;
  dimensions[0] = idim; dimensions[1] = jdim;
  
  MPI_Cart_create(comm   , ndim    , dimensions , periodic , 1 , &comm2d);
  MPI_Cart_coords(comm2d , procno  , ndim       , coord_2d              );
  MPI_Cart_rank  (comm2d , coord_2d, &rank_2d                           );
  
  cout << "myPE: " << myPE << endl;
  cout << "myPE: " << myPE << "Cartesian Communicator" << endl;
  cout << "myPE: " << myPE << endl;
  cout << "myPE: " << myPE << " iPE = " << coord_2d[0] << " jPE = " << coord_2d[1] << "  |  New Rank = " << rank_2d << endl;
  cout << "myPE: " << myPE << endl;
  
  // Using comm2d for an allgather operation
  
  int n = 5 ;  int a[n];    for ( int i = 0 ; i < n; ++i ) a[i] = 0;
  int m = 20;  int b[m];    for ( int i = 0 ; i < m; ++i ) b[i] = 0;
  
  for ( int i = 0 ; i < n; ++i ) a[i] = myPE;
  
  MPI_Neighbor_allgather( a , n , MPI_INT,
                          b , n , MPI_INT, comm2d);
  
  printArray("b on myPE = 0, gathered from neighbors ",m,b,myPE);  // Results good on all PE

  
  // =====================================================================================
  //
  // Custom Communicator
  //
  // Adapted from an example at riptutorials.com
  //
  // =====================================================================================
  
#define nnode 9

  MPI_Comm_rank(MPI_COMM_WORLD, &myPE);

  int source = myPE;
  int numNei;
  int nei   [nnode];
  int weight[nnode];
  int recv  [nnode];
  int send = myPE*100;
  
  for ( int i = 0 ; i < numPE ; ++i ) weight[i] = 1;
  for ( int i = 0 ; i < numPE ; ++i ) recv  [i] = -1;
  
  cout << "myPE: " << myPE << endl;
  cout << "myPE: " << myPE << "Custom Communicator" << endl;
  cout << "myPE: " << myPE << endl;
  
  // set nei and numNei.

  //                   |            Row Below               |         Same Row        |              Row Above                  |
  //                   |  ---          ---          ---     |      ---         ---    |    ---          ---          ---        |
  
       if (myPE == 0) {                                                     nei[0] = 1;               nei[1] = 3;  nei[2] = 4;    numNei = 3;   }
  else if (myPE == 2) {                                        nei[0] = 1;  nei[1] = 4;  nei[2] = 5;                              numNei = 3;   }
  else if (myPE == 6) {              nei[0] = 3;  nei[1] = 4;               nei[2] = 7;                                           numNei = 3;   }
  else if (myPE == 8) { nei[0] = 4;  nei[1] = 5;               nei[2] = 7;                                                        numNei = 3;   }
  else if (myPE == 1) {                                        nei[0] = 0;  nei[1] = 2;  nei[2] = 3;  nei[3] = 4;  nei[4] = 5;    numNei = 5;   }
  else if (myPE == 3) {              nei[0] = 0;  nei[1] = 1;               nei[2] = 4;  nei[3] = 6;  nei[4] = 7;                 numNei = 5;   }
  else if (myPE == 5) { nei[0] = 1;  nei[1] = 2;               nei[2] = 4;               nei[3] = 7;  nei[4] = 8;                 numNei = 5;   }
  else if (myPE == 7) { nei[0] = 3;  nei[1] = 4;  nei[2] = 5;  nei[3] = 6;  nei[4] = 8;                                           numNei = 5;   }
  else if (myPE == 4) { nei[0] = 0;  nei[1] = 1;  nei[2] = 2;  nei[3] = 3;  nei[4] = 5;  nei[5] = 6;  nei[6] = 7;  nei[7] = 8;    numNei = 8;   }
  
  // create graph.
  
  MPI_Comm graph;
  MPI_Dist_graph_create(MPI_COMM_WORLD, 1, &source, &numNei, nei, weight, MPI_INFO_NULL, 1, &graph);
  
  // send and gather myPE to/from neighbors.
  
  MPI_Neighbor_allgather(&send, 1, MPI_INT, recv, 1, MPI_INT, graph);
  
  printArray("recv: ",numNei,recv,myPE);  
  
  MPI_Finalize();
  return 0;

  
}

