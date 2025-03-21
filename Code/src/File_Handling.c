#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

/**
 * File_Handling.c
 * 
 * This file handles date operations and history tracking for files and directories.
 * It manages last update timestamps for notes and determines which files need processing.
 */

/**
 * Updates the history file with the current date
 * 
 * @param Path The path to the history file that needs to be updated
 * 
 * This function writes today's date in YYYY-MM-DD format to the specified file,
 * creating it if it doesn't exist or overwriting it if it does.
 */
void Update_Vault_History(const char *Path){
	if (Path == NULL) {
		return;  // Handle null path gracefully
	}
	
	// Get current date in YYYY-MM-DD format
	time_t rawtime;
	struct tm *info;
	time(&rawtime);
	info = localtime(&rawtime);
	char date[11];  // 10 chars for YYYY-MM-DD + null terminator
	strftime(date, sizeof(date), "%Y-%m-%d", info);

	// Open the file and write the date
	FILE *Update_file = fopen(Path, "w");
	if (Update_file == NULL) {
		fprintf(stderr, "Warning: Could not open history file: %s\n", Path);
		return;
	}
	
	fprintf(Update_file, "%s", date);
	fclose(Update_file);
}

/**
 * Retrieves the last update date for a specified path
 * 
 * @param Path The directory path whose history we want to check
 * @return A string in YYYY-MM-DD format representing the last update date
 * 
 * This function gets the last time a directory was processed by:
 * 1. Extracting the folder name from the path
 * 2. Checking if a corresponding history file exists in Notes_History
 * 3. Reading the date from that file or creating a default (1969-12-12) if not found
 */
char* Get_Vault_History(const char *Path){
	// Static variable to hold the date string (persists between calls)
	static char Last_Time_checked[11]; // YYYY-MM-DD\0

	// Default date (very old) if we can't determine the actual date
	if (Path == NULL) {
		strcpy(Last_Time_checked, "1969-12-12");
		return Last_Time_checked;
	}

	// Extract folder name from path
	const char *last_separator = strrchr(Path, '/');
	if (last_separator == NULL) {
		strcpy(Last_Time_checked, "1969-12-12");
		return Last_Time_checked;
	}
	
	// Create filename for history file based on folder name
	size_t length = strlen(last_separator + 1);
	char *last_folder_name = malloc(length + 5); // 5 for ".txt" and null terminator
	if (last_folder_name == NULL) {
		strcpy(Last_Time_checked, "1969-12-12");
		return Last_Time_checked;
	}
	
	// Construct the history file name (foldername.txt)
	strncpy(last_folder_name, last_separator + 1, length);
	strcpy(last_folder_name + length, ".txt");
	last_folder_name[length + 4] = '\0'; // Ensure null-termination
	
	// Ensure history directory exists
	mkdir("Notes_History", 0755);
	
	// Build the full path to the history file
	char Notes_History_File_Path[NAME_MAX];
	snprintf(Notes_History_File_Path, sizeof(Notes_History_File_Path), "%s%s", "Notes_History/", last_folder_name);
	
	// Read date from history file or create with default date if it doesn't exist
	FILE *file = fopen(Notes_History_File_Path, "r");
	if (file == NULL) {
		strcpy(Last_Time_checked, "1969-12-12"); // Set default date
		Update_Vault_History(Notes_History_File_Path);
	} else {
		fscanf(file, "%10s", Last_Time_checked);
		fclose(file);
		Update_Vault_History(Notes_History_File_Path);
	}
	
	free(last_folder_name);
	return Last_Time_checked;
}

/**
 * Gets the last access date of a file in YYYY-MM-DD format
 * 
 * @param Path The path to the file to check
 * @return A string containing the last access date
 * 
 * Uses system stat() to get file metadata and extract the last access time.
 */
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

/**
 * Compares two date strings in YYYY-MM-DD format
 * 
 * @param date1 First date string
 * @param date2 Second date string
 * @return -1 if date1 < date2, 0 if equal, 1 if date1 > date2
 * 
 * This function is used to determine if a file needs to be updated
 * by comparing its last access date with the vault history date.
 */
int compare_dates(const char *date1, const char *date2) {
    int year1, month1, day1;
    int year2, month2, day2;
    
    // Parse the date strings into components
    sscanf(date1, "%d-%d-%d", &year1, &month1, &day1);
    sscanf(date2, "%d-%d-%d", &year2, &month2, &day2);

    // Compare each component in order of significance
    if (year1 < year2) return -1;
    if (year1 > year2) return 1;
    if (month1 < month2) return -1;
    if (month1 > month2) return 1;
    if (day1 < day2) return -1;
    if (day1 > day2) return 1;
    return 0;  // Dates are equal
}

