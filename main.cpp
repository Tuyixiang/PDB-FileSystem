//
//  main.cpp
//  PDB-Final-Storage
//
//  Created by Yixiang Tu on 24/11/2017.
//  Copyright ? 2017 Yixiang Tu. All rights reserved.
//

/*
 KNOWN BUGS:
 1. Pressing arrow keys will put invisible characters into input line.
 */

#include "main.h"
#include "functions.h"
#include "disk_image.h"
#include "commands.h"
#include "subtypes.h"
//#define _TESTING_

//  Create a stream where data can be read from a real file
ifstream in_file;
//  Create a stream where data can be written to a real file
ofstream out_file;
//    If true, the process will terminate
bool _terminate = false;

//  In order to manage each byte of data, we use "unsigned char", which takes up exactly 1 byte
//  Here we rename it to "byte" to avoid confusion with character units
typedef unsigned char byte;

int current_path = 0;

//  VARIABLES GO HERE
//------------------------------------------------------------------
//  The repertoire of commands
/*  CURRENT STATUS
 Implemented:
 help
 save
 exit
 pwd
 cd
 mkdir
 ls
 rmdir
 echo
 cat
 rm
 df
 Not implemented:
 May be implemented:
 encrypt $path $password
 decrypt $path $password
 cp $path $path
 mv $path $path
 imp $name $path (import: read a file from outside the disk and store it)
 exp $name $path (export: export a file)
 ...
 */
const char command_pool[][20] = {
    "help", "pwd", "cd", "mkdir", "ls",
    "rmdir", "echo", "cat", "rm", "save",
    "exit", "df", ""
};
const char command_arguments[][30] = {
    "", "", "$path", "$path", "[$path]",
    "$path", "$str $path", "$path", "$path", "",
    "", "", ""
};
const char command_descriptions[][1000] = {
    "Print a list of supported commands.", //0
    "Print current directory.", //1
    "Goto directory $path.",  //2
    "Create directory $path.",  //3
    "Print a list of files/directories under $path. If no $path specified, the current path is used.",  //4
    "Delete directory $path.",  //5
    "Write $str to file $path.",  //6
    "Print content of file $path.",  //7
    "Delete file $path.",  //8
    "Save the disk image.",  //9
    "Save and exit.",    //10
    "Show disk space usage.",   //11
    ""
};

disk_image disk_image_buffer;
#ifdef _TESTING_
char names[4096][253];
#endif

//  MAIN FUNCTION GOES HERE
//------------------------------------------------------------------
int main(int argc, const char * argv[]) {
    
//     for some testing
//     cout << sizeof(super_block_type) << endl;
//     cout << sizeof(inode_block_type) << endl;
//     cout << sizeof(data_block_type) << endl;
//     cout << sizeof(disk_image) << endl;
    
    
    //  Try to open the file named "..."
    in_file.open("disk.image", ios::in | ios::binary);
    
    if (in_file.is_open()) {
        cout << "Disk image located." << endl;
        
        //  If the file is opened, firstly, test if its size is correct (16916480 Bytes)
        long long file_begin, file_end;
        
        //  There's a pointer within in_file, marking the location to read from
        //  tellg() returns the pointer location (now at the start of the file)
        file_begin = in_file.tellg();
        
        //  Move the pointer to the end of the file
        in_file.seekg(0, ios::end);
        
        //  tellg() returns the pointer location (now at the end of the file)
        file_end = in_file.tellg();
        
        if (file_end - file_begin != DISK_SIZE) {
            cout << "Disk image failed to open (wrong size)." << endl;
            in_file.close();
            create_disk_image();
        } else {
            //  Place the pointer at the start and read its data
            in_file.seekg(0, ios::beg);
            in_file.read((char*) & disk_image_buffer, DISK_SIZE);
            in_file.close();
            
            //  Check if the disk data is consistent with its hash value
            cout << "Checking image integrity";
            if (disk_image_buffer.super_block.disk_hash == simplified_hash((char*)&disk_image_buffer + 4, DISK_SIZE - 4)) {
                cout << "OK" << endl << "Disk image loaded." << endl;
            } else {
                cout << "Failed" << endl << "Disk image is damaged." << endl;
                create_disk_image();
            }
        }
    } else {
        cout << "Disk image not found." << endl;
        in_file.close();
        create_disk_image();
    }
    
    
    char line[4096];
    char command[20];
    char argument[10][4096];
    int p;
    bool error = false;
    current_path = 0;
    cout << "Enter \"help\" for a list of supported commands." << endl;
    

    
    //  _terminate is declared as a global variable, since other functions may terminate the system
    while (!_terminate) {
        
        //  This part is for debugging
#ifdef _TESTING_
        for (int j = 0; j < 10; j ++) {
            if (disk_image_buffer.data_block[j].data[0]) {
                cout << (char*) (disk_image_buffer.data_block[j].data + 3) << endl;
            } else {
                cout << "(empty)" << endl;
            }
        }
#endif
        
        //  Initialize everything for the next command
        //  memset(&a, b, c): starting from &a, write b to a byte of data for c times
        memset(line, 0, sizeof(line));
        memset(command, 0, sizeof(command));
        memset(argument, 0, sizeof(argument));
        error = false;
        cout << endl;
        
        cout << disk_image_buffer.get_name_by_dir_id(current_path) << ">>";
        cin.getline(line, 4096);
        
        //  If the user inputs too many characters, this is used to solve the problem
        if (cin.fail()) {
            cin.clear();
            cin.ignore(INT_MAX, '\n');
            cout << "Failed to execute command!" << endl;
            cout << "Input exceeds character limit (max 4095 characters) :(" << endl;
            continue;
        }
        
        //  The characters before the first space should be the keywords for a specific command
        for (p = 0; line[p] == ' '; p ++);
        for (int i = p; line[p] != '\0' and line[p] != ' '; p ++) {
            //  no keyword should have 20 chars
            if (p >= 20) {
                error = true;
                cout << "Failed to execute command!" << endl;
                cout << "Command not found. Enter \"help\" for a list of supported commands." << endl;
                break;
            }
            command[p - i] = line[p];
        }
        
        //  Following, there should be several arguments separated by spaces
        if (!error) {
            for (; line[p] == ' '; p ++);
            if (line[p] == '\0') {
                //  No arguments are entered
            } else {
                for (int i = 0; line[p] != '\0'; i ++) {
                    if (i >= 10) {
                        error = true;
                        cout << "Failed to execute command!" << endl;
                        cout << "Too many arguments." << endl;
                        break;
                    }
                    //  Seperate the arguments and store them into argument[0], argument[1], ...
                    int j = 0;
                    for (; line[p] != '\0'; p ++) {
                        if (line[p] == '\\') {
                            if (line[p + 1] == '\0') {
                                break;
                            } else {
                                argument[i][j ++] = line[++ p];
                                continue;
                            }
                        }
                        if (line[p] == ' ') {
                            for (; line[p] == ' '; p ++);
                            break;
                        } else {
                            argument[i][j ++] = line[p];
                        }
                    }
                }
            }
        }
        
        if (!error) {
            call(command, argument);
        }
    }
    cout << "..." << endl;
    return 0;
}

