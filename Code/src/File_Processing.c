#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include "../include/cJSON.h"

/**
 * File_Processing.c
 * 
 * This file handles the conversion between markdown note files and JSON data.
 * It parses markdown files with hierarchical headings into cJSON structures,
 * and manages saving/loading the JSON data to/from disk.
 */

/**
 * Converts a markdown file into a hierarchical cJSON structure
 * 
 * @param Path Path to the markdown file to process
 * @return A cJSON object containing the hierarchical content of the markdown file
 * 
 * This function processes a markdown file by:
 * 1. Creating a hierarchy based on heading levels (# to ####)
 * 2. Placing content under each heading into an "Answer" array
 * 3. Maintaining the proper nesting structure of the document
 */
cJSON *Extract(const char *Path){
	// Extract the filename (without extension) from the path
	char Field[156];
	const char *MDfilename = strrchr(Path, '/');
	MDfilename++; // Skip past the '/'
	strncpy(Field, MDfilename, sizeof(Field));
	char *dot = strrchr(Field, '.');
	*dot = '\0'; // Remove the file extension

	// Open and prepare the markdown file
	FILE *file = fopen(Path, "r");
	if (file == NULL) {
		fprintf(stderr, "Error: Cannot open file %s\n", Path);
		return NULL;
	}
	
	char line[1024];
	cJSON *file_entry = cJSON_CreateObject();

	// Check file size
	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	// Objects to track the current hierarchy level
	cJSON *Last = file_entry; 
	cJSON *Type = NULL;      // # Level 1 heading
	cJSON *Element = NULL;   // ## Level 2 heading
	cJSON *Concept = NULL;   // ### Level 3 heading
	cJSON *Awnser = NULL;    // Content array
	cJSON *Fact = NULL;      // #### Level 4 heading

	// Track which levels we've seen in the document
	int truth_table[3] = {0,0,0};
	int Init = 0; // Whether we've started collecting answer content
	
	// Process the file line by line
	while (fgets(line, sizeof(line), file)) {
		size_t len = strlen(line); 
		
		// If this line is a heading (starts with #)
		if (strncmp(line, "#", 1) == 0) {
			line[len - 1] = '\0'; // Remove newline
			
			// If we were collecting answer content, add it to the current object
			if (Init) {
				cJSON_AddItemToObject(Last, "Answer", Awnser);
				Init = 0;
			}
			
			// Process different heading levels
			if (strncmp(line, "# ", 2) == 0) {
				// Level 1 heading (# Heading)
				Type = cJSON_CreateObject();
				cJSON_AddItemToObject(file_entry, strdup(line + 2), Type);
				truth_table[0] = 1;
				Last = Type;
			} else if (strncmp(line, "## ", 3) == 0) {
				// Level 2 heading (## Heading)
				Element = cJSON_CreateObject();
				
				// Determine the parent object based on which levels exist
				if (truth_table[0]) Last = Type;
				if (!truth_table[0]) Last = file_entry;
				
				cJSON_AddItemToObject(Last, strdup(line + 3), Element);
				truth_table[1] = 1;
				Last = Element;
			} else if (strncmp(line, "### ", 4) == 0) {
				// Level 3 heading (### Heading)
				Concept = cJSON_CreateObject();
				
				// Determine the parent object based on which levels exist
				if (truth_table[1]) Last = Element;
				if (!truth_table[0] && !truth_table[1]) Last = file_entry;
				if (truth_table[0] && !truth_table[1]) Last = Type;
				
				cJSON_AddItemToObject(Last, strdup(line + 4), Concept);
				truth_table[2] = 1;
				Last = Concept;
			} else if (strncmp(line, "#### ", 5) == 0){ 
				// Level 4 heading (#### Heading)
				Fact = cJSON_CreateObject();
				
				// Determine the parent object based on which levels exist
				if (!truth_table[0] && !truth_table[1] && !truth_table[2]) Last = file_entry;
				if (truth_table[0] && !truth_table[1] && !truth_table[2]) Last = Type;
				if (truth_table[1] && !truth_table[2]) Last = Element;
				if (truth_table[2]) Last = Concept;
				
				cJSON_AddItemToObject(Last, strdup(line + 5), Fact);
				Last = Fact; 
			}
		} else {
			// This is content (not a heading)
			// Initialize the answer array if needed
			if (!Init) {
				Awnser = cJSON_CreateArray();
				Init = 1;	
			}
			// Add the content line to the answer array
			cJSON_AddItemToArray(Awnser, cJSON_CreateString(line));
		}
	}
	
	// Add the final answer object if we have pending content
	if (Init) {
		cJSON_AddItemToObject(Last, "Answer", Awnser);
		Init = 0;
	}
	
	fclose(file);
	return file_entry;	
}

/**
 * Reads and parses a JSON file into a cJSON structure
 * 
 * @param filename Path to the JSON file to read
 * @return A cJSON object containing the parsed content, or NULL on error
 * 
 * This function handles all error cases and memory management for reading JSON files.
 */
cJSON* read_json_file(const char *filename) {
	if (filename == NULL) {
		return NULL;
	}
	
	// Open the file
	FILE *file = fopen(filename, "r");
	if (file == NULL) {
		fprintf(stderr, "Error: Cannot open file %s\n", filename);
		return NULL;
	}
	
	// Check file size before allocating memory
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	cJSON *root;
	// Handle empty files
	if (size == 0) {
		fclose(file);
		root = cJSON_CreateObject();
		return root;
	}

	// Allocate memory for file contents
	char *buffer = (char *)malloc(size + 1);
	if (!buffer) {
		fprintf(stderr, "Error: Memory allocation failed for %s\n", filename);
		fclose(file);
		return NULL;
	}
	
	// Read the entire file
	size_t bytes_read = fread(buffer, 1, size, file);
	if (bytes_read != size) {
		fprintf(stderr, "Error: Could not read entire file %s\n", filename);
		free(buffer);
		fclose(file);
		return NULL;
	}
	
	buffer[size] = '\0'; // Ensure null-termination
	fclose(file);

	// Parse the JSON data
	root = cJSON_Parse(buffer);
	if (root == NULL) {
		fprintf(stderr, "Error: Failed to parse JSON in %s\n", filename);
	}

	free(buffer);
	return root;
}

/**
 * Saves a cJSON structure to a JSON file corresponding to a markdown file
 * 
 * @param file_object The cJSON object to save
 * @param Path Path to the original markdown file (used to determine JSON path)
 * 
 * This function:
 * 1. Extracts the filename from the markdown path
 * 2. Determines the appropriate JSON file location (based on parent folder)
 * 3. Reads any existing JSON data or creates a new file
 * 4. Updates the JSON with the new data
 * 5. Writes the updated JSON back to disk
 */
void Store(cJSON *file_object, const char *Path) {
	if (file_object == NULL || Path == NULL) {
		fprintf(stderr, "Error: Invalid parameters for Store function\n");
		return;
	}

	// Extract the markdown filename without extension
	char Field[256];
	const char *MDfilename = strrchr(Path, '/');
	if (MDfilename == NULL) {
		fprintf(stderr, "Error: Invalid path format\n");
		return;
	}
	
	MDfilename++; // Skip past the slash
	size_t field_len = strlen(MDfilename);
	if (field_len >= sizeof(Field)) {
		fprintf(stderr, "Error: Filename too long\n");
		return;
	}
	
	// Copy filename and remove extension
	strncpy(Field, MDfilename, sizeof(Field) - 1);
	Field[sizeof(Field) - 1] = '\0';
	
	char *dot = strrchr(Field, '.');
	if (dot == NULL) {
		fprintf(stderr, "Error: No file extension found\n");
		return;
	}
	*dot = '\0';

	// Determine the JSON file path based on parent directory structure
	char json_path[PATH_MAX];
	char parent_folder[PATH_MAX];
	
	// Find the parent directory by counting back two directory levels
	size_t path_len = strlen(Path);
	size_t i;
	int slash_count = 0;
	for (i = path_len - 1; i > 0; i--) {
		if (Path[i] == '/') {
			slash_count++;
			if (slash_count == 2) {
				break;
			}
		}
	}
	
	// Handle different directory structures
	if (slash_count < 2) {
		// Simple case: not enough directory levels, store in same dir
		snprintf(json_path, sizeof(json_path), "%s.json", Field);
	} else {
		// Extract parent directory name
		strncpy(parent_folder, Path, i);
		parent_folder[i] = '\0';
		
		const char *folder_name = strrchr(parent_folder, '/');
		if (folder_name != NULL) {
			folder_name++; // Skip the slash
			
			// Construct JSON path in parent dir with parent's name
			char dir_path[PATH_MAX];
			strncpy(dir_path, Path, path_len - strlen(MDfilename));
			dir_path[path_len - strlen(MDfilename)] = '\0';
			
			snprintf(json_path, sizeof(json_path), "%s%s.json", dir_path, folder_name);
		} else {
			fprintf(stderr, "Error: Cannot determine folder name\n");
			return;
		}
	}

	// Read existing JSON file or create a new one
	cJSON *JSON_file_Object = NULL;
	
	// Check if file exists first
	if (access(json_path, F_OK) == 0) {
		JSON_file_Object = read_json_file(json_path);
		if (JSON_file_Object == NULL) {
			JSON_file_Object = cJSON_CreateObject();
		}
	} else {
		JSON_file_Object = cJSON_CreateObject();
	}

	// Update the JSON object with the new data
	cJSON *item = cJSON_GetObjectItem(JSON_file_Object, Field);
	if (!item) {
		// Add new field (using deep copy to avoid double-free issues)
		cJSON *copy = cJSON_Duplicate(file_object, 1);
		if (copy != NULL) {
			cJSON_AddItemToObject(JSON_file_Object, Field, copy);
		} else {
			fprintf(stderr, "Error: Failed to duplicate JSON object\n");
			cJSON_Delete(JSON_file_Object);
			return;
		}
	} else {
		// Replace existing field (using deep copy to avoid double-free issues)
		cJSON *copy = cJSON_Duplicate(file_object, 1);
		if (copy != NULL) {
			cJSON_ReplaceItemInObject(JSON_file_Object, Field, copy);
		} else {
			fprintf(stderr, "Error: Failed to duplicate JSON object\n");
			cJSON_Delete(JSON_file_Object);
			return;
		}
	}
	
	// Write the updated JSON back to disk
	FILE *JSON_file = fopen(json_path, "w");
	if (JSON_file == NULL) {
		fprintf(stderr, "Error: Cannot open file for writing: %s\n", json_path);
		cJSON_Delete(JSON_file_Object);
		return;
	}
	
	// Convert JSON to string for file storage
	char *json_string = cJSON_Print(JSON_file_Object);
	if (json_string == NULL) {
		fprintf(stderr, "Error: Failed to convert JSON to string\n");
		fclose(JSON_file);
		cJSON_Delete(JSON_file_Object);
		return;
	}
	
	fputs(json_string, JSON_file);
	fclose(JSON_file);
	
	// Free allocated resources
	free(json_string);
	cJSON_Delete(JSON_file_Object);
}

