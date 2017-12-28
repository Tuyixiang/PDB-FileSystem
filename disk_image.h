//
//  disk_image.h
//  PDB-Final-Storage
//
//  Created by Yixiang Tu on 22/12/2017.
//  Copyright © 2017 涂轶翔. All rights reserved.
//

#ifndef disk_image_h
#define disk_image_h

#include "functions.h"
#include "subtypes.h"

//  This disk should take up exactly 16520KB
struct disk_image {
    super_block_type super_block;
    inode_block_type inode_block[4096];
    data_block_type data_block[4096];
    
    void initialize();
    void save();
    
    bool initialize_data_block_as_file(int id_block);
    bool initialize_data_block_as_dir(int id_block, int id_parent, const char* name);
    
    
    //  Use Bitwise Operations to access and manipulate the bitmap data
    //  These are bitwise operation samples
    bool inode_is_used(int id);
    bool block_is_used(int id);
    void use_inode(int id);
    void use_block(int id);
    void clear_inode(int id);
    void clear_block(int id);
    int get_dir_id_by_name_under(const char* name, int id_parent);
    int get_file_id_by_name_under(const char* name, int id_parent);
    int get_item_id_by_name_under(const char* name, int id_parent);
    const char* get_name_by_dir_id(int id);
    const char* get_name_by_file_id(int id);
    int get_parent_id_by_item_id(int id);
    int find_empty_block();
    int get_dir_id_by_path(const char* path);
    int get_file_id_by_path(const char* path);
    int get_dir_id_by_path(const char* start, int length);
    void recursive_remove(int id);
    void remove(int id);
    int used_space();
};

extern disk_image disk_image_buffer;

#endif /* disk_image_h */
