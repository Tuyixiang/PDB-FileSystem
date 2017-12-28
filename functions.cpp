//
//  functions.cpp
//  PDB-Final-Storage
//
//  Created by Yixiang Tu on 22/12/2017.
//  Copyright © 2017 涂轶翔. All rights reserved.
//

#include "main.h"
#include "functions.h"
#include "disk_image.h"
#include "commands.h"
#include "subtypes.h"

//  FUNCTION IMPLEMENTATIONS GO HERE
//------------------------------------------------------------------

/*
 INITIALIZE *disk_image_buffer* (STORED IN THIS EXE), AND WRITE IT TO *disk.image* (A REAL FILE)
 */
void create_disk_image() {
    disk_image_buffer.initialize();
    out_file.open("disk.image", ios::out | ios::binary | ios::trunc);
    out_file.seekp(0, ios::beg);
    out_file.write((char*) &disk_image_buffer.super_block.disk_hash, sizeof(disk_image));
    cout << "New disk image created." << endl;
}

/*
 RUN THE SPECIFIED COMMAND
 ARGUMENTS:
 char* command_keyword:
 a string containing the first word of a line of command the user inputs
 char argument[10][500]:
 several strings containing the arguments the user inputs
 // the arguments are seperately stored in argument[0], argument[1], ...
 */
void call(char* command_keyword, char arguments[10][4096]) {
    //  Call the specific command
    switch (find_command(command_keyword)) {
        case -1: {
            cout << "Failed to execute command \"" << command_keyword << "\"!\nCommand not recognized. Enter \"help\" for a list of supported commands." << endl;
            break;
        }
        case 0: {
            if (arguments[0][0] != '\0') {
                cout << "Failed to execute command \"help\"!\nArgument not recognized: this function does not require arguments." << endl;
            } else {
                COMMAND_help();
            }
            break;
        }
        case 1: {
            if (arguments[0][0] != '\0') {
                cout << "Failed to execute command \"pwd\"!\nArgument not recognized: this function does not require arguments." << endl;
            } else {
                COMMAND_pwd(current_path);
                cout << endl;
            }
            break;
        }
        case 2: {
            if (arguments[1][0] != '\0') {
                cout << "Failed to execute command \"cd\"!\nToo many arguments (needed 1)." << endl;
                cout << "Usage:\tcd $path" << endl;
                cout << "Example:\tcd /var/log" << endl;
                cout << "Example:\tcd dir" << endl;
            } else if (arguments[0][0] == '\0') {
                cout << "Failed to execute command \"cd\"!\nNo argument entered (needed 1)." << endl;
                cout << "Usage:\tcd $path" << endl;
                cout << "Example:\tcd /var/log" << endl;
                cout << "Example:\tcd dir" << endl;
            } else {
                correct_path(arguments[0]);
                COMMAND_cd(arguments[0]);
            }
            break;
        }
        case 3: {
            if (arguments[1][0] != '\0') {
                cout << "Failed to execute command \"mkdir\"!\nToo many arguments (needed 1)." << endl;
                cout << "Usage:\tmkdir $path" << endl;
                cout << "Example:\tmkdir /var/log" << endl;
                cout << "Example:\tmkdir dir" << endl;
            } else if (arguments[0][0] == '\0') {
                cout << "Failed to execute command \"mkdir\"!\nNo argument entered (needed 1)." << endl;
                cout << "Usage:\tmkdir $path" << endl;
                cout << "Example:\tmkdir /var/log" << endl;
                cout << "Example:\tmkdir dir" << endl;
            } else {
                correct_path(arguments[0]);
                COMMAND_mkdir(arguments[0]);
            }
            break;
        }
        case 4: {
            if (arguments[1][0] != '\0') {
                cout << "Failed to execute command \"ls\"!\nToo many arguments (needed 1 or 0)." << endl;
                cout << "Usage:\tls [$path]" << endl;
                cout << "Example:\tls /var/log" << endl;
                cout << "Example:\tls dir" << endl;
                cout << "Example:\tls" << endl;
            } else if (arguments[0][0] == '\0') {
                COMMAND_ls(current_path);
            } else {
                correct_path(arguments[0]);
                COMMAND_ls(arguments[0]);
            }
            break;
        }
        case 5: {
            if (arguments[1][0] != '\0') {
                cout << "Failed to execute command \"rmdir\"!\nToo many arguments (needed 1)." << endl;
                cout << "Usage:\trmdir $path" << endl;
                cout << "Example:\trmdir /var/log" << endl;
                cout << "Example:\trmdir dir" << endl;
            } else if (arguments[0][0] == '\0') {
                cout << "Failed to execute command \"rmdir\"!\nNo argument entered (needed 1)." << endl;
                cout << "Usage:\trmdir $path" << endl;
                cout << "Example:\trmdir /var/log" << endl;
                cout << "Example:\trmdir dir" << endl;
            } else {
                correct_path(arguments[0]);
                COMMAND_rmdir(arguments[0]);
            }
            break;
        }
        case 6: {
            if (arguments[2][0] != '\0') {
                cout << "Failed to execute command \"echo\"!\nToo many arguments (needed 2)." << endl;
                cout << "For spaces in the string, please use \"\\ \" instead." << endl;
                cout << "Usage:\techo $str $path" << endl;
                cout << "Example:\techo Hello\\ World! log.txt" << endl;
            } else if (arguments[1][0] == '\0') {
                cout << "Failed to execute command \"echo\"!\nNot enough arguments entered (needed 2)." << endl;
                cout << "Usage:\techo $str $path" << endl;
                cout << "Example:\techo Hello\\ World! log.txt" << endl;
            } else {
                correct_path(arguments[1]);
                COMMAND_echo(arguments[0], arguments[1]);
            }
            break;
        }
        case 7: {
            if (arguments[1][0] != '\0') {
                cout << "Failed to execute command \"cat\"!\nToo many arguments (needed 1)." << endl;
                cout << "Usage:\tcat $path" << endl;
                cout << "Example:\tcat log.txt" << endl;
            } else if (arguments[0][0] == '\0') {
                cout << "Failed to execute command \"cat\"!\nNo argument entered (needed 1)." << endl;
                cout << "Usage:\tcat $path" << endl;
                cout << "Example:\tcat log.txt" << endl;
            } else {
                correct_path(arguments[0]);
                COMMAND_cat(arguments[0]);
            }
            break;
        }
        case 8: {
            if (arguments[1][0] != '\0') {
                cout << "Failed to execute command \"rm\"!\nToo many arguments (needed 1)." << endl;
                cout << "Usage:\trm $path" << endl;
                cout << "Example:\trm log.txt" << endl;
            } else if (arguments[0][0] == '\0') {
                cout << "Failed to execute command \"rm\"!\nNo argument entered (needed 1)." << endl;
                cout << "Usage:\trm $path" << endl;
                cout << "Example:\trm log.txt" << endl;
            } else {
                correct_path(arguments[0]);
                COMMAND_rm(arguments[0]);
            }
            break;
        }
        case 9: {
            COMMAND_save();
            break;
        }
        case 10: {
            COMMAND_exit();
            break;
        }
        case 11: {
            COMMAND_df();
        }
        default: {
            break;
        }
    }
}

/*
 MATCH THE GIVEN WORD WITH SUPPORTED KEYWORDS
 ARGUMENTS:
 char* command_keyword:
 a string containing the inputted keyword
 RETURN:
 -1 if no match is found
 [index of the keyword in command_pool] if the keyword matchs one in the pool
 */
int find_command(char* command_keyword) {
    for (int i = 0; command_pool[i][0] != '\0'; i ++) {
        if (strcmp(command_keyword, command_pool[i]) == 0) {
            return i;
        }
    }
    return -1;
}

/*  you don't have to read this function
 CALCULATE A VALUE ACCORDING TO TWO NUMBERS
 */
unsigned int hash_multiply(unsigned int a, unsigned int b) {
    static const unsigned int constants[10] = {
        0xEB8F03FE, 0x5656DF6A, 0x127F74B8, 0x5204405D, 0x550442F9,
        0xE8736151, 0x8064F93E, 0xA516EF9B, 0xEEBF856C, 0x24DCDE2C
    };
    unsigned int result = 0x450915DC;
    for (int i = 0; i < 10; i ++) {
        result *= a ^ constants[i];
        result = (result >> 3) + (result << 29);
        result *= b ^ constants[i];
        result = (result >> 27) + (result << 5);
    }
    return result;
}

/*  you don't have to read this function
 CALCULATE A NUMERICAL RESULT FROM DATA
 ARGUMENTS:
 char* pStart:
 the beginning address of data
 int length:
 the length of data
 RETURN:
 a value after complex calculation over the data
 this value is same for same inputs and, theoretically, the output shouldn't collide for two different inputs
 */
unsigned int simplified_hash(char* pStart, int length) {
    static const unsigned int constants[10] = {
        0x788d79E7, 0xFCB0DAC9, 0x5EE86BBA, 0x5BE0F1A0, 0xE741F9CF,
        0x7FF8269B, 0xA40B0686, 0xEB8BEF94, 0xA086B7D5, 0xE4462E12
    };
    unsigned int result = 0x5D9B801F;
    if (length > 4096) {
        for (int i = 0; i < length; i ++) {
            result ^= hash_multiply(pStart[i % length], constants[(i * i) % 10]);
            result = (result >> (i & 31)) + (result << ((i & 31) ^ 31));
            if (!(i % (length / 3)) and i) {
                cout << ".";
            }
        }
    } else {
        for (int i = 0; i < 4096; i ++) {
            result ^= hash_multiply(pStart[i % length], constants[(i * i) % 10]);
            result = (result >> (i & 31)) + (result << ((i & 31) ^ 31));
        }
    }
    return result;
}

/*
 Autocorrection of input path
 "///path//path/dir" -> "/path/path/dir"
 */
void correct_path(char* input) {
    for (int i = 0; input[i]; i ++) {
        if (input[i] == '/') {
            while (input[i + 1] == '/') {
                for (int j = i; input[j] != '\0'; j ++) {
                    input[j] = input[j + 1];
                }
            }
        }
    }
}


