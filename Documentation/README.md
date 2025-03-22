Welcome to my Learning App.

You must first open the terminal in the Learning_App/Code directory.

To compile the Learning App:
gcc -o Learn Learning_App/Code/src/*.c -I Learning_App/Code/include

To run the Learning App: (with Notes being the directory to your markdown notes)
./Learn ~/Notes


You are required to write you notes in the .md format.

You must try to implement the following naming conventions:
- Folder = Field (this should be the overarching field of the subject eg "Cryptography")
- File name = Subject (this should relate to a specific Subject of the Field eg "AES.md")

The following syntax is relevent (the program will store the titles and the corresponding text underneath):
- #
- ##
- ###
- ####

when the question is generated it will appear as a line with the following parameters:

- #, ##, ###, ####


