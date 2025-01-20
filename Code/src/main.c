#include <stdlib.h>
#include <stdio.h>
#include "../include/Learning_Interface.h"
#include "../include/File_Processing.h"
#include "../include/cJSON.h"


int main(int argc, char *argv[]){

	char *PATH; 
	if (argv[1] != NULL){
		PATH = argv[1];
	}

	char Selected_Field[30];
	char *Selected_Field_Path = NULL;
	// styling
	system("clear");
	printf("%s\n","Choose a Field That you would like to revise:");
	printf("%s\n","");

	// Function Call
	Build(PATH);

	// styling
	while (Selected_Field_Path == NULL){
		printf("%s","Enter Field name:");
		scanf("%29s",Selected_Field);
		printf("%s\n","");
		// Function Call
		Selected_Field_Path = Find_Field_Path(PATH,Selected_Field);
	}
	
	system("clear");
	printf("Press Enter to view the answer.\n");
	printf("Press _ to clear the screen.\n");
	printf("\n");
	getchar();
	read_json_file(Selected_Field_Path);
	Learn(read_json_file(Selected_Field_Path));

	return 0;
}
