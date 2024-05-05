#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
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


void Learn(cJSON *item, const char* parent){

 if (!item || !cJSON_IsObject(item)) {
        return;
    }

    cJSON* current;
    cJSON_ArrayForEach(current, item) {
        char currentName[100]; // Assuming item names are less than 100 characters

        if (parent != NULL) {
            sprintf(currentName, "%s -> %s", parent, current->string);
        } else {
            sprintf(currentName, "%s", current->string);
        }

        printf("Title: %s\n", currentName);

        cJSON* familiarity = cJSON_GetObjectItemCaseSensitive(current, "Familiarity");
        cJSON* known = cJSON_GetObjectItemCaseSensitive(current, "Known");
        cJSON* answer = cJSON_GetObjectItemCaseSensitive(current, "Answer");

        if (familiarity) {
            printf("Familiarity: %s\n", familiarity->valuestring);
        }
        if (known) {
            printf("Known: %s\n", known->valuestring);
        }
        if (answer) {
            printf("Answer:\n");
            cJSON* lineList = cJSON_GetObjectItemCaseSensitive(answer, "Line_List");
            cJSON* text = cJSON_GetObjectItemCaseSensitive(answer, "Text");

            if (lineList) {
                printf("Line List: %s\n", lineList->valuestring);
            }
            if (text) {
                printf("Text: %s\n", text->valuestring);
            }

            // Ask for user input to modify Familiarity and Known entries
            printf("Enter Familiarity (0-100): ");
            scanf("%s", familiarity->valuestring);
            printf("Enter Known (True/False): ");
            scanf("%s", known->valuestring);
        }

        printf("\n");

        // Recursively traverse child items
        Learn(current->child, currentName);
    }
}












/*
	if (item && cJSON_IsObject(item)) {
		cJSON *answer = cJSON_GetObjectItemCaseSensitive(item, "answer");
		if (answer && cJSON_IsString(answer)) {
			printf("Answer: %s\n", answer->valuestring);
		}
	}

	// Recursively search children of current item
	if (item && cJSON_IsArray(item)) {
		cJSON *child = item->child;
		while (child) {
			Learn(child);
			child = child->next;
		}
	}
*/















