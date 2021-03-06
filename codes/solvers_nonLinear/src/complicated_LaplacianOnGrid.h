//  ================================================================================
//  ||                                                                            ||
//  ||              solvers                                                       ||
//  ||              ------------------------------------------------------        ||
//  ||              S O L V E R S                                                 ||
//  ||                                                                            ||
//  ||              D E M O N S T R A T I O N   C O D E                           ||
//  ||              ------------------------------------------------------        ||
//  ||                                                                            ||
//  ||       Developed by: Scott R. Runnels, Ph.D.                                ||
//  ||                     University of Colorado Boulder                         ||
//  ||                                                                            ||
//  ||                For: CU Boulder CSCI 4576/5576 and associated labs          ||
//  ||                                                                            ||
//  ||           Copyright 2020 Scott Runnels                                     ||
//  ||                                                                            ||
//  ||                     Not for distribution or use outside of the             ||
//  ||                     this course.                                           ||
//  ||                                                                            ||
//  ================================================================================


//  ==
//  ||
//  ||      C L A S S:   L A P L A C I A N O N G R I D 
//  ||
//  == 

class LaplacianOnGrid
{

public:

  double x0, x1, y0, y1;
  VD x,y;
  int nRealx    , nRealy   , nField;
  int nCellx    , nCelly   ;
  double dx, dy;
  VDD Acoef ;
  VII Jcoef ;
  VDD Jacobian ;  
  VD  phi ;  VD  b ;  VD f ; VD dPhi; VD phiNew; VD phiMMS;
  int bandwidth;
  int myPE;
  VD Qval;
  double k0 , k1, k2;
  double pi;

  //  ==
  //  ||
  //  ||  Constructor:  Initialize values
  //  ||
  //  ==

  LaplacianOnGrid(double _x0   , double _x1,  double _y0, double _y1 , int ncell_x , int ncell_y, mpiInfo &myMPI )
  {

    k0 =  2.0 ;
    k1 =  0.0 ;
    k2 =  0.0 ;

    x0 = _x0;    x1 = _x1;
    y0 = _y0;    y1 = _y1;

    nCellx     = ncell_x;
    nCelly     = ncell_y;
    
    nRealx     = ncell_x + 1; 
    nRealy     = ncell_y + 1; 
    nField     = nRealx*nRealy;  
    dx         = (x1-x0)/ncell_x;
    dy         = (y1-y0)/ncell_y;

    // Form the mesh

    x.resize(nField+1); y.resize(nField+1); Qval.resize(nField+1);
		       
    for ( int i = 1 ; i <= nRealx ; ++i )
      for ( int j = 1 ; j <= nRealy ; ++j )
	{
	  int p = pid(i,j);
	  x[p] = x0 + (i-1)*dx;
	  y[p] = y0 + (j-1)*dy;
	}

    // Allocate memory for matrices and field variables
    
    bandwidth = 5;
    
    Acoef   .resize( nField+1 ); rLOOP Acoef[r]   .resize( bandwidth+1 );
    Jacobian.resize( nField+1 ); rLOOP Jacobian[r].resize( bandwidth+1 );
    Jcoef   .resize( nField+1 ); rLOOP Jcoef[r]   .resize( bandwidth+1 );
    b       .resize( nField+1 );
    phi     .resize( nField+1 );
    dPhi    .resize( nField+1 );
    phiNew  .resize( nField+1 );
    phiMMS  .resize( nField+1 );
    f       .resize( nField+1 );

    for ( int r = 0 ; r <= nField ; ++r )
      {
	phi   [r] = 0.;
	dPhi  [r] = 0.;
	phiNew[r] = 0.;
	Qval  [r] = 0.1;
      }

    pi = 3.141596;

    rLOOP phiMMS[r] = sin(pi*x[r]);
      
    
    myPE = myMPI.myPE;

  }

  //  ==
  //  ||
  //  ||  Form Linear System Ax = b
  //  ||
  //  ==

  void FormLS(mpiInfo &myMPI)
  {
    double half    = 0.5;
    double quarter = 0.25;
    
    rLOOP cLOOP Acoef   [r][c] = 0.;  // Initialize linear system
    rLOOP cLOOP Jacobian[r][c] = 0.;  // Initialize Jacobian
    rLOOP cLOOP Jcoef   [r][c] = 0 ;  // The default is to point to entry zero; note that phi[0] is set to 0, above.
    rLOOP       Jcoef   [r][1] = r;   //
    rLOOP        b      [r] = 0.;
    
    double dx2 = dx*dx;           // Form matrix entries for the interior grid points
    double dy2 = dy*dy;           // Form matrix entries for the interior grid points

    // Set up the E/W/N/S stencil:

    for ( int i = 1 ; i <= nRealx ; ++i )
      for ( int j = 1 ; j <= nRealy ; ++j )
	{
	  int p = pid(i,j);
	                    Jcoef[ p ][ 1 ] =  p;
          if ( i < nRealx ) Jcoef[ p ][ 2 ] =  p+1;           // East
	  if ( i > 1      ) Jcoef[ p ][ 3 ] =  p-1;           // West
	  if ( j < nRealy ) Jcoef[ p ][ 4 ] =  p+nRealx;      // North
	  if ( j > 1      ) Jcoef[ p ][ 5 ] =  p-nRealx;      // South
	}

    // Loop over each cell, and let each cell contribute to its points' finite difference equation
    
    for ( int i = 1 ; i <= nCellx ; ++i )
      for ( int j = 1 ; j <= nCelly ; ++j )
	{
	  int p;
	  double k;

	  p = pid(i,j);
	  k = Eval_k( phi[p] );
	  Acoef[p][1] += k * half * ( -1./dx2 - 1./dy2 );
	  Acoef[p][2] += k * half *    1./dx2;
	  Acoef[p][4] += k * half *    1./dy2;
	  b[p]        +=     quarter * (k0 + k1*sin(pi*x[p]))*(-pi*pi*sin(pi*x[p]));

	  p = pid(i+1,j);
	  k = Eval_k( phi[p] );
	  Acoef[p][1] += k * half * ( -1./dx2 - 1./dy2 );
	  Acoef[p][3] += k * half *    1./dx2;
	  Acoef[p][4] += k * half *    1./dy2;
	  b[p]        +=     quarter * (k0 + k1*sin(pi*x[p]))*(-pi*pi*sin(pi*x[p]));

	  p = pid(i+1,j+1);
	  k = Eval_k( phi[p] );
	  Acoef[p][1] += k * half * ( -1./dx2 - 1./dy2 );
	  Acoef[p][3] += k * half *    1./dx2;
	  Acoef[p][5] += k * half *    1./dy2;
	  b[p]        +=     quarter * (k0 + k1*sin(pi*x[p]))*(-pi*pi*sin(pi*x[p]));

	  p = pid(i,j+1);
	  k = Eval_k( phi[p] );
	  Acoef[p][1] += k * half * ( -1./dx2 - 1./dy2 );
	  Acoef[p][2] += k * half *    1./dx2;
	  Acoef[p][5] += k * half *    1./dy2;
	  b[p]        +=     quarter * (k0 + k1*sin(pi*x[p]))*(-pi*pi*sin(pi*x[p]));
	  
      }

    // Loop over each cell, and let each cell contribute to its points' Jacobian
    
    for ( int i = 1 ; i <= nCellx ; ++i )
      for ( int j = 1 ; j <= nCelly ; ++j )
	{
	  int p;
	  double k;
	  double dk;

	  p  = pid(i,j);
	  k  = Eval_k ( phi[p] );
	  dk = Eval_dk( phi[p] );
	  Jacobian[p][1] += dk * half * ( -1./dx2 - 1./dy2 );
	  Jacobian[p][2] +=  k * half *    1./dx2;
	  Jacobian[p][4] +=  k * half *    1./dy2;
	  
	  p  = pid(i+1,j);
	  k  = Eval_k ( phi[p] );
	  dk = Eval_dk( phi[p] );
	  Jacobian[p][1] += dk * half * ( -1./dx2 - 1./dy2 );
	  Jacobian[p][3] +=  k * half *    1./dx2;
	  Jacobian[p][4] +=  k * half *    1./dy2;
	  
	  p  = pid(i+1,j+1);
	  k  = Eval_k ( phi[p] );
	  dk = Eval_dk( phi[p] );
	  Jacobian[p][1] += dk * half * ( -1./dx2 - 1./dy2 );
	  Jacobian[p][3] +=  k * half *    1./dx2;
	  Jacobian[p][5] +=  k * half *    1./dy2;
	  
	  p  = pid(i,j+1);
	  k  = Eval_k( phi[p] );
	  dk = Eval_dk( phi[p] );
	  Jacobian[p][1] += dk * half * ( -1./dx2 - 1./dy2 );
	  Jacobian[p][2] +=  k * half *    1./dx2;
	  Jacobian[p][5] +=  k * half *    1./dy2;
	  
      }

    // Apply BCs

    for ( int i = 1 ; i <= nRealx ; ++i )
      for ( int j = 1 ; j <= nRealy ; ++j )
	{
	  int p      = pid(i,j);
	  
	  double mms = sin(pi*x[p]);
	  
	  if ( myMPI.iPE == 0            ) if ( i == 1      ) { ApplyBC( Acoef , b, p, mms );  ApplyBC( Jacobian , f, p,  phi[p] - mms ); }
	  if ( myMPI.iPE == myMPI.nPEx-1 ) if ( i == nRealx ) { ApplyBC( Acoef , b, p, mms );  ApplyBC( Jacobian , f, p,  phi[p] - mms ); }
	  if ( myMPI.jPE == 0            ) if ( j == 1      ) { ApplyBC( Acoef , b, p, mms );  ApplyBC( Jacobian , f, p,  phi[p] - mms ); }
          if ( myMPI.jPE == myMPI.nPEy-1 ) if ( j == nRealy ) { ApplyBC( Acoef , b, p, mms );  ApplyBC( Jacobian , f, p,  phi[p] - mms ); }
	  
	}

    // Compute right-hand side for Jacobian * delta = -f where f is defined as f = A*phi - b.

    // (a) Compute A*phi and store it in f.

    rowLOOP
      {
	f[row] = 0.;
	colLOOP f[row] += Acoef[row][col] * phi[ Jcoef[row][col] ];
      }
    
    // (a) Finish computing f by subtracting b, such that f = A*phi - b

    rLOOP f[r] = f[r] - b[r];

    // (b) The right-hand-side is actually minus f, so:
    
    rLOOP f[r] = -f[r];

  }

  
  void ApplyBC(VDD &Matrix , VD &RHS , int BCrow,double BCvalue)
  {
    cLOOP Matrix[BCrow][c] = 0.; 
    cLOOP Jcoef [BCrow][c] = 0 ;

    Matrix[ BCrow ] [ 1 ] = 1.      ; 
    Jcoef [ BCrow ] [ 1 ] = BCrow   ;
    RHS   [ BCrow ]       = BCvalue ;

    rLOOP
      if (  r != BCrow )
	{
	  cLOOP
	    {
	      if ( Jcoef[r][c] == BCrow )
		{
		  RHS[r] -= Matrix[r][c]*BCvalue;
		  Jcoef[r][c] = 0; Matrix[r][c] = 0.;
		}
	    }
	}
  }

  //  ==
  //  ||
  //  ||  Update nonlinear solution 
  //  ||
  //  ==

  int NR_Phi_Update( double tolerance , double relax)
  {
    int converged = 1;
    int numTH = 4;
    int numTHconverged = numTH;
    int *THconverged = new int [ numTH ];
    for(int i = 0; i < numTH; i++) THconverged[i] = 1;
    omp_set_num_threads(numTH);
    #pragma omp parallel shared(numTH)
    {
      int myTH = omp_get_thread_num();
      int numPerTH =    nField/numTH  ;
      int Lower    =    myTH*numPerTH  ;
      int Upper    =    Lower + numPerTH -1  ;
      if ( myTH == numTH-1) Upper = nField -1;
      int thisThreadConverged = 1;

      #pragma omp barrier
      #pragma omp single
      {
        numTHconverged = numTH;
      }

      for (int row = Lower; row <= Upper; row++)
        {
     	    phi[row] = phi[row]*relax + (1.-relax)*( phi[row] + dPhi[row] );

          if ( fabs( dPhi[row] ) > tolerance ) thisThreadConverged = 0;
        }
      printf("thisThreadConverged: %d  --- myTH: %d \n", thisThreadConverged, myTH);
      THconverged[myTH] = thisThreadConverged;
      printf("THconverged[myTH]: %d \n", THconverged[myTH]);
      #pragma omp barrier
      #pragma omp single
      {
        numTHconverged = numTH;
        for ( int i = 0 ; i < numTH ; ++i ) numTHconverged -= THconverged[i];
          printf("numTHconverged: %d \n", numTHconverged);
          if(numTHconverged != 0) converged = 0;
      }
    }
    return converged;
  }
  int SA_Phi_Update( double tolerance , double relax)
  {
    int converged = 1;
    rLOOP
      {
	if ( fabs( phiNew[r] - phi[r] ) > tolerance ) converged = 0;
	phi[r] = relax*phi[r] + (1.-relax)*phiNew[r] ;
      }
    return converged;
  }

  //  ==
  //  ||
  //  ||  Material Model
  //  ||
  //  ==

  double Eval_k   (double T)  { return k0 + k1*T + k2*T*T           ;  }
  double Eval_dk  (double T)  { return Eval_k(T) + T*(k1 + 2*k2*T)  ;  }

  //  ==
  //  ||
  //  ||  Utility routines
  //  ||
  //  ==

  void printLinearSystem(int myPE)
  {

    rLOOP
      {
	printf("myPE: %d row = %3d: ",myPE,r);
	cLOOP printf(" | %2d--> %9.5e ",Jcoef[r][c],Acoef[r][c] );

      }

  }

    int pid(int i,int j) { return i + (j-1)*nRealx; }  
                                                       
  #include "plotter.h"
  #include "linearSolver.h"
};


