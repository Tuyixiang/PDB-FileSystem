//
//  functions.h
//  PDB-Final-Storage
//
//  Created by Yixiang Tu on 22/12/2017.
//  Copyright © 2017 涂轶翔. All rights reserved.
//

#ifndef functions_h
#define functions_h



//  FUNCTION DECLARATIONS GO HERE
//------------------------------------------------------------------
struct hash_result_type;
void create_disk_image();
void call(char* command, char arguments[10][4096]);
int find_command(char* command);
unsigned int simplified_hash(char* start, int length);
unsigned int hash_multiply(unsigned int a, unsigned int b);
void correct_path(char* str);





#endif /* functions_h */
