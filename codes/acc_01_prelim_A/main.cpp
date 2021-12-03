// ==
// ||
// ||    acc_01: OpenACC GPU Demonstration Code
// ||
// ||    On Summit, run on the sgpu partition, e.g., 
// ||
// ||         sinteractive --partition sgpu
// ||
// ||    And load the correct modules by sourcing the
// ||    **local** (this directory) copy of Modules_Summit.
// ||
// ==

#include<time.h>  
#include<openacc.h>  
#include <iostream>
#include <string>
#include <sstream>
#include "string.h"
using std :: cout;
using std :: endl;
using std :: string;

#define rLOOP    for ( int r = 0 ;  r < nField ; ++r )
#define cLOOP    for ( int c = 0 ;  c < nField ; ++c )
#define iterLOOP for ( int iter = 0 ;  iter < maxIter ; ++iter )

// ====================================================================================
// ||                                                                                ||
// ||                      M e m o r y   A l l o c a t i o n                         ||
// ||                                                                                ||
// ====================================================================================

double  * Array1D_double(int nRows)
{
  double *myArray;
  myArray = new double [ nRows ];
  return myArray;
}

double  * * Array2D_double(int nRows,int nCols)
{
  double *   myArray    ;  myArray     = new double [ nRows * nCols ];
  double * * myArray_ptr;  myArray_ptr = new double * [nRows];

  int count = 0;
  for ( int row = 0 ; row < nRows ; ++ row )
    {
      myArray_ptr[row] = &myArray[ count*nCols ];
      ++count;
    }

  return myArray_ptr;
}

double  * Array1D_double_acc(int nRows)
{
  double *myArray = (double*) acc_malloc( nRows * sizeof(double) );
  return myArray;
}

// ====================================================================================
// ||                                                                                ||
// ||                      T i m i n g   U t i l i i t e s                           ||
// ||                                                                                ||
// ====================================================================================

void StartTimer(struct timespec &t0  )
{
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t0);
}

void EndTimer( string KernelName , struct timespec &t0 , struct timespec &t1 )
{
  // The timespec struct provides time information in this format:
  //
  // tv_sec:tv_nsec  
  //
  // which should be thought of as an anology to the more commonly understood
  // time format
  //
  //  HH:MM:SS
  //
  // So in order to compute elapsed time, one must subtract each place (e.g., HH, MM, SS),
  // and add the differences while including units conversion

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &t1);

  int64_t sec_place     = t1.tv_sec  - t0.tv_sec ;
  int64_t ns_place      = t1.tv_nsec - t0.tv_nsec;
  int64_t sec_2_ns      = 1000000000;
  int64_t exeTime_in_ns = sec_place*sec_2_ns + ns_place;

  cout << "[ " << KernelName << " ] EXECUTION TIME  = " << exeTime_in_ns << " (ns)" << endl;
}


// ====================================================================================
// ||                                                                                ||
// ||                           M a i n   R o u t i n e                              ||
// ||                                                                                ||
// ====================================================================================

int main(  int argc, char *argv[] )
{
  struct timespec t0,t1;

  // (1) User Inputs

  int nCellx   =  0;
  int nCelly   =  0;
  int nRealx       ;     // Number of real points in the x- and y-directions
  int nRealy       ;     //
  int nField       ;     // Number of unknowns, total, in the mesh (no ghost nodes)
  int maxIter  = 0 ;

  for (int count =  0 ; count < argc; ++count)
    {
      if ( !strcmp(argv[count],"-nCellx"  ) ) nCellx  = atoi(argv[count+1]);
      if ( !strcmp(argv[count],"-nCelly"  ) ) nCelly  = atoi(argv[count+1]);
      if ( !strcmp(argv[count],"-maxIter" ) ) maxIter = atoi(argv[count+1]);
    }

  nRealx = nCellx + 1;
  nRealy = nCelly + 1;
  nField = nRealx*nRealy; 

  if ( nField * maxIter == 0 )
    {
      printf("Fatal Error: You must specify the number of cells and iterations.\n");
      exit(0);
    }
  
  // (1) Allocate memory, 1 based

  double **A   = Array2D_double(nField,nField);
  double *x    = Array1D_double(nField);
  double *prod = Array1D_double(nField);
  
  // (2) Initialize values
  
  rLOOP { x[r] = r; prod[r] = r; }
  rLOOP cLOOP A[r][c] = r*10. + c;
  
  double dot = 0.;

  printf("\n");
  printf("-------------------------------------------------\n");
  printf("   OpenACC Demonstration Code                    \n");
  printf("-------------------------------------------------\n");
  printf("\n");
  printf("Parameters: \n");
  printf("\n");
  printf("nRealx  = %d\n",nRealx );
  printf("nRealy  = %d\n",nRealy );
  printf("nField  = %d\n",nField );
  printf("maxIter = %d\n",maxIter);
  printf("\n");

  // ---------------------------------------------------
  // Vector Add, Dot, Ax - CPU
  // ---------------------------------------------------

  StartTimer(t0);

  rLOOP x[r] = x[r] + prod[r] ;

  rLOOP  dot += x[r] * prod[r] ;
  
  iterLOOP
  rLOOP
    {
      prod[r] = 0.;
      cLOOP
	prod[r] += A[r][c]*x[c];
    }
  EndTimer(" Add,Dot,Ax on CPU                 ",t0,t1);
  
  // ---------------------------------------------------
  // Vector Add, Dot, Ax - GPU
  // ---------------------------------------------------


#pragma acc enter data create(x[0:nField],prod[0:nField],A[0:nField][0:nField],nField,maxIter)

#pragma acc parallel loop present(x[0:nField],prod[0:nField],A[0:nField][0:nField],nField,maxIter)
      rLOOP { x[r] = 1; prod[r] = 1; }

#pragma acc parallel loop present(A[0:nField][0:nField],nField,maxIter)
      rLOOP 
        cLOOP 
          A[r][c] = r*10. + c;

  StartTimer(t0);

#pragma acc parallel loop present(x[0:nField],prod[0:nField],nField,maxIter)
      rLOOP  x[r] = x[r] + prod[r] ; 

#pragma acc parallel loop reduction(+:dot) present(x[0:nField],prod[0:nField],nField,maxIter)
      rLOOP dot += x[r] * prod[r] ; 

      iterLOOP
#pragma acc parallel loop present(x[0:nField],prod[0:nField],A[0:nField][0:nField],nField,maxIter)
	rLOOP
	  {
	    prod[r] = 0.;
	    cLOOP
	      prod[r] += A[r][c]*x[c];
	  }

#pragma acc exit data delete(x[0:nField],prod[0:nField],A[0:nField][0:nField],nField,maxIter)

  EndTimer(" Add,Dot,Ax on GPU                 ",t0,t1);

  
  printf("\n");
  return 0;
}




