#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/cJSON.h"



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





void Store(cJSON *file_object,const char *Path){
	// Get the folder name
	char *last_slash = strrchr(Path, '/');
	last_slash--;
	char *second_last_slash = strrchr(Path, '/');
	size_t parent_length = second_last_slash - Path;
	char parent_folder[256];
	strncpy(parent_folder, Path, parent_length);
	parent_folder[parent_length] = '\0';
	const char *Folder_name = strrchr(parent_folder, '/'); // Find the last occurrence of '/'
	Folder_name++;


	char json_file_name[256];
	snprintf(json_file_name, sizeof(json_file_name),"%s.json",Folder_name);

	char json_path[256];
	strcpy(json_path,Path);

	char *filename = strrchr(json_path, '/');
	filename++;
	strcpy(filename,json_file_name);

	cJSON *root;

	FILE *JSON_file = fopen(json_path, "r");
	if(JSON_file){
		fclose(JSON_file);
		root = read_json_file(json_path);
	}else{
		root = cJSON_CreateObject();
	}
}











cJSON *Extract(const char *Path){

	// Get the .md file name
	char Field[156];
	const char *MDfilename = strrchr(Path, '/'); // Find the last occurrence of '/'
	MDfilename++; // Move to the next character after '/'
	strncpy(Field, MDfilename, sizeof(Field)); // Copy the filename
	char *dot = strrchr(Field, '.'); // Find the last occurrence of '.'
	*dot = '\0'; // If '.' found, truncate the string



	cJSON *root = NULL; 

	root = cJSON_CreateObject();
	FILE *file = fopen(Path, "r");
   char line[1024];
	cJSON *file_entry = cJSON_CreateObject();
	cJSON_AddItemToObject(root,Field,file_entry);
	cJSON *Last;
	cJSON *Type; 
	cJSON *Element;
	cJSON *Concept;
	cJSON *Awnser = NULL;
//	cJSON *Fact;

	int truth_table[3] = {0,0,0};

	int Init = 0;

	while (fgets(line, sizeof(line), file)) {
		if (line == NULL) {break;}

		size_t len = strlen(line);
		int code_block_started = 0;
		
		if (strncmp(line, "#",1) == 0) {
			line[len - 1] = '\0'; // Replace newline with null terminator
			if (Init){
				cJSON_AddStringToObject(Last,"Familiarity","0");
				cJSON_AddStringToObject(Last,"Known","0");
				cJSON_AddStringToObject(Last,"Question","");
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
				if (truth_table[0] && truth_table[1]) Last = Element;
				if (!truth_table[0] && !truth_table[1]) Last = file_entry;
				if (truth_table[0] && !truth_table[1]) Last = Type;
				cJSON_AddItemToObject(Last,strdup(line + 4),Concept);
				truth_table[2] = 1;
				Last = Concept;
	/*		} else if (strncmp(line, "#### ", 5) == 0) 
				Fact = cJSON_CreateObject();
				cJSON_AddItemToObject(Last,strdup(line + 5),Fact);
				Last = Fact; */
			}
		} else {


			if (!Init){
				Awnser = cJSON_CreateObject();
				Init = 1;
			}
			if (strncmp(line, "- ", 2) == 0) {
				cJSON_AddStringToObject(Awnser,"Line_List", strdup(line + 2));
			} else if (strlen(line) >= 4 && isdigit(line[0]) && line[1] == '.' && line[2] == ' ') {
				cJSON_AddStringToObject(Awnser,"Numbered_List", strdup(line + 2));
			} else if (strncmp(line, "```", 3) == 0) {
				if (!code_block_started) {
					code_block_started = 1;
				} else {
					code_block_started = 0;
				}
			} else if (code_block_started) {
				cJSON_AddStringToObject(Awnser,"Code_Block", line);
			} else {
				line[len - 1] = '\0'; // Replace newline with null terminator
				cJSON_AddStringToObject(Awnser,"Text", line);
			}
		}
	}
	free(json_string);
	fclose(file);
	return root;	
}

