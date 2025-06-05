// this code is orignally based on https://github.com/switchbrew/switch-examples/tree/master/fs/save (public domain)

#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <filesystem>
#include <iostream>
#include <vector>
#include <switch.h>

#include "Copy.h"
#include "ReadFile.h"
#include "WriteFile.h"
#include "Decompress.h"

namespace fs = std::filesystem;

const size_t NPOS = -1;

int CHOSEN_LIST_INT = -1;
int CHOSEN_LIST_MIN = 0;
int CHOSEN_LIST_MAX = 0;
int CHOSEN_COMMITTED = 0; // boolean

int file_count;
char *file_list[256]; // If you have more than 256 saves, no you don't.
// char **file_list;

const std::string OPENING_SAVE_TAG = "<SaveGame xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\">";
const std::string ALLOW_CHAT_CHEATS_TRUE = "<allowChatCheats>true</allowChatCheats>";
const std::string ALLOW_CHAT_CHEATS_FALSE = "<allowChatCheats>false</allowChatCheats>";

int main(int argc, char **argv)
{
    printf("enablecheatcheatsnx - press + to exit.\n\n");
    Result rc=0;

    DIR* dir;
    DIR* dir2;
    struct dirent* ent;

    AccountUid uid={0};
    u64 application_id=0x0100E65002BB8000;//ApplicationId of the save to mount, in this case SDV.

    consoleInit(NULL);

    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeDefault(&pad);

    rc = accountInitialize(AccountServiceType_Application);
    if (R_FAILED(rc)) {
        printf("accountInitialize() failed: 0x%x\n", rc);
    }

    if (R_SUCCEEDED(rc)) {
        // rc = accountGetPreselectedUser(&uid);

        // if (R_FAILED(rc)) {
        //     printf("accountGetPreselectedUser() failed: 0x%x, using pselShowUserSelector..\n", rc);

        //     /* Create player selection UI settings */
            
        // }
        PselUserSelectionSettings settings;
        memset(&settings, 0, sizeof(settings));

        rc = pselShowUserSelector(&uid, &settings);

        if (R_FAILED(rc)) {
            printf("pselShowUserSelector() failed: 0x%x\n", rc);
        }

        accountExit();
    }


    if (R_SUCCEEDED(rc)) {
        printf("Using application_id=0x%016lx\n", application_id);
    }

    if (R_SUCCEEDED(rc)) {
        rc = fsdevMountSaveData("save", application_id, uid);
        if (R_FAILED(rc)) {
            printf("fsdevMountSaveData() failed: 0x%x\n", rc);
        }
    }

    if (R_SUCCEEDED(rc)) {
        dir2 = opendir("save:/");
        if(dir2==NULL) {
            printf("Failed to open dir.\n");
        } else {
            int file_count = 0;
            dir = opendir("save:/");
            if (dir == NULL) {
                perror("Could not open directory");
                return 1;
            }

            while ((ent = readdir(dir)) != NULL) {
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    file_count++;
                }
            }
            closedir(dir);

            dir = opendir("save:/");
            if (dir == NULL) {
                perror("Could not re-open directory");
                return 1;
            }

            int i = 0;
            while ((ent = readdir(dir)) != NULL) {
                if (strcmp(ent->d_name, "startup_preferences") == 0) {
                    file_count--;
                    continue;
                } else if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                    file_list[i] = (char*) malloc(strlen(ent->d_name) + 1); 
                    if (file_list[i] != NULL) { 
                        strcpy(file_list[i], ent->d_name); 
                        i++;
                    }
                }
            }
            closedir(dir);

            printf("Dir-listing for 'save:/' (%d items):\n\n", file_count);
            CHOSEN_LIST_MAX = file_count - 1;
            for (i = 0; i < file_count; i++) {
                printf("(%d) %s\n", i, file_list[i]);
            }

            printf("\nUse up/down to change selected option. A to select\n");
            printf("Chosen option: ?\r"); // \r makes the line be overwritten by the next print
        }
    }

    // Main loop
    while(appletMainLoop())
    {
        padUpdate(&pad);

        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus) break; // break in order to return to hbmenu
        
        if (kDown & HidNpadButton_AnyUp && !CHOSEN_COMMITTED) { // move option up
            if (++CHOSEN_LIST_INT > CHOSEN_LIST_MAX) CHOSEN_LIST_INT = CHOSEN_LIST_MIN;
            printf("Chosen option: %d  \r", CHOSEN_LIST_INT); // \r makes the line be overwritten by the next print
        };
        
        if (kDown & HidNpadButton_AnyDown && !CHOSEN_COMMITTED) { // move option down
            if (--CHOSEN_LIST_INT < CHOSEN_LIST_MIN) CHOSEN_LIST_INT = CHOSEN_LIST_MAX;
            printf("Chosen option: %d  \r", CHOSEN_LIST_INT); // \r makes the line be overwritten by the next print
        };

        if (kDown & HidNpadButton_A) {
            if (CHOSEN_LIST_INT < CHOSEN_LIST_MIN || CHOSEN_LIST_INT > CHOSEN_LIST_MAX) {
                printf("Invalid choice: %d\r", CHOSEN_LIST_INT);
            } else {
                CHOSEN_COMMITTED = 1;
                printf("Chose save: (%d) %s\n\n", CHOSEN_LIST_INT, file_list[CHOSEN_LIST_INT]);

                std::string save_file_path = "save:/" + std::string(file_list[CHOSEN_LIST_INT]) + "/" + std::string(file_list[CHOSEN_LIST_INT]);
                std::string backup_file_path = save_file_path + ".eccnx_bak";

                if (access(backup_file_path.c_str(), 0 ) == 0) { // not sure why fs::exists doesn't work, but it doesn't?
                    printf("Removing existing backup.\n");
                    std::remove(backup_file_path.c_str());
                }
                if (access(save_file_path.c_str(), 0 ) != 0) { // not sure why fs::exists doesn't work, but it doesn't?
                    printf("For some reason, this save's main file does not exist. Confirm it works in game and try again.\n");
                    continue;
                }

                printf("Backing up save...\n");
                CopyFile(save_file_path.c_str(), backup_file_path.c_str());
                printf("\n");

                std::vector<char> save_bytes = readFileToBuffer(save_file_path);
                size_t save_bytes_size = save_bytes.size();
                
                if (save_bytes_size < 100) {
                    printf("Failed to read save file. Only contains %zu bytes.", save_bytes_size);
                    continue;
                }
                printf("File read successfully, size: %zu bytes.\n", save_bytes_size);

                std::string decoded_save = "";

                char first_byte = save_bytes.front(); // This is also how vanilla checks if a save is compressed. Around SaveGame.cs Line 671
                if (static_cast<unsigned char>(first_byte) == 0x78) { // Decimal 120
                    std::vector<char> decompressed_bytes = decompressZlib(save_bytes);
                    size_t decompressed_bytes_size = decompressed_bytes.size();
                    if (decompressed_bytes_size < 100) {
                        printf("Failed to decompress save file. Only contains %zu bytes decompressed.\n", decompressed_bytes_size);
                        continue;
                    }
                    decoded_save = std::string(decompressed_bytes.begin(), decompressed_bytes.end());

                } else {
                    printf("Save is not compressed\n");
                    decoded_save = std::string(save_bytes.begin(), save_bytes.end());
                }

                printf("First 20 characters of save: %.20s\n", decoded_save.c_str());
                
                if (decoded_save.find(ALLOW_CHAT_CHEATS_TRUE) != NPOS) {
                    printf("\nallowChatCheats already enabled. Nothing needs to be done. You may now exit.\n");
                    continue;
                }

                if (decoded_save.find("<allowChatCheats>") == NPOS) {
                    // save doesnt have the tag
                    size_t pos = decoded_save.find(OPENING_SAVE_TAG);
                    decoded_save.insert(pos + 1, ALLOW_CHAT_CHEATS_TRUE);
                    printf("\nAdded and enabled allowChatCheats.");
                } else if (decoded_save.find(ALLOW_CHAT_CHEATS_TRUE) == NPOS && decoded_save.find(ALLOW_CHAT_CHEATS_FALSE) != NPOS) {
                    decoded_save.replace(
                        decoded_save.find(ALLOW_CHAT_CHEATS_FALSE),
                        ALLOW_CHAT_CHEATS_FALSE.length(),
                        ALLOW_CHAT_CHEATS_TRUE
                    );
                }

                
                writeStringToFile(decoded_save, save_file_path);
                fsdevCommitDevice("save");
                printf("\nCommitted save. You may now exit.\n");
            }
        }

        consoleUpdate(NULL);
    }

    consoleExit(NULL);
    return 0;
}
