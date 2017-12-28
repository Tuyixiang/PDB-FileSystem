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
    cout << "Save before exitting? (Y/N)" << endl;
    if (ask()) {
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
    int p, id_path, length;
    for (p = 0; path[p] != '\0'; p ++);
    for (length = 0; str[length]; length ++);
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
            cout << "A file with the same name already exists. Overwrite? (Y/N)" << endl;
            if (ask()) {
                int id_item = disk_image_buffer.get_item_id_by_name_under(new_name, id_path);
                disk_image_buffer.inode_block[id_item].file_size = length;
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
            disk_image_buffer.inode_block[new_node].file_size = length;
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
            cout << "A file with the same name already exists. Overwrite? (Y/N)" << endl;
            if (ask()) {
                int id_item = disk_image_buffer.get_item_id_by_name_under(new_name, id_path);
                disk_image_buffer.inode_block[id_item].file_size = length;
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
            disk_image_buffer.inode_block[new_node].file_size = length;
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
        int total = disk_image_buffer.inode_block[id].file_size;
        while (total > 0) {
            if (total >= 4096) {
                for (int i = 0; i < 4096; i ++) {
                    cout << disk_image_buffer.data_block[id].data[i];
                }
                total -= 4096;
                id = disk_image_buffer.inode_block[id].next_node;
            } else {
                for (int i = 0; i < total; i ++) {
                    cout << disk_image_buffer.data_block[id].data[i];
                }
                break;
            }
        }
        cout << endl;
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

/*
 IMPORT AN EXTERNAL FILE
 imp example.txt /dir/
 */
void COMMAND_imp(const char* file_name, const char* path) {
    //  Try to open the file named "..."
    in_file.open(file_name, ios::in | ios::binary);
    
    if (in_file.is_open()) {
        cout << "External file located." << endl;
        //  The name of external file may be "../resources/text/sample.txt"
        //  In this case, the name of the new file should be "sample.txt"
        int p, length;
        char new_name[253];
        memset(new_name, 0, sizeof(new_name));
        for (p = 0; file_name[p] != '\0'; p ++);
        length = p;
        for (; p > 0 and file_name[p] != '/'; p --);
        if (p > 0 or file_name[0] == '/') {
            for (int i = 0; i < length - p; i ++) {
                new_name[i] = file_name[i + p + 1];
            }
        } else {
            for (int i = 0; i < length; i ++) {
                new_name[i] = file_name[i];
            }
        }
        long long file_begin, file_end;
        int file_size;
        file_begin = in_file.tellg();
        in_file.seekg(0, ios::end);
        file_end = in_file.tellg();
        file_size = (int) (file_end - file_begin);
        if (disk_image_buffer.used_space() < 4096 and file_size > (4096 - disk_image_buffer.used_space()) * 4096) {
            cout << "Failed to import file!" << endl;
            cout << "Not enough space for external file." << endl;
            in_file.close();
        } else if (file_size == 0) {
            int target = disk_image_buffer.find_empty_block();
            int id_path = disk_image_buffer.get_dir_id_by_path(path);
            if (int id = disk_image_buffer.get_file_id_by_name_under(new_name, id_path) >= 0) {
                cout << "A file with the same name already exists. Overwrite? (Y/N)" << endl;
                if (ask()) {
                    disk_image_buffer.remove(id);
                } else {
                    return;
                }
            }
            if (int id = disk_image_buffer.get_dir_id_by_name_under(new_name, id_path) >= 0) {
                cout << "Failed to import file!\nA directory with the same name already exists." << endl;
                return;
            }
            if (target == -1) {
                //  This definitely shouldn't be happening because there should be enough empty space
                cout << "Oh, nooooOOOOO!" << endl;
                cin >> target;
                in_file.close();
                return;
            }
            if (disk_image_buffer.data_block[id_path].new_child(target, new_name)) {
                disk_image_buffer.initialize_data_block_as_file(target);
                cout << "Import complete." << endl;
            }   //  Otherwise, new_child() will print "Directory full"
        } else {
            in_file.seekg(0, ios::beg);
            while (file_size > 0) {
                int target = disk_image_buffer.find_empty_block();
                int id_path = disk_image_buffer.get_dir_id_by_path(path);
                if (int id = disk_image_buffer.get_file_id_by_name_under(new_name, id_path) >= 0) {
                    cout << "A file with the same name already exists. Overwrite? (Y/N)" << endl;
                    if (ask()) {
                        disk_image_buffer.remove(id);
                    } else {
                        return;
                    }
                }
                if (int id = disk_image_buffer.get_dir_id_by_name_under(new_name, id_path) >= 0) {
                    cout << "Failed to import file!\nA directory with the same name already exists." << endl;
                    return;
                }
                if (target == -1) {
                    //  This definitely shouldn't be happening because there should be enough empty space
                    cout << "Oh, nooooOOOOO!" << endl;
                    cin >> target;
                    in_file.close();
                    return;
                }
                if (disk_image_buffer.data_block[id_path].new_child(target, new_name)) {
                    disk_image_buffer.initialize_data_block_as_file(target);
                    in_file.read((char*) disk_image_buffer.data_block[target].data, min(4096, file_size));
                    disk_image_buffer.inode_block[target].file_size = (int) file_size;
                    file_size -= 4096;
                    while (file_size > 0) {
                        disk_image_buffer.inode_block[target].continues = true;
                        target = disk_image_buffer.inode_block[target].next_node = disk_image_buffer.find_empty_block();
                        if (target == -1) {
                            //  This definitely shouldn't be happening because there should be enough empty space
                            cout << "Oh, nooooOOOOO!" << endl;
                            cin >> target;
                            in_file.close();
                            return;
                        }
                        disk_image_buffer.initialize_data_block_as_file(target);
                        disk_image_buffer.inode_block[target].continued = true;
                        in_file.read((char*) disk_image_buffer.data_block[target].data, min(4096, file_size));
                        file_size -= 4096;
                    }
                    cout << "Import complete." << endl;
                }   //  Otherwise, new_child() will print "Directory full"
            }
        }
    } else {
        cout << file_name << ": no such file." << endl;
    }
    in_file.close();
}

/*
 IMPORT AN EXTERNAL FILE
 This one imports the file under current directory.
 The only difference is that this doesn't search for the directory.
 */
void COMMAND_imp(const char* file_name) {
    //  Try to open the file named "..."
    in_file.open(file_name, ios::in | ios::binary);
    
    if (in_file.is_open()) {
        cout << "External file located." << endl;
        //  The name of external file may be "../resources/text/sample.txt"
        //  In this case, the name of the new file should be "sample.txt"
        int p, length;
        char new_name[253];
        memset(new_name, 0, sizeof(new_name));
        for (p = 0; file_name[p] != '\0'; p ++);
        length = p;
        for (; p > 0 and file_name[p] != '/'; p --);
        if (p > 0 or file_name[0] == '/') {
            for (int i = 0; i < length - p; i ++) {
                new_name[i] = file_name[i + p + 1];
            }
        } else {
            for (int i = 0; i < length; i ++) {
                new_name[i] = file_name[i];
            }
        }
        long long file_begin, file_end;
        int file_size;
        file_begin = in_file.tellg();
        in_file.seekg(0, ios::end);
        file_end = in_file.tellg();
        file_size = (int) (file_end - file_begin);
        if (disk_image_buffer.used_space() < 4096 and file_size > (4096 - disk_image_buffer.used_space()) * 4096) {
            cout << "Failed to import file!" << endl;
            cout << "Not enough space for external file." << endl;
            in_file.close();
        } else if (file_size == 0) {
            int target = disk_image_buffer.find_empty_block();
            if (target == -1) {
                //  This definitely shouldn't be happening because there should be enough empty space
                cout << "Oh, nooooOOOOO!" << endl;
                cin >> target;
                in_file.close();
                return;
            }
            if (int id = disk_image_buffer.get_file_id_by_name_under(new_name, 0) >= current_path) {
                cout << "A file with the same name already exists. Overwrite? (Y/N)" << endl;
                if (ask()) {
                    disk_image_buffer.remove(id);
                } else {
                    return;
                }
            }
            if (int id = disk_image_buffer.get_dir_id_by_name_under(new_name, 0) >= current_path) {
                cout << "Failed to import file!\nA directory with the same name already exists." << endl;
                return;
            }
            disk_image_buffer.initialize_data_block_as_file(target);
            cout << "Import complete." << endl;
            cout << "Warning: file is empty." << endl;
        } else {
            in_file.seekg(0, ios::beg);
            while (file_size > 0) {
                int target = disk_image_buffer.find_empty_block();
                int id_path = current_path;
                if (target == -1) {
                    //  This definitely shouldn't be happening because there should be enough empty space
                    cout << "Oh, nooooOOOOO!" << endl;
                    cin >> target;
                    in_file.close();
                    return;
                }
                if (int id = disk_image_buffer.get_file_id_by_name_under(new_name, current_path) >= 0) {
                    cout << "A file with the same name already exists. Overwrite? (Y/N)" << endl;
                    if (ask()) {
                        disk_image_buffer.remove(id);
                    } else {
                        return;
                    }
                }
                if (int id = disk_image_buffer.get_dir_id_by_name_under(new_name, current_path) >= 0) {
                    cout << "Failed to import file!\nA directory with the same name already exists." << endl;
                    return;
                }
                if (disk_image_buffer.data_block[id_path].new_child(target, new_name)) {
                    disk_image_buffer.initialize_data_block_as_file(target);
                    in_file.read((char*) disk_image_buffer.data_block[target].data, min(4096, file_size));
                    disk_image_buffer.inode_block[target].file_size = (int) file_size;
                    file_size -= 4096;
                    while (file_size > 0) {
                        disk_image_buffer.inode_block[target].continues = true;
                        target = disk_image_buffer.inode_block[target].next_node = disk_image_buffer.find_empty_block();
                        if (target == -1) {
                            //  This definitely shouldn't be happening because there should be enough empty space
                            cout << "Oh, nooooOOOOO!" << endl;
                            cin >> target;
                            in_file.close();
                            return;
                        }
                        disk_image_buffer.initialize_data_block_as_file(target);
                        disk_image_buffer.inode_block[target].continued = true;
                        in_file.read((char*) disk_image_buffer.data_block[target].data, min(4096, file_size));
                        file_size -= 4096;
                    }
                    cout << "Import complete." << endl;
                }   //  Otherwise, new_child() will print "Directory full"
            }
        }
    } else {
        cout << file_name << ": no such file." << endl;
    }
    in_file.close();
}

/*
 EXPORT A FILE
 */
void COMMAND_exp(const char* path) {
    int id = disk_image_buffer.get_file_id_by_path(path);
    if (id >= 0) {
        const char* name = disk_image_buffer.get_name_by_file_id(id);
        int total = disk_image_buffer.inode_block[id].file_size;
        ofstream out;
        out.open(name, ios::out | ios::binary | ios::trunc);
        while (total > 0) {
            out.write((char*) disk_image_buffer.data_block[id].data, min(total, 4096));
            total -= 4096;
            id = disk_image_buffer.inode_block[id].next_node;
        }
    }
}
