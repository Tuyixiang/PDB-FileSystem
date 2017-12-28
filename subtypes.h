//
//  subtypes.h
//  PDB-Final-Storage
//
//  Created by Yixiang Tu on 22/12/2017.
//  Copyright © 2017 涂轶翔. All rights reserved.
//

#ifndef subtypes_h
#define subtypes_h

//  DATA STRUCTURE DECLARATIONS GO HERE
//------------------------------------------------------------------
/*struct hash_result_type {
 byte r[16];
 bool equals(hash_result_type a) {
 for (int i = 0; i < 16; i ++) {
 if (r[i] != a.r[i]) {
 return false;
 }
 }
 return true;
 }
 }hash_result;*/

struct super_block_type {
    unsigned int disk_hash;
    byte free_space[7164];
    
    //  To use every bit, we store the bitmaps in this way
    //  The boolean value (1 or 0) of the n-th node is stored at the (n % 8)-th digit(from right to left) of bitmap[n / 8]
    byte inode_bitmap[512];
    byte block_bitmap[512];
    
    void initialize();
};



struct inode_block_type {
    
    //  THESE ARE UNUSED VARIABLES
    //  These are created as a pool of ideas we can work further on, but not yet completed
    int file_size, inode_id;
    unsigned int hash_value;
    bool continues, continued;
    
    byte free_space;
    
    //  TRUE if the data block is a directory, FALSE if it's a file
    bool is_dir;
    
    //  Its parent directory id
    int father;
    
    //  more free space to add exciting features (encryption, integrity check, etc.)
    byte more_free_space[12];
    
    void initialize();
};

struct data_block_type {
    //  Both file blocks and directory blocks share this type
    //  In directories, data[256*k + 1] ~ data[256*k + 2] stores its k-th child's address, data[256*k] indicates whether it's occupied
    //  For directories, 1st child is always itself, with name of itself, and 2nd child is always parent, with name ".."
    byte data[4096];
    
    bool set_parent(int);
    bool rename(const char*);
    bool new_child(unsigned int, const char*);
    
    void initialize();
    void ls();
};

#endif /* subtypes_h */
