#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "../include/File_Handling.h"
#include "../include/File_Processing.h"
#include "../include/Learning_Interface.h"
#include "../include/cJSON.h"

/**
 * Learning_Interface.c
 * 
 * This file provides the main user interface for the Learning App.
 * It handles the building of the knowledge base, finding specific fields,
 * and the interactive learning process through the JSON data structures.
 */

/**
 * Processes all markdown files in a directory (and its subdirectories)
 * and converts them to JSON if they need updating
 * 
 * @param Path The directory path to process
 * @return 0 on success, -1 on error
 * 
 * This function:
 * 1. Checks each markdown file's last access time against vault history
 * 2. Converts files that have been updated since last processing
 * 3. Recursively processes all subdirectories
 */
int Build(const char *Path){
	if (Path == NULL) {
		fprintf(stderr, "Error: NULL path provided to Build\n");
		return -1;
	}

	// Get vault history once at the start for efficiency
	char* Vault_history = Get_Vault_History(Path);
	char Recursive_PATH[PATH_MAX];
	cJSON *file_json;

	// Open the directory
	DIR *dir = opendir(Path);
	if (dir == NULL) {
		fprintf(stderr, "Error: Cannot open directory %s\n", Path);
		return -1;
	}

	// Process each entry in the directory
	struct dirent *Next_Folder;
	while ((Next_Folder = readdir(dir)) != NULL){
		// Skip hidden files and directories
		if (Next_Folder->d_name[0] == '.') {
			continue;
		}
		
		// Create full path for current entry
		snprintf(Recursive_PATH, sizeof(Recursive_PATH), "%s/%s", Path, Next_Folder->d_name);
		
		// Process markdown files
		if (Next_Folder->d_type == DT_REG && (strstr(Next_Folder->d_name, ".md") != NULL)){
			// Only process files that need updating (modified since last check)
			if (compare_dates(Vault_history, last_access_date(Recursive_PATH)) < 1) {
				file_json = Extract(Recursive_PATH);
				if (file_json != NULL) {
					Store(file_json, Recursive_PATH);
					// Note: Store takes ownership of file_json and will free it
				}
			}
		}
		// Recursively process subdirectories
		else if (Next_Folder->d_type == DT_DIR){
			printf("%s\n", Next_Folder->d_name); // Display folder being processed
			Build(Recursive_PATH);
		}
	}
	
	closedir(dir);
	return 0;
}

/**
 * Searches recursively through directories to find a specific field's JSON file path
 * 
 * @param Path The base directory to start searching
 * @param Selected_Field The field name to search for
 * @return The full path to the field's JSON file, or NULL if not found
 * 
 * This function searches through directories to find a folder matching the field name,
 * then constructs the path to the corresponding JSON file.
 */
char* Find_Field_Path(const char *Path, const char *Selected_Field) {
    if (Path == NULL || Selected_Field == NULL) {
        return NULL;
    }
    
    // Use static buffer to persist result after function returns
    static char result_path[PATH_MAX];
    result_path[0] = '\0'; // Clear the buffer
    
    // Open the directory for searching
    DIR *dir = opendir(Path);
    if (dir == NULL) {
        fprintf(stderr, "Error opening directory: %s\n", Path);
        return NULL;
    }
    
    struct dirent *Next_Folder;
    char Recursive_PATH[PATH_MAX];

    // Check each entry in the directory
    while ((Next_Folder = readdir(dir)) != NULL) {
        if (Next_Folder->d_name[0] == '.') { // Skip hidden files/directories
            continue;
        }
        
        // Construct the full path for this entry
        snprintf(Recursive_PATH, sizeof(Recursive_PATH), "%s/%s", Path, Next_Folder->d_name);

        // Check if this is a directory
        if (Next_Folder->d_type == DT_DIR) {
            // If the directory name matches the field we're looking for
            if (strcmp(Next_Folder->d_name, Selected_Field) == 0) {
                // Construct the path to the JSON file (Field/Field.json)
                snprintf(result_path, sizeof(result_path), "%s/%s/%s.json", 
                         Path, Selected_Field, Selected_Field);
                closedir(dir);
                return result_path;
            }
            
            // Recursively search this subdirectory
            char *found = Find_Field_Path(Recursive_PATH, Selected_Field);
            if (found != NULL && found[0] != '\0') {
                closedir(dir);
                return found;
            }
        }
    }
    
    closedir(dir);
    return NULL; // Field not found
}

/**
 * Constants for path tracking in the Learn function
 */
#define MAX_PATH_LENGTH 1024    // Maximum length of the full hierarchical path
#define MAX_PATH_SEGMENTS 50    // Maximum number of segments in a path
#define MAX_SEGMENT_LENGTH 100  // Maximum length of each segment

/**
 * Structure to keep track of the hierarchical path through JSON nodes
 * This is used to display the context for each answer
 */
typedef struct {
    char segments[MAX_PATH_SEGMENTS][MAX_SEGMENT_LENGTH]; // The path segments
    int levels[MAX_PATH_SEGMENTS];                        // The level of each segment
    int count;                                           // Number of segments in path
} PathContext;

// Global context for the current path
static PathContext pathContext;
static char fullPath[MAX_PATH_LENGTH] = {0};

/**
 * Handles user input during the learning process
 * 
 * Waits for the user to press Enter to continue, or _ to clear the screen.
 * This controls the pace of the learning session.
 */
static void readInputUntilEnter() {
    printf("\n");
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (ch == '_') {
            system("clear"); // Clear the console screen
            printf("\n:");
        }
    }
}

/**
 * Initialize or reset the path context
 * 
 * This is called at the start of the Learn function to ensure
 * we're starting with a clean path tracking state.
 */
static void resetPath() {
    pathContext.count = 0;
    fullPath[0] = '\0';
    
    // Clear all segment storage
    for (int i = 0; i < MAX_PATH_SEGMENTS; i++) {
        pathContext.segments[i][0] = '\0';
        pathContext.levels[i] = 0;
    }
}

/**
 * Builds the full path string from the current context
 * 
 * Combines all segments in the path context into a single
 * space-separated string for display purposes.
 */
static void buildFullPath() {
    fullPath[0] = '\0';
    
    // Concatenate all path segments with spaces between them
    for (int i = 0; i < pathContext.count; i++) {
        if (i > 0) {
            strncat(fullPath, " ", MAX_PATH_LENGTH - strlen(fullPath) - 1);
        }
        strncat(fullPath, pathContext.segments[i], MAX_PATH_LENGTH - strlen(fullPath) - 1);
    }
}

/**
 * Updates the path context when navigating to a new node
 * 
 * @param nodeName The name of the new node
 * @param level The hierarchical level of the node
 * 
 * This function handles the complex logic of updating the path context
 * when moving through the JSON hierarchy. It manages three cases:
 * 1. Going deeper in the hierarchy
 * 2. Moving to a sibling node
 * 3. Going back up the hierarchy to a higher level
 */
static void updatePath(const char* nodeName, int level) {
    if (nodeName == NULL) {
        return;
    }
    
    // Handle different hierarchy navigation scenarios
    if (pathContext.count > 0) {
        // Case 1: Going deeper in hierarchy - keep everything and add new node
        if (level > pathContext.levels[pathContext.count - 1]) {
            // No adjustment needed, just add the new node
        }
        // Case 2: Moving to sibling - replace the last node at same level
        else if (level == pathContext.levels[pathContext.count - 1]) {
            pathContext.count--; // Remove the last segment before adding the new one
        }
        // Case 3: Going back up in hierarchy - remove all deeper nodes
        else if (level < pathContext.levels[pathContext.count - 1]) {
            // Keep only nodes at or above the current level
            int newCount = 0;
            for (int i = 0; i < pathContext.count; i++) {
                if (pathContext.levels[i] < level) {
                    // Preserve nodes at higher levels in the hierarchy
                    if (i != newCount) {
                        strncpy(pathContext.segments[newCount], pathContext.segments[i], MAX_SEGMENT_LENGTH - 1);
                        pathContext.segments[newCount][MAX_SEGMENT_LENGTH - 1] = '\0';
                        pathContext.levels[newCount] = pathContext.levels[i];
                    }
                    newCount++;
                }
            }
            pathContext.count = newCount;
        }
    }
    
    // Add the new node to our path
    if (pathContext.count < MAX_PATH_SEGMENTS) {
        strncpy(pathContext.segments[pathContext.count], nodeName, MAX_SEGMENT_LENGTH - 1);
        pathContext.segments[pathContext.count][MAX_SEGMENT_LENGTH - 1] = '\0';
        pathContext.levels[pathContext.count] = level;
        pathContext.count++;
    }
    
    // Update the full path string
    buildFullPath();
}

/**
 * Displays an answer array with the current hierarchical path as context
 * 
 * @param answer The cJSON array containing the answer content
 * 
 * This function:
 * 1. Waits for user input to continue
 * 2. Displays the hierarchical path as context
 * 3. Displays the answer content
 */
static void displayAnswer(cJSON* answer) {
    if (!answer || !cJSON_IsArray(answer)) {
        return;
    }
    
    // Wait for user to press Enter to see the answer
    readInputUntilEnter();
    
    // Print the hierarchical path as context for the answer
    if (fullPath[0] != '\0') {
        printf("\033[1m%s:\033[0m\n", fullPath); // Bold formatting for path
    }
    
    // Print each answer item (usually lines of text)
    for (int i = 0; i < cJSON_GetArraySize(answer); i++) {
        cJSON* answerItem = cJSON_GetArrayItem(answer, i);
        if (answerItem && cJSON_IsString(answerItem)) {
            printf("%s", cJSON_GetStringValue(answerItem));
        }
    }
    printf("\n"); // End with a newline for spacing
}

/**
 * Processes a single JSON node during the learning session
 * 
 * @param node The cJSON node to process
 * @param level The hierarchical level of the node
 * 
 * This function:
 * 1. Updates the path context with the node's name
 * 2. Checks if this node has an answer to display
 */
static void processNode(cJSON* node, int level) {
    if (node == NULL) {
        return;
    }
    
    // Add the node's name to our path if it's a real content node
    // (ignore technical nodes like "Answer")
    if (node->string != NULL && strcmp(node->string, "Answer") != 0) {
        updatePath(node->string, level);
    }
    
    // Check if this is a leaf node with an answer to display
    cJSON* answer = cJSON_GetObjectItemCaseSensitive(node, "Answer");
    if (answer && cJSON_IsArray(answer)) {
        displayAnswer(answer);
    }
}

/**
 * Main interactive learning function
 * 
 * @param item The root cJSON object containing all learning data
 * 
 * This function navigates through the hierarchical JSON structure,
 * displaying content in a structured way that shows the context
 * of each piece of information. It uses a non-recursive approach
 * with a custom stack to prevent stack overflow with deep hierarchies.
 */
void Learn(cJSON *item) {
    if (item == NULL) {
        fprintf(stderr, "Error: NULL JSON item provided to Learn\n");
        return;
    }
    
    // Initialize path tracking
    resetPath();
    
    // Set up our custom stack for non-recursive traversal
    #define MAX_STACK_SIZE 100
    cJSON* nodeStack[MAX_STACK_SIZE];
    int levelStack[MAX_STACK_SIZE];
    int stackTop = -1;
    
    // Start with the root's children
    cJSON* current = item->child;
    int currentLevel = 0;
    
    // Traverse the entire JSON structure
    while (current != NULL || stackTop >= 0) {
        if (current != NULL) {
            // Process the current node
            processNode(current, currentLevel);
            
            // If the node has children, push current to stack and go down
            if (current->child && cJSON_IsObject(current->child)) {
                if (stackTop < MAX_STACK_SIZE - 1) {
                    stackTop++;
                    nodeStack[stackTop] = current;
                    levelStack[stackTop] = currentLevel;
                    current = current->child;
                    currentLevel++; // Increase level as we go deeper
                } else {
                    fprintf(stderr, "Error: JSON nesting too deep\n");
                    break;
                }
            } else {
                // No children, move to next sibling
                current = current->next;
            }
        } else {
            // No more nodes at this level, pop stack and go to next sibling
            current = nodeStack[stackTop]->next;
            currentLevel = levelStack[stackTop]; // Restore parent's level
            stackTop--;
        }
    }
}






