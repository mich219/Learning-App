#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/cJSON.h"


cJSON *Extract(const char *Path){

	// Get the .md file name
	char Field[156];
	const char *MDfilename = strrchr(Path, '/'); // Find the last occurrence of '/'
	MDfilename++; // Move to the next character after '/'
	strncpy(Field, MDfilename, sizeof(Field)); // Copy the filename
	char *dot = strrchr(Field, '.'); // Find the last occurrence of '.'
	*dot = '\0'; // If '.' found, truncate the string

	FILE *file = fopen(Path, "r");
   char line[1024];
	cJSON *file_entry = cJSON_CreateObject();

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	cJSON *Last = file_entry; 
	cJSON *Type = NULL;  
	cJSON *Element = NULL; 
	cJSON *Concept = NULL; 
	cJSON *Awnser = NULL; 
	cJSON *Fact = NULL; 

	int truth_table[3] = {0,0,0};
	int Init = 0;

	while (fgets(line, sizeof(line), file)) {
		size_t len = strlen(line); 
		if (strncmp(line, "#",1) == 0) {
			line[len - 1] = '\0'; // Replace newline with null terminator
			if (Init){
				cJSON_AddItemToObject(Last,"Answer",Awnser);
				Init = 0;
			}	
			if (strncmp(line, "# ",2) == 0) {
				Type = cJSON_CreateObject();
				cJSON_AddItemToObject(file_entry,strdup(line + 2),Type);
				truth_table[0] = 1;
				Last = Type;
			} else if (strncmp(line, "## ", 3) == 0) {
				Element = cJSON_CreateObject();
				if (truth_table[0]) Last = Type;
				if (!truth_table[0]) Last = file_entry;
				cJSON_AddItemToObject(Last,strdup(line + 3),Element);
				truth_table[1] = 1;
				Last = Element;
			} else if (strncmp(line, "### ", 4) == 0) {
				Concept = cJSON_CreateObject();
				if (truth_table[1]) Last = Element;
				if (!truth_table[0] && !truth_table[1]) Last = file_entry;
				if (truth_table[0] && !truth_table[1]) Last = Type;
				cJSON_AddItemToObject(Last,strdup(line + 4),Concept);
				truth_table[2] = 1;
				Last = Concept;
			} else if (strncmp(line, "#### ", 5) == 0){ 
				Fact = cJSON_CreateObject();
				if (!truth_table[0] && !truth_table[1] && !truth_table[2]) Last = file_entry;
				if (truth_table[0] && !truth_table[1] && !truth_table[2]) Last = Type;
				if (truth_table[1] && !truth_table[2]) Last = Element;
				if (truth_table[2]) Last = Concept;
				cJSON_AddItemToObject(Last,strdup(line + 5),Fact);
				Last = Fact; 
			}
		} else {

			if (!Init){
				Awnser = cJSON_CreateArray();
				Init = 1;	
			}	
			cJSON_AddItemToArray(Awnser,cJSON_CreateString(line));
		}
		

	}
	cJSON_AddItemToObject(Last,"Answer",Awnser);
	Init = 0;
	fclose(file);
	return file_entry;	
}



cJSON* read_json_file(const char *filename) {
	FILE *file = fopen(filename, "r");
	cJSON *root;
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (size == 0){
		root = cJSON_CreateObject();
		return root;
	}

	char *buffer = (char *)malloc(size + 1);
	if (!buffer) {
	printf("%s\n","ERROROROROR");
    }
	fread(buffer, 1, size, file);
	buffer[size] = '\0'; // Null-terminate the string

	fclose(file);

	// Parse the JSON data
	root = cJSON_Parse(buffer);

	free(buffer);
	return root;
}

void Store(cJSON *file_object,const char *Path){

	// Get the .md file name -> Field
	char Field[156];
	const char *MDfilename = strrchr(Path, '/'); 
	MDfilename++;
	strncpy(Field, MDfilename, sizeof(Field));
	char *dot = strrchr(Field, '.');
	*dot = '\0';

	// create the .json file name from the parent folder of the .md file -> json_file_name 
	char *last_slash = strrchr(Path, '/');
	last_slash--;
	char *second_last_slash = strrchr(Path, '/');
	size_t parent_length = second_last_slash - Path;
	char parent_folder[256];
	strncpy(parent_folder, Path, parent_length);
	parent_folder[parent_length] = '\0';
	const char *Folder_name = strrchr(parent_folder, '/');
	Folder_name++;
	char json_file_name[256];
	snprintf(json_file_name, sizeof(json_file_name),"%s.json",Folder_name);

	// add the previous path to the new .json file name -> json_path	
	char json_path[256];
	strcpy(json_path,Path);
	char *filename = strrchr(json_path, '/');
	filename++;
	strcpy(filename,json_file_name);


	int used = 0;
	char *json_string;
	cJSON *JSON_file_Object;

	// Read the JSON file into the program
	FILE *JSON_file = fopen(json_path, "r");
	if(JSON_file){
		fclose(JSON_file);
		JSON_file_Object = read_json_file(json_path);
	}else{
		JSON_file_Object = cJSON_CreateObject();
	}
	// Edit the JSON file with the new data
	cJSON *item = cJSON_GetObjectItem(JSON_file_Object, Field);
	if (!item) {
		cJSON_AddItemToObject(JSON_file_Object, Field, file_object);
	} else {
		cJSON_ReplaceItemInObject(JSON_file_Object, Field, file_object);
	}
	JSON_file = fopen(json_path, "w");
	json_string = cJSON_Print(JSON_file_Object);
	fputs(json_string, JSON_file);
	fclose(JSON_file);
}


