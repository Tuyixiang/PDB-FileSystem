//
//  subtypes.cpp
//  PDB-Final-Storage
//
//  Created by Yixiang Tu on 22/12/2017.
//  Copyright © 2017 涂轶翔. All rights reserved.
//

#include "main.h"
#include "disk_image.h"
#include "subtypes.h"

//  STRUCTURE FUNCTION IMPLEMENTATIONS GO HERE
//------------------------------------------------------------------

void super_block_type::initialize() {
    memset(this, 0, sizeof(super_block_type));
}

void inode_block_type::initialize() {
    memset(this, 0, sizeof(inode_block_type));
}

void data_block_type::initialize() {
    memset(this, 0, sizeof(data_block_type));
}

/*
 SET NODE *id_parent* AS PARENT OF THIS DIRECTORY
 WARNING: if this data block contains a file, this function will change its content in undesired ways
 */
bool data_block_type::set_parent(int id_parent) {
    for (int i = 3; i < 256; i ++) {
        data[256 + i] = disk_image_buffer.data_block[id_parent].data[i];
    }
    return false;
}
/*
 RENAME THIS DIRECTORY
 WARNING: if this data block contains a file, this function will change its content in undesired ways
 */
bool data_block_type::rename(const char* name) {
    for (int i = 0; name[i] != '\0'; i ++) {
        if (i == 252) {
            cout << "Failed to rename directory!\nName exceeds character limit (max 252)." << endl;
            return false;
        }
    }
    memset(data + 3, 0, 253);
    for (int i = 0; name[i] != '\0'; i ++) {
        data[3 + i] = name[i];
    }
    return true;
}
/*
 ADD A NEW CHILD INTO THIS DIRECTORY
 WARNING: if this data block contains a file, this function will change its content in undesired ways
 */
bool data_block_type::new_child(unsigned int inode, const char* name) {
    for (int i = 0; name[i] != '\0'; i ++) {
        if (i == 252) {
            cout << "Failed to create file/directory!\nName exceeds character limit (max 252)." << endl;
            return false;
        }
    }
    for (int i = 0; i < 4096; i += 256) {
        if (data[i] == 0) {
            data[i] = 1;
            data[i + 1] = inode >> 8;
            data[i + 2] = inode & 0xFF;
            memset(data + i + 3, 0, 253);
            for (int j = 0; name[j] != '\0'; j ++) {
                data[i + 3 + j] = name[j];
            }
            return true;
        }
    }
    cout << "Failed to create file/directory!\nParent directory full." << endl;
    return false;
}

/*
 PRINT A LIST OF CHILD FILES/DIRECTORIES
 */
void data_block_type::ls() {
    for (int i = 512; i < 4096; i += 256) {
        if (data[i] == 1) {
            cout << data + i + 3 << endl;
        }
    }
}
