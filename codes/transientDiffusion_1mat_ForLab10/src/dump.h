//  ================================================================================
//  ||                                                                            ||
//  ||              transientDiffusion                                            ||
//  ||              ------------------------------------------------------        ||
//  ||              T R A N S I E N T D I F F U S I O N                           ||
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


//  ================================================================================
//  ||                                                                            ||
//  ||  M P I I O   D u m p   W r i t e r                                         ||
//  ||                                                                            ||
//  ================================================================================

void write_mpiio_dump(mpiInfo &myMPI)
{
	MPI_Datatype myRealNodes;
	
	int idxStartThisPE  [1] = { 0 };  // Index coordinates of the sub-array inside this PE's array, A
	int AsizeThisPE     [1] = { nField };  // Size of the A array on this PE    
	int sub_AsizeThisPE [1] = { nField-1 };  // Size of the A-sub-array on this PE 

	MPI_Type_create_subarray(1, AsizeThisPE, sub_AsizeThisPE, idxStartThisPE, MPI_ORDER_C, MPI_FLOAT, &myRealNodes);
	
	MPI_Type_commit(&myRealNodes);

	MPI_Datatype myPartOfGlobal;
  	
	int idxStartInGlobal [1] = { myPE * (nField-1) };  // Index cordinates of the sub-arrayinside the global array
  	int AsizeGlobal      [1] = { myMPI.numPE * (nField)};  // Size of the global array

	MPI_Type_create_subarray(1, AsizeGlobal, sub_AsizeThisPE, idxStartInGlobal, MPI_ORDER_C, MPI_FLOAT, &myPartOfGlobal);

	MPI_Type_commit(&myPartOfGlobal);

  	MPI_File fh;
  
  	MPI_File_open(MPI_COMM_WORLD, "output.bin", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);

  	MPI_File_set_view (fh, 0, MPI_FLOAT, myPartOfGlobal, "native", MPI_INFO_NULL);  

	MPI_File_write_all(fh, &phi[0], 1, myRealNodes, MPI_STATUS_IGNORE);

  	MPI_File_close(&fh);
  
  
  	//free(phi[0]);
  	//free(phi);
  
  	MPI_Type_free(&myPartOfGlobal);
  	MPI_Type_free(&myRealNodes);
  	MPI_Finalize();
	
}





