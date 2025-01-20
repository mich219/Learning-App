#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>


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



