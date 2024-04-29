#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <ctype.h>
#include "../include/cJSON.h"


void Update_Vault_History(const char *Path){
	time_t rawtime;
	struct tm *info;
	time(&rawtime);
	info = localtime(&rawtime);
	char date[11];
	strftime(date, sizeof(date), "%Y-%m-%d", info);

	FILE *Update_file = fopen(Path, "w");
	fprintf(Update_file , "%s", date);
	fclose(Update_file);
}

char* Get_Vault_History(const char *Path){
	static int Initialised = 0;
	static char *Last_Time_checked = NULL; // Allocate memory for Last_Time_checked

	const char *last_separator = strrchr(Path, '/');
	size_t length = strlen(last_separator + 1);
	char *last_folder_name = malloc(length + 5); // 5 for ".txt" and null terminator
	strncpy(last_folder_name, last_separator + 1, length);
	strcpy(last_folder_name + length, ".txt");
	last_folder_name[length + 4] = '\0'; // Null-terminate the string
	char Notes_History_File_Path[NAME_MAX];
	Last_Time_checked = malloc(10); 
	snprintf(Notes_History_File_Path, sizeof(Notes_History_File_Path), "%s%s", "../Notes_History/", last_folder_name);
	FILE *file = fopen(Notes_History_File_Path, "r");
	if (file == NULL) {
		free(last_folder_name);
		strcpy(Last_Time_checked, "1969-12-12"); // Set default date
		Update_Vault_History(Notes_History_File_Path);
		return Last_Time_checked;
	}else{
		fscanf(file, "%10s", Last_Time_checked);
		fclose(file);
		free(last_folder_name);
		Update_Vault_History(Notes_History_File_Path);
		return Last_Time_checked;
	}
}




char* last_access_date(const char *Path) {
    struct stat file_stat;
    if (stat(Path, &file_stat) == 0) {
        // Get the last accessed time from the file status
        time_t last_accessed_time = file_stat.st_atime;

        // Convert the last accessed time to a local time representation
        struct tm *time_info = localtime(&last_accessed_time);
        if (time_info == NULL) {
            perror("Error converting time");
            exit(EXIT_FAILURE);
        }

        // Format the date as a string
        static char date_str[11]; // YYYY-MM-DD\0
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", time_info);

        return date_str;
    } else {
        perror("Error getting file attributes");
        exit(EXIT_FAILURE);
    }
}


int compare_dates(const char *date1, const char *date2) {
    int year1, month1, day1;
    int year2, month2, day2;
    
    sscanf(date1, "%d-%d-%d", &year1, &month1, &day1);
    sscanf(date2, "%d-%d-%d", &year2, &month2, &day2);

    if (year1 < year2) return -1;
    if (year1 > year2) return 1;
    if (month1 < month2) return -1;
    if (month1 > month2) return 1;
    if (day1 < day2) return -1;
    if (day1 > day2) return 1;
    return 0;  // dates are equal
}



//locate file, 
//create object from file 
//implement object or create JSON from object

cJSON* read_json_file(const char *filename) {
	FILE *file = fopen(filename, "r");
	
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char *buffer = (char *)malloc(size + 1);
	fread(buffer, 1, size, file);
	buffer[size] = '\0'; // Null-terminate the string

	fclose(file);

	// Parse the JSON data
	cJSON *root = cJSON_Parse(buffer);

	free(buffer);
	return root;
}



void Extract(const char *Path, char *Field){

	char json_file_name[256];
	cJSON *root = NULL; 
	snprintf(json_file_name, sizeof(json_file_name),"%s.json",Field);

	char json_path[256];
	strcpy(json_path,Path);

	char *filename = strrchr(json_path, '/');
	filename++;
	strcpy(filename,json_file_name);

	FILE *JSON_file = fopen(json_path, "r");
	if(JSON_file){
		fclose(JSON_file);
		root = read_json_file(json_path);
	}else{
		root = cJSON_CreateArray();
	}

	
	FILE *file = fopen(Path, "r");
   char line[1024];
	cJSON *file_entry = cJSON_CreateObject();
	cJSON_AddItemToObject(root,json_file_name,file_entry);
	cJSON *Last = file_entry;
	cJSON *Type; 
	cJSON *Element;
	cJSON *Concept;
	cJSON *Fact;

	while (fgets(line, sizeof(line), file)) {
		if (line == NULL) {break;}

		size_t len = strlen(line);
		int code_block_started = 0;

		if (strncmp(line, "#",1) == 0) {
			Type = cJSON_CreateObject();
			cJSON_AddItemToObject(file_entry,strdup(line + 1),Type);
			Last = Type;

		} else if (strncmp(line, "##", 2) == 0) {
			Element = cJSON_CreateObject();
			cJSON_AddItemToObject(Last,strdup(line + 2),Element);
			Last = Element;

		} else if (strncmp(line, "###", 3) == 0) {
			Concept = cJSON_CreateObject();
			cJSON_AddItemToObject(Last,strdup(line + 3),Concept);
			Last = Concept;

		} else if (strncmp(line, "####", 4) == 0) {
			Fact = cJSON_CreateObject();
			cJSON_AddItemToObject(Last,strdup(line + 4),Fact);
			Last = Fact;

		} else if (strncmp(line, "- ", 2) == 0) {
			cJSON_AddStringToObject(Last,"Awnser", strdup(line + 2));

		} else if (strlen(line) >= 5 && isdigit(line[0]) && line[strlen(line) - 1] == '.' && line[strlen(line) - 2] == ' ') {
			cJSON_AddStringToObject(Last,"Awnser", strdup(line + 2));
			cJSON_AddStringToObject(Last,"Awnser_type","Numbered_List");

		} else if (strncmp(line, "```", 3) == 0) {
			if (!code_block_started) {
				code_block_started = 1;
			} else {
				code_block_started = 0;
			}
		} else if (code_block_started) {
			cJSON_AddStringToObject(Last,"Awnser", line);
		} else {
			line[len - 1] = '\0'; // Replace newline with null terminator
			cJSON_AddStringToObject(Last,"Awnser", line);
		}
	}

   char *json_string = cJSON_Print(root);
	printf("%s",json_string);
	free(json_string);
	fclose(file);
	cJSON_Delete(root);
}








void Build(const char *Path){
	static int Init = 0;
	static char* Vault_history = NULL;
	static char* Field = NULL;
	if (!Init){
		Vault_history = Get_Vault_History(Path);
		Init = 1;
	}
	DIR *dir = opendir(Path);
	struct dirent *Next_Folder;
	while ((Next_Folder = readdir(dir)) != NULL){
	
		if (Next_Folder->d_name[0] == '.') {continue;}
		char Recursive_PATH[PATH_MAX];
		snprintf(Recursive_PATH, sizeof(Recursive_PATH), "%s/%s", Path, Next_Folder->d_name);

		if (Next_Folder -> d_type == DT_REG){
			if (compare_dates(Vault_history,last_access_date(Recursive_PATH)) < 1)
//				printf("%s\n",Recursive_PATH);
				Extract(Recursive_PATH,Field);
		}
		if (Next_Folder->d_type == DT_DIR){
			Field = Next_Folder -> d_name;
			Build(Recursive_PATH);
		}
	}
	closedir(dir);
}

