#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include "../include/File_Handling.h"
#include "../include/File_Processing.h"
#include "../include/cJSON.h"


//"/media/michael/NEW VOLUME/Notes"
#define Path "/home/michael/Desktop/test"

int main(){
	int Init = 0;
	char* Vault_history = NULL;
	char Recursive_PATH[PATH_MAX];
	if (!Init){
		Vault_history = Get_Vault_History(Path);
		Init = 1;
	}
	DIR *dir = opendir(Path);
	struct dirent *Next_Folder;
	while ((Next_Folder = readdir(dir)) != NULL){
	
		if (Next_Folder->d_name[0] == '.') {continue;}
		snprintf(Recursive_PATH, sizeof(Recursive_PATH), "%s/%s", Path, Next_Folder->d_name);
		if (Next_Folder -> d_type == DT_REG){
			if (compare_dates(Vault_history,last_access_date(Recursive_PATH)) < 1)
				Extract(Recursive_PATH);
		}
		if (Next_Folder->d_type == DT_DIR){
			main(Recursive_PATH);
		}
	}
	closedir(dir);

	return 0;
}

