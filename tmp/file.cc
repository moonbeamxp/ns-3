#include "ns3/stdafx.h"
#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
     using namespace std;
 
    cout << "Hello, Let's begin a test of cout to file." << endl;
     // 保存cout流缓冲区指针
      streambuf* coutBuf = cout.rdbuf();
 
     ofstream of("out.txt");
     // 获取文件out.txt流缓冲区指针
      streambuf* fileBuf = of.rdbuf();
     
     // 设置cout流缓冲区指针为out.txt的流缓冲区指针
      cout.rdbuf(fileBuf);
     cout << "Name " << "Chen"        << endl;
     cout << "Sex  " << "Female"      << endl;
     cout << "E-mail"<< "Chen@qq.com" << endl;
 
     of.flush();
     of.close();
 
     // 恢复cout原来的流缓冲区指针
      cout.rdbuf(coutBuf);
     cout << "Write Personal Information over..." << endl;
 
     system("PAUSE");
     return 0;
 } 
