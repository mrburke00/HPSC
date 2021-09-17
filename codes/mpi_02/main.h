#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include "math.h"
using std :: string;
using std :: vector;
using std :: stringstream;
using std :: cout;
using std :: endl;


void printArray(string arrayName, int n , int *array , int myPE)
{
  for ( int i = 0 ; i < n ; ++i )
    cout << "myPE: " << myPE << " " << arrayName << "[" << i << "] = " << array[i] << endl;
}

void printVal(string valName, int val , int myPE)
{
  cout << "myPE: " << myPE << " " << valName << " = " << val << endl;
}


