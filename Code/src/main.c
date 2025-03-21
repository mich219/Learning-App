#include <stdlib.h>
#include <stdio.h>
#include "../include/Learning_Interface.h"
#include "../include/File_Processing.h"
#include "../include/cJSON.h"

/**
 * Learning App - A study and revision tool for notes stored in markdown format
 * 
 * This application allows users to:
 * 1. Process markdown notes into JSON structures for quick access
 * 2. Navigate through hierarchical learning content
 * 3. Review information in an organized way
 * 
 * The main.c file orchestrates the overall flow of the application,
 * connecting the file processing, directory navigation, and learning interface components.
 */

/**
 * Main program entry point
 * 
 * @param argc Number of command-line arguments
 * @param argv Array of command-line arguments (argv[1] should be the notes directory path)
 * @return 0 on success, non-zero on error
 * 
 * Program flow:
 * 1. Get and validate the notes directory path from command line
 * 2. Process all markdown files in the directory (if needed)
 * 3. Prompt user to select a field to study
 * 4. Load the selected field's JSON data
 * 5. Enter the interactive learning interface
 */
int main(int argc, char *argv[]){
	// Get the path to the notes directory from command line
	char *PATH; 
	if (argv[1] != NULL){
		PATH = argv[1];
	} else {
		fprintf(stderr, "Error: Path argument is required\n");
		return 1;
	}

	// Variables for field selection
	char Selected_Field[30];
	char *Selected_Field_Path = NULL;
	
	// Clear screen and display welcome message
	system("clear");
	printf("%s\n","Choose a Field That you would like to revise:");
	printf("%s\n","");

	// Process markdown files in the directory (updates only if needed)
	Build(PATH);

	// Field selection loop - continues until a valid field is selected
	while (Selected_Field_Path == NULL){
		printf("%s","Enter Field name:");
		scanf("%29s",Selected_Field);
		printf("%s\n","");
		
		// Find the JSON file path for the selected field
		Selected_Field_Path = Find_Field_Path(PATH,Selected_Field);
		// If not found, the loop will continue
	}
	
	// Clear screen and display usage instructions
	system("clear");
	printf("Press Enter to view the answer.\n");
	printf("Press _ to clear the screen.\n");
	printf("\n");
	getchar(); // Consume the lingering newline from scanf
	
	// Load the JSON data for the selected field
	cJSON *json_data = read_json_file(Selected_Field_Path);
	if (json_data == NULL) {
		fprintf(stderr, "Error: Failed to parse JSON file\n");
		return 1;
	}
	
	// Enter the interactive learning interface with the loaded data
	Learn(json_data);
	
	// Clean up: free the JSON data when done
	cJSON_Delete(json_data);
	
	return 0;
}
