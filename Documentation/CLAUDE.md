# CLAUDE.md - Development Guide for Learning App

## Build Commands
```bash
# Compile the Learning App
gcc -o Learn Learning_App/Code/src/*.c -I Learning_App/Code/include

# Run the application (with notes directory as parameter)
./Learn Notes

# Single test/debug run
gcc -g -o Learn Learning_App/Code/src/*.c -I Learning_App/Code/include && ./Learn Notes
```

## Code Style Guidelines
- **Naming Conventions**:
  - Functions: PascalCase (e.g., `Build()`, `Learn()`, `Find_Field_Path()`)
  - Variables: snake_case (e.g., `selected_field`, `file_path`)
  - Constants: UPPERCASE (e.g., `PATH`)
  
- **Error Handling**: Return null/error codes for failures, check return values

- **File Organization**:
  - Notes must be in Markdown (.md) format
  - Note structure: Field folders → Subject files
  - Syntax: Use Markdown headings (#, ##, ###, ####)

- **Memory Management**: Allocate with malloc/calloc, always free when done

## Function Reference

### File_Handling.c
- `Update_Vault_History`: Writes current date to a tracking file
- `Get_Vault_History`: Retrieves last update date for a specific folder
- `last_access_date`: Returns formatted date string of file's last access time
- `compare_dates`: Compares two date strings (returns -1, 0, or 1)

### File_Processing.c
- `Extract`: Converts markdown file to cJSON structure based on heading hierarchy
- `read_json_file`: Loads and parses JSON file into cJSON structure
- `Store`: Saves processed markdown data to corresponding JSON file

### Learning_Interface.c
- `Build`: Recursively processes all markdown files in path that need updating
- `Find_Field_Path`: Locates JSON file path for a specified field
- `Learn`: Interactive function to display learning content from JSON structure
- `readInputUntilEnter`: Helper function for Learn to manage user input

### main.c
- `main`: Program entry point that handles parameter parsing and orchestrates app flow

1. Fixed memory management issues in File_Handling.c (removed memory leaks)
2. Enhanced error handling in File_Processing.c for better robustness
3. Improved the JSON processing to prevent double-free issues with duplicate objects
4. Restructured the Learn function to use a more efficient approach
5. Updated path handling for Notes_History files

## Learning Interface Hierarchical Path Improvements (3/21/2025)

Enhanced the Learning_Interface.c file to better display hierarchical node paths:

### Changes to updatePath():
```c
// If we're going back up or changing to a different branch in the hierarchy
if (pathContext.count > 0) {
    // Case 1: Going deeper in hierarchy - keep everything
    if (level > pathContext.levels[pathContext.count - 1]) {
        // No adjustment needed, just add the new node
    }
    // Case 2: Moving to sibling - remove the last node at same level
    else if (level == pathContext.levels[pathContext.count - 1]) {
        pathContext.count--; // Remove the last segment before adding the new one
    }
    // Case 3: Going back up in hierarchy - remove all deeper nodes
    else if (level < pathContext.levels[pathContext.count - 1]) {
        // Keep only nodes at or above the current level
        int newCount = 0;
        for (int i = 0; i < pathContext.count; i++) {
            if (pathContext.levels[i] < level) {
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
```

### Improvements to processNode():
- Only add meaningful node names to the path
- Ignore "Answer" pseudo-nodes in the hierarchical path
- Better handling of node level transitions

### displayAnswer() enhancements:
- Added bold formatting for the hierarchical path
- Improved visual distinction between path and answer content

These changes ensure that when traversing the JSON structure:
1. The hierarchical path accurately shows parent-child relationships
2. When moving between branches, the path resets appropriately
3. The full context is displayed with each answer

## Code Documentation Improvements (3/21/2025)

Comprehensive documentation was added throughout the codebase to improve maintainability:

### File_Handling.c:
- Added file overview explaining date operations and history tracking
- Documented each function's purpose, parameters, and return values
- Clarified the vault history storage mechanism
- Added inline comments for complex date handling logic

### File_Processing.c:
- Added detailed explanation of the markdown to JSON conversion process
- Documented the hierarchical heading structure (#, ##, ###, ####)
- Improved error handling documentation
- Clarified the process of determining JSON output paths
- Added memory management explanations for avoiding double-free issues

### Learning_Interface.c:
- Documented the hierarchical path tracking system
- Added detailed explanation of the non-recursive JSON traversal algorithm
- Clarified the three navigation cases (deeper, sibling, parent)
- Improved documentation of the interactive learning process
- Added comments explaining path context management

### main.c:
- Added application overview and program flow documentation
- Clarified command-line argument handling
- Documented the interactive user interface elements
- Added explanations for JSON data loading and cleanup

These documentation improvements enhance code maintainability by:
1. Making complex algorithms easier to understand
2. Clarifying the relationships between different components
3. Explaining the purpose and logic of each function
4. Documenting potential edge cases and error handling
5. Providing context for future development and improvements

## Performance Optimization Recommendations (3/21/2025)

After analyzing the codebase, here are recommended performance and efficiency improvements:

### File I/O Optimizations

1. **Reduce File Operations**:
   - Eliminate redundant file reads/writes in Get_Vault_History()
   - Replace multiple open/close operations with a single session
   - Cache file existence checks
   - Add filesystem notification support to detect changes instead of checking timestamps

2. **Lazy Loading**:
   - Implement incremental JSON loading (load only when needed)
   - Add pagination for large content
   - Stream file parsing instead of loading entire files into memory

### Memory Management

1. **Buffer Optimization**:
   - Replace fixed-size buffers with dynamic allocation
   - Use string views instead of copying strings where possible
   - Implement memory pooling for frequently allocated objects
   - Add buffer size checks to prevent overflows

2. **JSON Structure Improvements**:
   - Minimize deep copying of cJSON objects
   - Implement reference counting for shared JSON structures
   - Use a more efficient JSON library (e.g., simdjson, rapidjson)
   - Add custom allocators for JSON objects

### Algorithm Efficiency

1. **Search Improvements**:
   - Replace directory traversal with indexed lookups
   - Implement hash tables for field and node lookups
   - Replace string comparisons with integer key comparisons
   - Optimize the Extract() function with a state machine parser

2. **Path Management**:
   - Implement a more efficient path tracking data structure
   - Use a rope or tree structure for hierarchical paths
   - Optimize updatePath() to avoid O(n²) complexity
   - Pre-allocate path segments to avoid frequent reallocations

### Caching Strategies

1. **Content Caching**:
   - Add LRU cache for frequently accessed fields
   - Implement result caching for Find_Field_Path()
   - Cache directory structure information
   - Add persistence for caches between sessions

2. **Metadata Caching**:
   - Cache file stats and modification times
   - Implement a metadata database instead of individual files
   - Use bloom filters for quick existence checks
   - Add a journal to track changes between sessions

### Architectural Improvements

1. **Structural Changes**:
   - Separate UI logic from data processing
   - Implement a proper MVC architecture
   - Add a dedicated indexing component
   - Consider a plugin architecture for extensibility

2. **Storage Improvements**:
   - Replace text-based Notes_History with a SQLite database
   - Implement a proper search index (e.g., inverted index)
   - Consider a binary format for JSON storage
   - Add compression for large content

3. **Concurrency**:
   - Process files in parallel during Build()
   - Add worker threads for file I/O operations
   - Implement thread-safe caches
   - Use non-blocking I/O where appropriate

4. **UI Improvements**:
   - Replace system() calls with ANSI escape codes or ncurses
   - Implement incremental display for large content
   - Add asynchronous loading indicators
   - Consider a more responsive TUI framework

### Drastic Architectural Changes

For significant performance gains, consider these more radical changes:

1. **Complete Rewrite with Optimization Focus**:
   - Replace file-based storage with a database (SQLite, LMDB)
   - Implement a custom binary format instead of JSON
   - Design a custom memory-mapped storage engine
   - Use modern C++ or Rust for better memory safety and performance

2. **Distributed Architecture**:
   - Split into client/server architecture for multi-user support
   - Implement separate indexing and content servers
   - Add a dedicated caching layer
   - Design for horizontal scaling

These improvements would require significant refactoring but could provide orders of magnitude better performance for large knowledge bases.

