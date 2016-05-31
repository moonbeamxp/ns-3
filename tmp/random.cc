#include "ns3/random-variable.h"
#include <iostream>
#include <fstream>

using namespace std;
using namespace ns3;

int main(void)
{

  //streambuf* coutBuf = cout.rdbuf();
  //ofstream of("out.txt");
  //streambuf* fileBuf = of.rdbuf();
  //cout.rdbuf(fileBuf);
  //cout << "Write Personal Information over..." << endl;
 
  UniformVariable var(1, 20); 
  double val = var.GetValue();	
  int val_int = var.GetInteger(1, 8);
  int r=rand()%20+1;
  cout << val << endl << val_int << endl << r <<endl;

  //cout.rdbuf(coutBuf);
  //of.flush();
  //of.close();
  return 0;
}
