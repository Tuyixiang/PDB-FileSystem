//
//  commands.cpp
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

/*
 PRINT A LIST OF AVAILABLE COMMANDS
 */
void COMMAND_help() {
    cout << "Supported commands:" << endl;
    for (int i = 0; command_pool[i][0] != '\0'; i ++) {
        cout << "-" << command_pool[i] << " " << command_arguments[i] << endl;
        cout << "    " << command_descriptions[i] << endl;
    }
}

/*
 WRITE disk_image_buffer TO REAL FILE disk.image
 */
void COMMAND_save() {
    disk_image_buffer.save();
    cout << "Disk image saved." << endl;
}

/*
 TERMINATE THIS PROGRAM
 */
void COMMAND_exit() {
    char in[3];
    cout << "Save before exitting? (Y/N)" << endl;
    cin.getline(in, 3);
    if (cin.fail()) {
        cin.clear();
        cin.ignore(INT_MAX, '\n');
    }
    if (in[1] != '\0' or (in[0] != 'y' and in[0] != 'n' and in[0] != 'Y' and in[0] != 'N')) {
        cout << "Command canceled." << endl;
    } else if (in[0] == 'Y' or in[0] == 'y') {
        disk_image_buffer.save();
    }
    cout << "Quitting";
    _terminate = true;
}

/*
 PRINT CURRENT PATH
 This goes back to the root directory with a recursive method
 */
void COMMAND_pwd(int id) {
    if (id == 0) {
        cout << "/";
    } else {
        COMMAND_pwd(disk_image_buffer.inode_block[id].father);
        cout << disk_image_buffer.data_block[id].data + 3 << "/";
    }
}

/*
 GOTO SPECIFIED DIRECTORY
 */
void COMMAND_cd(const char* path) {
    int temp_path = disk_image_buffer.get_dir_id_by_path(path);
    if (temp_path >= 0) {
        current_path = temp_path;
    }
}

/*
 CREATE DIRECTORY IN SPECIFIED PATH
 *path* should be "/dir1/dir2/dir3/new_dir" (absolute) or "dir1/new_dir" (relative)
 */
void COMMAND_mkdir(const char* path) {
    //  new_name will store the new directory's name
    static char new_name[253];
    memset(new_name, 0, sizeof(new_name));
    int p, id_path;
    
    //  Move p to the end of string *path*
    for (p = 0; path[p] != '\0'; p ++);
    
    //  If the user enters "mkdir dir/", we don't want to write down the "/"
    if (path[-- p] == '/') {
        p --;
    }
    
    if (p == -1) {
        //  This should only happen when "mkdir /"
        cout << "Failed to create directory!\nNo directory name entered." << endl;
        return;
    }
    
    //  Move p to the end of the parent path of the directory to be created
    for (; p > 0 and path[p] != '/'; p --);
    
    if (p == 0) {
        //  No parent path entered ("mkdir /new_dir" or "mkdir new_dir")
        if (path[0] == '/') {
            id_path = 0;
            for (int i = 0; path[i + 1] != '\0' and path[i + 1] != '/'; i ++) {
                new_name[i] = path[i + 1];
            }
        } else {
            id_path = current_path;
            for (int i = 0; path[i] != '\0' and path[i] != '/'; i ++) {
                new_name[i] = path[i];
            }
        }
        if (new_name[0] == '\0') {
            //  This should only happen when "mkdir /", which shouldn't end up here
            cout << "Failed to create directory!\nNo directory name entered." << endl;
            cout << "Something crazy just happened!" << endl;
            cin >> new_name;
            return;
        }
        if (disk_image_buffer.get_dir_id_by_name_under(new_name, id_path) >= 0) {
            cout << "Failed to create directory!\nDirectory already exists." << endl;
            return;
        } else if (disk_image_buffer.get_dir_id_by_name_under(new_name, id_path) == -2) {
            cout << "Failed to create directory!\nA file with the same name already exists." << endl;
            return;
        }
        int new_node = disk_image_buffer.find_empty_block();
        if (new_node == -1) {
            cout << "Failed to create directory!\nDisk full." << endl;
            return;
        }
        if (disk_image_buffer.data_block[id_path].new_child(new_node, new_name)) {
            disk_image_buffer.initialize_data_block_as_dir(new_node, id_path, new_name);
        }
        return;
    } else {
        //  "mkdir parent_path/new_dir" or "mkdir /parent_path/new_dir"
        //  the / is at path[p]
        for (int i = 0; path[p + i + 1] != '\0' and path[p + i + 1] != '/'; i ++) {
            new_name[i] = path[p + i + 1];
        }
        id_path = disk_image_buffer.get_dir_id_by_path(path, p + 1);
        if (disk_image_buffer.get_dir_id_by_name_under(new_name, id_path) >= 0) {
            cout << "Failed to create directory!\nDirectory already exists." << endl;
            return;
        } else if (disk_image_buffer.get_dir_id_by_name_under(new_name, id_path) == -2) {
            cout << "Failed to create directory!\nA file with the same name already exists." << endl;
            return;
        }
        int new_node = disk_image_buffer.find_empty_block();
        if (new_node == -1) {
            cout << "Failed to create directory!\nDisk full." << endl;
            return;
        }
        if (disk_image_buffer.data_block[id_path].new_child(new_node, new_name)) {
            disk_image_buffer.initialize_data_block_as_dir(new_node, id_path, new_name);
        }
    }
    
}

/*
 PRINT A LIST OF CHILD FILES/DIRECTORIES
 */
void COMMAND_ls(int id) {
    disk_image_buffer.data_block[id].ls();
}
void COMMAND_ls(const char* path) {
    disk_image_buffer.data_block[disk_image_buffer.get_dir_id_by_path(path)].ls();
}

/*
 DELETE A DIRECTORY RECURSIVELY
 */
void COMMAND_rmdir(const char* path) {
    int id = disk_image_buffer.get_dir_id_by_path(path);
    if (id == 0) {
        cout << "Failed to remove directory!\nCannot remove root directory." << endl;
        return;
    } else if (id > 0) {
        for (int i = 0; i < 4096; i += 256) {
            //  Delete the directory name in its parent directory
            if (disk_image_buffer.data_block[disk_image_buffer.inode_block[id].father].data[i] == 1 and (disk_image_buffer.data_block[disk_image_buffer.inode_block[id].father].data[i + 1] << 8) + disk_image_buffer.data_block[disk_image_buffer.inode_block[id].father].data[i + 2] == id) {
                memset(disk_image_buffer.data_block[disk_image_buffer.inode_block[id].father].data + i, 0, 256);
                break;
            }
        }
        disk_image_buffer.remove(id);
    }
    if (!disk_image_buffer.block_is_used(current_path)) {
        //  RUN! ESCAPE THIS DIRECTORY! IT'S JUST BEEN DESTROYED!
        current_path = 0;
    }
}

/*
 WRITE STRING TO A FILE
 The structure of this function is similar to COMMAND_mkdir
 */
void COMMAND_echo(const char* str, const char* path) {
    static char new_name[253];
    memset(new_name, 0, sizeof(new_name));
    int p, id_path;
    for (p = 0; path[p] != '\0'; p ++);
    if (path[-- p] == '/') {
        p --;
    }
    if (p == -1) {
        cout << "Failed to create file!\nNo file name entered." << endl;
        return;
    }
    for (; p > 0 and path[p] != '/'; p --);
    if (p == 0) {
        if (path[0] == '/') {
            id_path = 0;
            for (int i = 0; path[i + 1] != '\0' and path[i + 1] != '/'; i ++) {
                new_name[i] = path[i + 1];
            }
        } else {
            id_path = current_path;
            for (int i = 0; path[i] != '\0' and path[i] != '/'; i ++) {
                new_name[i] = path[i];
            }
        }
        if (new_name[0] == '\0') {
            cout << "Failed to create file!\nNo file name entered." << endl;
            return;
        }
        if (disk_image_buffer.get_dir_id_by_name_under(new_name, id_path) >= 0) {
            cout << "Failed to create file!\nA directory with the same name already exists." << endl;
            return;
        } else if (disk_image_buffer.get_dir_id_by_name_under(new_name, id_path) == -2) {
            char in[3];
            cout << "A file with the same name already exists. Overwrite? (Y/N)" << endl;
            cin.getline(in, 3);
            if (cin.fail()) {
                cin.clear();
                cin.ignore(INT_MAX, '\n');
            }
            if (in[1] != '\0' or (in[0] != 'y' and in[0] != 'n' and in[0] != 'Y' and in[0] != 'N')) {
                cout << "Command canceled." << endl;
            } else if (in[0] == 'Y' or in[0] == 'y') {
                int id_item = disk_image_buffer.get_item_id_by_name_under(new_name, id_path);
                memset(disk_image_buffer.data_block[id_item].data, 0, 4096);
                for (int i = 0; str[i] != '\0'; i ++) {
                    disk_image_buffer.data_block[id_item].data[i] = str[i];
                }
            }
            return;
        }
        int new_node = disk_image_buffer.find_empty_block();
        if (new_node == -1) {
            cout << "Failed to create file!\nDisk full." << endl;
            return;
        }
        if (disk_image_buffer.data_block[id_path].new_child(new_node, new_name)) {
            disk_image_buffer.initialize_data_block_as_file(new_node);
            for (int i = 0; str[i] != '\0'; i ++) {
                disk_image_buffer.data_block[new_node].data[i] = str[i];
            }
        }
        return;
    } else {
        for (int i = 0; path[p + i + 1] != '\0' and path[p + i + 1] != '/'; i ++) {
            new_name[i] = path[p + i + 1];
        }
        id_path = disk_image_buffer.get_dir_id_by_path(path, p + 1);
        if (disk_image_buffer.get_dir_id_by_name_under(new_name, id_path) >= 0) {
            cout << "Failed to create file!\nA directory with the same name already exists." << endl;
            return;
        } else if (disk_image_buffer.get_dir_id_by_name_under(new_name, id_path) == -2) {
            char in[3];
            cout << "A file with the same name already exists. Overwrite? (Y/N)" << endl;
            cin.getline(in, 3);
            if (cin.fail()) {
                cin.clear();
                cin.ignore(INT_MAX, '\n');
            }
            if (in[1] != '\0' or (in[0] != 'y' and in[0] != 'n' and in[0] != 'Y' and in[0] != 'N')) {
                cout << "Command canceled." << endl;
            } else if (in[0] == 'Y' or in[0] == 'y') {
                int id_item = disk_image_buffer.get_item_id_by_name_under(new_name, id_path);
                memset(disk_image_buffer.data_block[id_item].data, 0, 4096);
                for (int i = 0; str[i] != '\0'; i ++) {
                    disk_image_buffer.data_block[id_item].data[i] = str[i];
                }
            }
            return;
        }
        int new_node = disk_image_buffer.find_empty_block();
        if (new_node == -1) {
            cout << "Failed to create file!\nDisk full." << endl;
            return;
        }
        if (disk_image_buffer.data_block[id_path].new_child(new_node, new_name)) {
            disk_image_buffer.initialize_data_block_as_file(new_node);
            for (int i = 0; str[i] != '\0'; i ++) {
                disk_image_buffer.data_block[new_node].data[i] = str[i];
            }
        }
    }
}

/*
 PRINT CONTENT OF A FILE
 */
void COMMAND_cat(const char* path) {
    int id = disk_image_buffer.get_file_id_by_path(path);
    if (id >= 0) {
        cout << disk_image_buffer.data_block[id].data << endl;
    }
}

/*
 REMOVE A FILE
 */
void COMMAND_rm(const char* path) {
    int id = disk_image_buffer.get_file_id_by_path(path);
    if (id >= 0) {
        disk_image_buffer.remove(id);
    }
}

/*
 PRINT DISK USAGE
 */
void COMMAND_df() {
    int result = disk_image_buffer.used_space();
    cout << "Inode used: \t\t" << result << "/4096" << endl;
    cout << "Data block used: \t" << result << "/4096" << endl;
}
