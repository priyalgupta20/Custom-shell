#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> //file operations and directory management
#include <sys/stat.h> //handle file metadata, retrieve or manipulate file attributes likesize
#include <fcntl.h>//file control options like opening files, creating or opening files, O_RDONLY(read only), O_WRONLY(write only)
#include <dirent.h>//directory handeling, listing files by readdir()
#include <errno.h>//handling errors in system calls or lib functs

#define RED "\033[1;31m"
#define GREEN "\033[1;32m"
#define YELLOW "\033[1;33m"
#define CYAN "\033[1;36m"
#define RESET "\033[0m"

// Function declarations
void shell_loop();
void execute_command(char **args);
char *read_command();
char **parse_command(char *input);
void cd_command(char *path);
void help_command();
void exit_command();
void delete_command();
void create_file_command();
void list_files_command();
void copy_file_command();
void move_file_command();
void rename_file_command();
void cpu_command();
void find_size_command();
void search_file();
void disk_usage();
void merge_files_command();
void create_from_text();

// Display the shell welcome message
void display_welcome()
{
    printf(GREEN "\n======================================================================\n" RESET);
    printf(YELLOW "                     Welcome to Our Custom Shell\n" RESET);
    printf(GREEN "======================================================================\n" RESET);
    printf(CYAN "Type 'help' to see available commands.\n" RESET);
}

// Function to read a command from the user
char *read_command() //read imput from user
{
    char *input = NULL; //input is a ptr
    size_t len = 0; //size of memory alloc
    if (getline(&input, &len, stdin) == -1)
    {
        if (feof(stdin)) //(end of file detection)check if end of input reached
        {
            printf(RED "End of input detected. Exiting...\n" RESET);
            exit(EXIT_SUCCESS);
        }
        else
        {
            perror(RED "Error reading input" RESET);
            exit(EXIT_FAILURE);
        }
    }
    return input;
}

// Function to parse the command into tokens
char **parse_command(char *input) //splits single array of input into array of tokens i.e. the commans name and arguments
{
    int bufsize = 64; //token array size default 64
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char *)); //dynamically alocates memory for tokens, each token is a string ptr
    char *token; //holds each token from input string 

    if (!tokens) //failure in memo alloc of tokens
    {
        fprintf(stderr, RED "Allocation error\n" RESET);
        exit(EXIT_FAILURE);
    }

    token = strtok(input, " \t\r\n\a"); //strtok: split string into tokens
    while (token != NULL)
    {
        tokens[position++] = token; //token extracted stored in tokens array

        if (position >= bufsize) //if no of tokens exceeds bufsize
        {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                fprintf(stderr, RED "Allocation error\n" RESET);
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL;
    return tokens;
}

//command to change directory
void cd_command(char *path) //path is a string that has directory we have to naviate to
{
    if (path == NULL)
    {
        fprintf(stderr, RED "cd: expected argument\n" RESET);
        return;
    }
    if (chdir(path) != 0) //system call to change directory
    {
        perror(RED "myshell: cd" RESET); //when directory doesnt exist
    }
}

void help_command()
{
    // Display a list of available commands
    printf(YELLOW "\n------------------------------------------------\n" RESET);
    printf(CYAN "        Our Custom Shell Commands:\n" RESET);
    printf(YELLOW "------------------------------------------------\n" RESET);
    printf(GREEN "cd <directory>" RESET " - Change directory\n");
    printf(GREEN "exit" RESET " - Exit the shell\n");
    printf(GREEN "copy <src> <dest>" RESET " - Copy files\n");
    printf(GREEN "move <src> <dest>" RESET " - Move files\n");
    printf(GREEN "rename <old> <new>" RESET " - Rename files\n");
    printf(GREEN "cpu" RESET " - Display CPU usage\n");
    printf(GREEN "delete <file>" RESET " - Delete a file\n");
    printf(GREEN "create <file/folder>" RESET " - Create a new file or folder\n");
    printf(GREEN "list" RESET " - List all files in the current directory\n");
    printf(GREEN "findsize <file>" RESET " - Find the size of a file\n");
    printf(GREEN "search <file>" RESET " - Find the path of a file\n");
    printf(GREEN "diskusage" RESET " - Display disk usage\n");
    printf(GREEN "merge" RESET " - Merge two files into one\n");
    printf(GREEN "create_from_text" RESET " - Create a file from user input text\n");
}

// Exit command
void exit_command()
{
    printf(YELLOW "Exiting the shell...\n" RESET);
    exit(EXIT_SUCCESS); //exit is stdlib function  to terminate a program
}

// Delete file with confirmation
void delete_command()
{
    char filename[256];
    char response;
    //getting file name in "filename" string
    printf(YELLOW "Enter the file name you want to delete: " RESET);
    scanf("%255s", filename);
    getchar();
    //getting confirmation in response
    printf(YELLOW "Are you sure you want to delete %s? (y/n): " RESET, filename);
    response = getchar();

    if (response == 'y' || response == 'Y')
    {
        if (unlink(filename) == 0) //unlink function call to delete 
        {
            printf(GREEN "File deleted successfully.\n" RESET);
        }
        else
        {
            perror(RED "myshell: delete" RESET);
        }
    }
    else
    {
        printf(CYAN "Deletion canceled\n" RESET);
    }
}

// Create file or folder command
void create_file_command()
{
    char choice[10];
    char name[256];

    printf(YELLOW "What do you want to create? (file/folder): " RESET);
    scanf("%9s", choice);
    getchar();

    if (strcmp(choice, "file") == 0)
    {
        printf(YELLOW "Enter the name of the file: " RESET);
        scanf("%255s", name);
        getchar();

        if (access(name, F_OK) != -1)
        {
            printf(RED "Error: File '%s' already exists!\n" RESET, name);
        }
        else
        {
            int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1)
            {
                perror(RED "myshell: create" RESET);
            }
            else
            {
                printf(GREEN "File created successfully: %s\n" RESET, name);
                close(fd);
            }
        }
    }
    else if (strcmp(choice, "folder") == 0)
    {
        printf(YELLOW "Enter the name of the folder: " RESET);
        scanf("%255s", name);
        getchar();

        if (mkdir(name) == 0)
        {
            printf(GREEN "Folder created successfully: %s\n" RESET, name);
        }
        else
        {
            perror(RED "myshell: create folder" RESET);
        }
    }
    else
    {
        printf(RED "Invalid choice! Please specify 'file' or 'folder'.\n" RESET);
    }
}

// List files in the current directory
void list_files_command()
{
    DIR *dir = opendir(".");
    struct dirent *entry;

    if (!dir)
    {
        perror(RED "myshell: list" RESET);
        return;
    }

    printf(CYAN "Files in the current directory:\n" RESET);
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            printf(GREEN "  %s\n" RESET, entry->d_name);
        }
        else if (entry->d_type == DT_DIR)
        {
            printf(YELLOW "  %s/\n" RESET, entry->d_name);
        }
    }
    closedir(dir);
}

// Copy file command
void copy_file_command()
{
    char src[256], dest[256];
    printf(YELLOW "Enter the source file path: " RESET);
    scanf("%255s", src);
    printf(YELLOW "Enter the destination file path: " RESET);
    scanf("%255s", dest);

    int src_fd = open(src, O_RDONLY);
    if (src_fd < 0)
    {
        perror(RED "myshell: copy - opening source file" RESET);
        return;
    }

    int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0)
    {
        perror(RED "myshell: copy - opening/creating destination file" RESET);
        close(src_fd);
        return;
    }

    char buffer[4096];
    ssize_t bytes;
    while ((bytes = read(src_fd, buffer, sizeof(buffer))) > 0)
    {
        if (write(dest_fd, buffer, bytes) != bytes)
        {
            perror(RED "myshell: copy - writing to destination file" RESET);
            close(src_fd);
            close(dest_fd);
            return;
        }
    }

    printf(GREEN "File copied successfully from %s to %s\n" RESET, src, dest);
    close(src_fd);
    close(dest_fd);
}

// Move file command
void move_file_command()
{
    char src[256], dest[256];
    printf(YELLOW "Enter the source file path: " RESET);
    scanf("%255s", src);
    printf(YELLOW "Enter the destination file path: " RESET);
    scanf("%255s", dest);

    if (rename(src, dest) == 0)
    {
        printf(GREEN "File moved successfully from %s to %s\n" RESET, src, dest);
    }
    else
    {
        perror(RED "myshell: move" RESET);
    }
}

// Rename file command
void rename_file_command()
{
    char old_name[256], new_name[256];
    printf(YELLOW "Enter the current file name: " RESET);
    scanf("%255s", old_name);
    printf(YELLOW "Enter the new file name: " RESET);
    scanf("%255s", new_name);

    if (rename(old_name, new_name) == 0)
    {
        printf(GREEN "File renamed successfully from %s to %s\n" RESET, old_name, new_name);
    }
    else
    {
        perror(RED "myshell: rename" RESET);
    }
}

// CPU command to display usage
void cpu_command() {
    char buffer[256];
    FILE *fp;

    // Check for Windows or Unix-like OS (Linux/macOS)
    #ifdef _WIN32
        // Windows command to get CPU usage using wmic
        fp = popen("wmic cpu get loadpercentage", "r");

        if (fp == NULL) {
            perror("Error executing command to get CPU usage");
            return;
        }

        // Read the output and display CPU usage
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            // Skipping the first line (header from wmic output)
            fgets(buffer, sizeof(buffer), fp);  // skip the next empty line or header
            printf("Current CPU Usage: %s", buffer); // Prints the CPU usage in percentage
        } else {
            printf("Unable to retrieve CPU usage.\n");
        }

        fclose(fp); // Close the file pointer (popen)

    #else
        // Unix/Linux/MacOS command to get CPU usage using top command
        fp = popen("top -bn1 | grep 'Cpu(s)' | sed 's/.*, *\\([0-9.]*\\)%* id.*/\\1/' | awk '{print 100 - $1\"%\"}'", "r");

        if (fp == NULL) {
            perror("Error executing command to get CPU usage");
            return;
        }

        // Read the output and display CPU usage
        if (fgets(buffer, sizeof(buffer), fp) != NULL) {
            printf("Current CPU Usage: %s", buffer); // Prints the CPU usage in percentage
        } else {
            printf("Unable to retrieve CPU usage.\n");
        }

        fclose(fp); // Close the file pointer (popen)
    #endif
}

// Find file size command
void find_size_command()
{
    char filename[256];
    printf(YELLOW "Enter the file name: " RESET);
    scanf("%255s", filename);

    struct stat st;
    if (stat(filename, &st) == 0)
    {
        printf(GREEN "Size of %s: %ld bytes\n" RESET, filename, st.st_size);
    }
    else
    {
        perror(RED "myshell: findsize" RESET);
    }
}

// Function to search for a file in the directory and subdirectories
void search_file(const char *filename)
{
    char current_dir[1024];
    DIR *dir;
    struct dirent *entry;

    if (getcwd(current_dir, sizeof(current_dir)) == NULL)
    {
        perror("getcwd() error");
        return;
    }

    dir = opendir(current_dir);
    if (dir == NULL)
    {
        perror("opendir() error");
        return;
    }

    int found = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, filename) == 0)
        {
            printf("File found: %s/%s\n", current_dir, entry->d_name);
            found = 1;
            break;
        }
    }

    if (!found)
    {
        printf("File not found\n");
    }

    closedir(dir);
}

// Function to create a file from user input text
void create_from_text()
{
    char filename[256];
    char text[1024];
    printf(YELLOW "Enter the filename: " RESET);
    scanf("%s", filename);
    getchar(); // To consume the newline character

    // Check if the file already exists
    if (access(filename, F_OK) != -1) // F_OK checks for file existence
    {
        printf(RED "Error: File '%s' already exists! Operation aborted to prevent overwriting.\n" RESET, filename);
        return;
    }

    // If file does not exist, proceed to create it
    printf(YELLOW "Enter the text to save in the file: (Press Enter and Ctrl+D to finish)\n" RESET);
    char *input = fgets(text, sizeof(text), stdin);
    if (input == NULL)
    {
        perror(RED "Error reading input" RESET);
        return;
    }

    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        perror(RED "myshell: create file" RESET);
        return;
    }

    fprintf(file, "%s", text);
    fclose(file);
    printf(GREEN "File '%s' created with the provided text.\n" RESET, filename);
}
void disk_usage()
{
    printf("\033[1;36mDisk Usage Information:\n\033[0m");

    // Check for Windows or Unix-like OS (Linux/macOS)
    #ifdef _WIN32
        // Windows command to get disk usage
        system("wmic logicaldisk get size,freespace,caption");
    #else
        // Unix/Linux/MacOS command to get disk usage
        system("df -h");
    #endif
}

// Merge two files into one output file
void merge_files_command()
{
    char file1[256], file2[256], output_file[256];
    printf(YELLOW "Enter the name of the first file: " RESET);
    scanf("%s", file1);
    printf(YELLOW "Enter the name of the second file: " RESET);
    scanf("%s", file2);
    printf(YELLOW "Enter the name of the output file: " RESET);
    scanf("%s", output_file);
    getchar();

    FILE *fp1 = fopen(file1, "r");
    if (!fp1)
    {
        perror(RED "myshell: merge - opening first file" RESET);
        return;
    }

    FILE *fp2 = fopen(file2, "r");
    if (!fp2)
    {
        perror(RED "myshell: merge - opening second file" RESET);
        fclose(fp1);
        return;
    }

    FILE *fp_out = fopen(output_file, "w");
    if (!fp_out)
    {
        perror(RED "myshell: merge - creating output file" RESET);
        fclose(fp1);
        fclose(fp2);
        return;
    }

    char buffer[4096];
    size_t bytes;

    // Copy content from first file
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp1)) > 0)
    {
        fwrite(buffer, 1, bytes, fp_out);
    }

    // Copy content from second file
    while ((bytes = fread(buffer, 1, sizeof(buffer), fp2)) > 0)
    {
        fwrite(buffer, 1, bytes, fp_out);
    }

    printf(GREEN "Files merged successfully into %s\n" RESET, output_file);

    fclose(fp1);
    fclose(fp2);
    fclose(fp_out);
}
// Shell loop to continuously prompt user for commands
void shell_loop()
{
    char *input;
    char **args;
    int keep_running = 1;

    while (keep_running)
    {
        printf(CYAN "myshell> " RESET); // Print the prompt
        input = read_command();         // Get the user input
        args = parse_command(input);    // Parse the command into arguments

        if (args[0] == NULL)
        { // If no command entered, continue the loop
            free(input);
            free(args);
            continue;
        }

        // Handle the commands
        if (strcmp(args[0], "exit") == 0)
        {
            exit_command();
        }
        else if (strcmp(args[0], "help") == 0)
        {
            help_command();
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            cd_command(args[1]);
        }
        else if (strcmp(args[0], "create") == 0)
        {
            create_file_command();
        }
        else if (strcmp(args[0], "delete") == 0)
        {
            delete_command();
        }
        else if (strcmp(args[0], "list") == 0)
        {
            list_files_command();
        }
        else if (strcmp(args[0], "copy") == 0)
        {
            copy_file_command();
        }
        else if (strcmp(args[0], "move") == 0)
        {
            move_file_command();
        }
        else if (strcmp(args[0], "rename") == 0)
        {
            rename_file_command();
        }
        else if (strcmp(args[0], "cpu") == 0)
        {
            cpu_command();
        }
        else if (strcmp(args[0], "findsize") == 0)
        {
            find_size_command();
        }
        else if (strcmp(args[0], "search") == 0)
        {
            if (args[1] != NULL)
            {
                search_file(args[1]);
            }
            else
            {
                printf(RED "Error: Filename argument missing for search.\n" RESET);
            }
        }
        else if (strcmp(args[0], "diskusage") == 0)
        {
            disk_usage();
        }
        else if (strcmp(args[0], "merge") == 0)
        {
            merge_files_command();
        }
        else if (strcmp(args[0], "create_from_text") == 0)
        {
            create_from_text();
        }
        else
        {
            printf(RED "myshell: command not found: %s\n" RESET, args[0]);
        }

        // Free allocated memory for input and arguments after processing
        free(input);
        free(args);
    }
}

// Main function
int main()
{
    display_welcome();
    shell_loop();
    return 0;
}
