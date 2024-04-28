#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/stat.h>
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

void Extract(const char *Path){




}


void Build(const char *Path){
	static int Init = 0;
	static char* Vault_history = NULL;
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
				printf("%s\n",Recursive_PATH);
		}
		if (Next_Folder->d_type == DT_DIR){
			printf("%s\n",Next_Folder -> d_name);
			Build(Recursive_PATH);
		}
	}
	closedir(dir);
}

