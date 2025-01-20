#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "../include/File_Handling.h"
#include "../include/File_Processing.h"
#include "../include/Learning_Interface.h"
#include "../include/cJSON.h"




int Build(const char *Path){
	int Init = 0;
	char* Vault_history = NULL;
	char Recursive_PATH[PATH_MAX];
	char *string;
	cJSON *file_json;

	if (!Init){
		Vault_history = Get_Vault_History(Path);
		Init = 1;
	}
	DIR *dir = opendir(Path);
	struct dirent *Next_Folder;
	while ((Next_Folder = readdir(dir)) != NULL){
		if (Next_Folder->d_name[0] == '.') {continue;}
		snprintf(Recursive_PATH, sizeof(Recursive_PATH), "%s/%s", Path, Next_Folder->d_name);
		if (Next_Folder -> d_type == DT_REG && (strstr(Next_Folder->d_name, ".md") != NULL)){
			if (compare_dates(Vault_history,last_access_date(Recursive_PATH)) < 1) {
				file_json = Extract(Recursive_PATH);
				Store(file_json,Recursive_PATH);
			}
		}
		if (Next_Folder->d_type == DT_DIR){
			printf("%s\n",Next_Folder->d_name);
			Build(Recursive_PATH);
		}
	}
	closedir(dir);
	return 0;
}


char* Find_Field_Path(const char *Path, const char *Selected_Field){

	char Recursive_PATH[PATH_MAX];
	DIR *dir = opendir(Path);
	struct dirent *Next_Folder;

	while ((Next_Folder = readdir(dir)) != NULL){
		if (Next_Folder->d_name[0] == '.') {continue;}
		snprintf(Recursive_PATH, sizeof(Recursive_PATH), "%s/%s", Path, Next_Folder->d_name);

		if (Next_Folder->d_type == DT_DIR){
			if (strcmp(Next_Folder->d_name, Selected_Field) == 0){
				return strcat(strcat(strcat(Recursive_PATH, "/"), Selected_Field), ".json");
			}
			Find_Field_Path(Recursive_PATH, Path);
		}
	}
	closedir(dir);
	return NULL;
}


void Learn(cJSON *item) {

void readInputUntilEnter() {
    printf("\n");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (ch == '_') {
            system("clear"); // Clear the console screen
            printf("\n:");
        }
    }
}

    // Define a constant for the maximum number of keys
    #define MAX_KEYS 100

    // Array to store printed keys
    static char printedKeys[MAX_KEYS][100] = {0};
    static int numPrintedKeys = 0;

    // Function to check if a key has been printed
    bool keyAlreadyPrinted(const char* key) {
        for (int i = 0; i < numPrintedKeys; i++) {
            if (strcmp(printedKeys[i], key) == 0) {
                return true;
            }
        }
        return false;
    }

    // Function to add a key to the printed keys array
    void addPrintedKey(const char* key) {
        if (numPrintedKeys < MAX_KEYS) {
            strcpy(printedKeys[numPrintedKeys], key);
            numPrintedKeys++;
        }
    }

    cJSON* current = item->child;

    // Traverse through each child item
    while (current != NULL) {
        // Check if the current key has already been printed
        if (!keyAlreadyPrinted(current->string)) {
            // Print the current item string
            printf("%s ", current->string);
            addPrintedKey(current->string);
        }

        // Check if the current item has children
        if (current->child && cJSON_IsObject(current->child)) {
            // Recursively call Learn() for children
            Learn(current);
        } else {
            // If it's a leaf node, print its value
            cJSON* answer = cJSON_GetObjectItemCaseSensitive(current, "Answer");
            if (answer && cJSON_IsArray(answer)) {
                printf("\n");
        readInputUntilEnter();

                printf("Answer:\n");
                int i;
                for (i = 0; i < cJSON_GetArraySize(answer); i++) {
                    cJSON* answerItem = cJSON_GetArrayItem(answer, i);
                    printf("%s", cJSON_GetStringValue(answerItem));
                }
                printf("\n"); // Add a newline after printing the answer
            }
        }

        // Move to the next sibling item
        current = current->next;
    }
}






