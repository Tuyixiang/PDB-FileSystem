//
//  main.h
//  PDB-Final-Storage
//
//  Created by Yixiang Tu on 22/12/2017.
//  Copyright © 2017 涂轶翔. All rights reserved.
//

#ifndef main_h
#define main_h

#include <iostream>
#include <cstring>
#include <fstream>  //  Used to read/write data from/to files
#define DISK_SIZE 16916480
using namespace std;

//  In order to manage each byte of data, we use "unsigned char", which takes up exactly 1 byte
//  Here we rename it to "byte" to avoid confusion with character units
typedef unsigned char byte;

//  Create a stream where data can be read from a real file
extern ifstream in_file;
//  Create a stream where data can be written to a real file
extern ofstream out_file;
//    If true, the process will terminate
extern bool _terminate;

extern int current_path;

extern const char command_pool[][20], command_arguments[][30], command_descriptions[][1000];


#endif /* main_h */
