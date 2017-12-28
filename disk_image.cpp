//
//  disk_image.cpp
//  PDB-Final-Storage
//
//  Created by Yixiang Tu on 22/12/2017.
//  Copyright © 2017 涂轶翔. All rights reserved.
//

#include "main.h"
#include "functions.h"
#include "disk_image.h"
#include "subtypes.h"

void disk_image::initialize() {
    cout << "Initializing disk image...";
    super_block.initialize();
    for (int i = 0; i < 4096; i ++) {
        inode_block[i].initialize();
        data_block[i].initialize();
    }
    initialize_data_block_as_dir(0, 0, "");
}

void disk_image::save() {
    if (!out_file.is_open()) {
        out_file.open("disk.image", ios::out | ios::binary | ios::trunc);
    }
    out_file.seekp(0, ios::beg);
    cout << "Hashing";
    super_block.disk_hash = simplified_hash((char*)this + 4, DISK_SIZE - 4);
    cout << endl << "Writing image..." << endl;
    out_file.write((char*)this, DISK_SIZE);
}
    
//  True if inode[id] is used
bool disk_image::inode_is_used(int id) {
    return super_block.inode_bitmap[id >> 3] & (1 << (id & 7));
}
//  True if block[id] is used
bool disk_image::block_is_used(int id) {
    return super_block.block_bitmap[id >> 3] & (1 << (id & 7));
}
//  Set inode[id] as used
void disk_image::use_inode(int id) {
    super_block.inode_bitmap[id >> 3] |= (1 << (id & 7));
}
//  Set block[id] as used
void disk_image::use_block(int id) {
    super_block.block_bitmap[id >> 3] |= (1 << (id & 7));
}
//  Set inode[id] as unused
void disk_image::clear_inode(int id) {
    super_block.inode_bitmap[id >> 3] &= 0xFF ^ (1 << (id & 7));
}
//  Set block[id] as unused
void disk_image::clear_block(int id) {
    super_block.block_bitmap[id >> 3] &= 0xFF ^ (1 << (id & 7));
}
bool disk_image::initialize_data_block_as_file(int id_block) {
    use_inode(id_block);
    use_block(id_block);
    inode_block[id_block].initialize();
    data_block[id_block].initialize();
    return true;
}
bool disk_image::initialize_data_block_as_dir(int id_block, int id_parent, const char* name) {
    for (int i = 0; name[i] != '\0'; i ++) {
        if (i == 252) {
            cout << "Failed to create directory!\nName exceeds character limit (max 252)." << endl;
            return false;
        }
    }
    use_inode(id_block);
    use_block(id_block);
    inode_block[id_block].initialize();
    data_block[id_block].initialize();
    inode_block[id_block].father = id_parent;
    inode_block[id_block].is_dir = true;
    inode_block[id_block].inode_id = id_block;
    memset(data_block[id_block].data, 0, 4096);
    data_block[id_block].new_child(id_block, name);
    data_block[id_block].new_child(id_parent, "..");
    return true;
}
/*
 FIND THE ID OF THE DIRECTORY WITH GIVEN NAME UNDER DIR *id_parent*
 Returns -1 if not found, -2 if found but it's not a directory
 */
int disk_image::get_dir_id_by_name_under(const char* name, int id_parent) {
    //  Each directory contains a link to itself with name "."
    if (name[0] == '.' and name[1] == '\0') {
        return id_parent;
    }
    //  For directories, their 1st child is itself (with its own name), so we'll skip the 1st one
    for (int i = 259; i < 4096; i += 256) {
        if (strcmp((char*)(data_block[id_parent].data + i), name) == 0) {
            if (disk_image_buffer.inode_block[data_block[id_parent].data[i - 1] + (data_block[id_parent].data[i - 2] << 8)].is_dir) {
                return data_block[id_parent].data[i - 1] + (data_block[id_parent].data[i - 2] << 8);
            } else {
                return -2;
            }
        }
    }
    return -1;
}
/*
 FIND THE ID OF THE FILE WITH GIVEN NAME UNDER DIR *id_parent*
 Returns -1 if not found, -2 if found but it's not a file
 */
int disk_image::get_file_id_by_name_under(const char* name, int id_parent) {
    //  Each directory contains a link to itself with name "."
    if (name[0] == '.' and name[1] == '\0') {
        return -2;
    }
    //  For directories, their 1st child is itself (with its own name), so we'll skip the 1st one
    for (int i = 259; i < 4096; i += 256) {
        if (strcmp((char*)(data_block[id_parent].data + i), name) == 0) {
            if (disk_image_buffer.inode_block[data_block[id_parent].data[i - 1] + (data_block[id_parent].data[i - 2] << 8)].is_dir) {
                return -2;
            } else {
                return data_block[id_parent].data[i - 1] + (data_block[id_parent].data[i - 2] << 8);
            }
        }
    }
    return -1;
}

/*
 FIND THE ID OF THE FILE/DIRECTORY WITH GIVEN NAME UNDER DIR *id_parent*
 Returns -1 if not found,
 */
int disk_image::get_item_id_by_name_under(const char* name, int id_parent) {
    //  Each directory contains a link to itself with name "."
    if (name[0] == '.' and name[1] == '\0') {
        return id_parent;
    }
    //  For directories, their 1st child is itself (with its own name), so we'll skip the 1st one
    for (int i = 3843; i > 3; i -= 256) {
        if (strcmp((char*)(data_block[id_parent].data + i), name) == 0) {
            return data_block[id_parent].data[i - 1] + (data_block[id_parent].data[i - 2] << 8);
        }
    }
    return -1;
}

const char* disk_image::get_name_by_dir_id(int id) {
    return (const char*)(data_block[id].data + 3);
}

const char* disk_image::get_name_by_file_id(int id) {
    const static char def[] = "";
    for (int i = 0; i < 4096; i ++) {
        if (inode_block[i].is_dir) {
            for (int j = 0; j < 4096; j += 256) {
                if (data_block[i].data[j] and id == (data_block[i].data[j + 1] << 8) + data_block[i].data[j + 2]) {
                    return (const char*) (data_block[i].data + j + 3);
                }
            }
        }
    }
    cout << "No Wayyyyyy!" << endl;
    return def;
}

int disk_image::get_parent_id_by_item_id(int id) {
    for (int i = 0; i < 4096; i ++) {
        if (inode_block[i].is_dir) {
            for (int j = 0; j < 4096; j += 256) {
                if (data_block[i].data[j] and id == (data_block[i].data[j + 1] << 8) + data_block[i].data[j + 2]) {
                    return i;
                }
            }
        }
    }
    return -1;
}

/*
 FIND AN EMPTY BLOCK FOR NEW DATA
 RETURN:
    id of that block, -1 if disk full
 */
int disk_image::find_empty_block() {
    for (int i = 1; i < 4096; i ++) {
        if (!inode_is_used(i) and !block_is_used(i)) {
            return i;
        }
    }
    return -1;
}

/*
 Get directory id by paths such as /dir1/dir2
 */
int disk_image::get_dir_id_by_path(const char* path) {
    char temp_name[253];
    int id_path, p;
    if (path[0] == '/') {
        //  Absolute path, starting from root (id = 0)
        p = 1;
        id_path = 0;
    } else {
        //  Relative path, starting from *current_path*
        p = 0;
        id_path = current_path;
    }
    for (; ; p ++) {
        //  Copy the next directory's name to *temp_name*
        //  The next directory's name starts from *p* and ends with either '\0' or '/'
        memset(temp_name, 0, sizeof(temp_name));
        for (int i = p; path[p] != '\0' and path[p] != '/'; p ++) {
            if (p - i == 252) {
                cout << "Failed to locate directory!" << endl;
                cout << "Input name exceeds character limit (max 252)." << endl;
                return -1;
            }
            temp_name[p - i] = path[p];
        }
        if (temp_name[0] == '\0') {
            //  Reaches the end of string *path*
            break;
        }
        id_path = get_dir_id_by_name_under(temp_name, id_path);
        if (id_path == -1) {
            cout << "Failed to locate directory!" << endl;
            for (int j = 0; j < p; j ++) {
                cout << path[j];
            }
            cout << ": No such directory." << endl;
            return -1;
        } else if (id_path == -2) {
            cout << "Failed to locate directory!" << endl;
            for (int j = 0; j < p; j ++) {
                cout << path[j];
            }
            cout << " is not a directory." << endl;
            return -2;
        }
        if (path[p] == '\0' or path[p + 1] == '\0') {
            //  Reaches the end of string *path*
            break;
        }
    }
    return id_path;
}

/*
 CALL THE PREVIOUS FUNCTION, USING THE FIRST *length* CHARACTERS OF STRING *start* AS *path*
 */
int disk_image::get_dir_id_by_path(const char* start, int length) {
    static char path[4096];
    memset(path, 0, sizeof(path));
    for (int i = 0; i < length; i ++) {
        path[i] = start[i];
    }
    return get_dir_id_by_path(path);
}


/*
 Get file id by paths such as /dir1/file
 This is really similar to get_dir_id_by_path, the only difference is when to return -2 (wrong type)
 */
int disk_image::get_file_id_by_path(const char* path) {
    char temp_name[253];
    int id_path, p;
    for (p = 0; path[p] != '\0'; p ++);
    if (path[p - 1] == '/') {
        cout << "Failed to locate file!" << endl;
        cout << "No file name entered." << endl;
        return -1;
    }
    if (path[0] == '/') {
        p = 1;
        id_path = 0;
    } else {
        p = 0;
        id_path = current_path;
    }
    for (; ; p ++) {
        memset(temp_name, 0, sizeof(temp_name));
        for (int i = p; path[p] != '\0' and path[p] != '/'; p ++) {
            if (p - i == 252) {
                cout << "Failed to locate file!" << endl;
                cout << "Input name exceeds character limit (max 252)." << endl;
                return -1;
            }
            temp_name[p - i] = path[p];
        }
        if (temp_name[0] == '\0') {
            break;
        }
        id_path = get_item_id_by_name_under(temp_name, id_path);
        if (id_path == -1) {
            cout << "Failed to locate file!" << endl;
            for (int j = 0; j < p; j ++) {
                cout << path[j];
            }
            if (path[p] == '\0') {
                cout << ": No such file." << endl;
            } else {
                cout << ": No such directory." << endl;
            }
            return -1;
        }
        if (path[p] == '\0' or path[p + 1] == '\0') {
            break;
        }
    }
    if (inode_block[id_path].is_dir) {
        cout << "Failed to locate file!" << endl;
        for (int j = 0; j < p; j ++) {
            cout << path[j];
        }
        cout << " is not a file." << endl;
        return -2;
    }
    return id_path;
}

/*
 RECURSIVELY, REMOVE BLOCK *id*
 This doesn't clear the file link stored in its parent directory
 */
void disk_image::recursive_remove(int id) {
    if (inode_block[id].is_dir) {
        //  If target is a directory, also call remove on the files/directories in it
        //  The first two children of a directory is itself and its parent, so we start from 512 (3rd)
        for (int i = 512; i < 4096; i += 256) {
            if (data_block[id].data[i] == 1) {
                recursive_remove((data_block[id].data[i + 1] << 8) + data_block[id].data[i + 2]);
            }
        }
    }
    if (inode_block[id].continues) {
        recursive_remove(inode_block[id].next_node);
    }
    inode_block[id].initialize();
    data_block[id].initialize();
    clear_inode(id);
    clear_block(id);
}

/*
 SIMPLY REMOVE BLOCK *id*
 */
void disk_image::remove(int id) {
    if (id >= 0) {
        //  Firstly, clear the file link stored in its parent directory
        int id_parent = get_parent_id_by_item_id(id);
        for (int i = 0; i < 4096; i += 256) {
            if (data_block[id_parent].data[i] and id == (data_block[id_parent].data[i + 1] << 8) + data_block[id_parent].data[i + 2]) {
                memset(data_block[id_parent].data + i, 0, 256);
            }
        }
        
        recursive_remove(id);
    }
}

/*
 RETURN THE NUMBER OF USED INODES/DATA BLOCKS
 We never seperate an inode with its corresponding data block, so the number is same for inodes/data blocks
 */
int disk_image::used_space() {
    int cnt = 0;
    for (int i = 0; i < 4096; i ++) {
        if (block_is_used(i)) {
            cnt ++;
        }
    }
    return cnt;
}
