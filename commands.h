//
//  commands.h
//  PDB-Final-Storage
//
//  Created by Yixiang Tu on 22/12/2017.
//  Copyright © 2017 涂轶翔. All rights reserved.
//

#ifndef commands_h
#define commands_h

void COMMAND_help();
void COMMAND_pwd(int id);
void COMMAND_cd(const char* path);
void COMMAND_mkdir(const char* path);
void COMMAND_ls(int id);
void COMMAND_ls(const char* path);
void COMMAND_rmdir(const char* path);
void COMMAND_echo(const char* str, const char* path);
void COMMAND_cat(const char* path);
void COMMAND_rm(const char* path);
void COMMAND_save();
void COMMAND_exit();
void COMMAND_df();


#endif /* commands_h */
