//Used for functions like input/output operations.
#include <stdio.h>
//Standard Library, used for memory allocation, exit handling, and other utility functions.
#include <stdlib.h>
//It is used for string related functions.
#include <string.h>
//Used for various constants, types, and functions related to system calls 
#include <unistd.h>
//Used for managing and reporting errors.
#include <errno.h>
//provides declarations and structures related to file and file system status. 
#include <sys/stat.h>
//Used for functions and constants used to manage and monitor child processes.
#include <sys/wait.h>
//Used for working with date and time-related functions.
#include <time.h>
//the header file related to directory handling.
#include <dirent.h>
//Socket Library, used for socket programming and creating/managing network sockets.
#include <sys/socket.h>
//Used for functions related to IP address manipulation and conversion.
#include <arpa/inet.h>
//Used for defining structures and constants related to network protocols
#include <netinet/in.h>
//The program will use port number 8000 for network communication.
#define PORT 8000

#define MIRROR_PORT 9000
//The IP address 127.0.0.1 of the server this program will connect to. 
#define MIRROR_SERVER_IP_ADDR "127.0.0.1"
//Maximum length of an IP address.
#define IP_LENGTH 16
//Mximum length of a port number.
#define PORT_LENGTH 6
//It is used for recieving message for successful connection. 
#define CONN_SUCCESS "success"

#define MAX_CLIENTS 6
//Used for sending and receiving data between the client and the server.
#define BUFFER_SIZE 1024
//Name of tar file that is used for program.
#define TAR_FILE_NAME "server_temp.tar.gz"
//It will handle maximum 6 files.
#define MAX_FILES 6
//Mximum length of File. 
#define MAX_FILENAME_LEN 50
//This command is used to search specific file.
#define FIND_FILE "filesrch"
//This command used to get temp.tar.gz that contains all file whose size is between specific bytes from the server.
#define S_GET_FILES "tarfgetz"
//This command used to get temp.tar.gz that contains all file whose date of creation is between specific bytes from the server.
#define D_GET_FILES "getdirf"
//Used to get files 
#define GET_FILES "fgets"
//The server must return temp.tar.gz that contains all the files in its directory belongigs to its extension.
#define GET_TAR_GZ "targzf"
//The command is transferred to the server and the client process is terminated 
#define QUIT "quit"

//to find the file in the server from /home/parikh34
char* findfile(char* filename) {
    // to store the final formatted string that will be returned.
    char str_appended[BUFFER_SIZE];
    // to store the command that will be passed
    char command[BUFFER_SIZE];
    // to search for the given filename
    sprintf(command, "find ~/ -name %s -printf '%%f|%%s|%%T@\\n' | head -n 1", filename);
    // opens a pipe to read the output.
    FILE* fp = popen(command, "r");
    char path[BUFFER_SIZE];
    //If it read a line then,
    if (fgets(path, BUFFER_SIZE, fp) != NULL) {
        //then print requestred file found.
        printf("Requested File Found.\n");
        path[strcspn(path, "\n")] = 0;
        // It extracts the filename, size, and date information from the path.
        char* filename_ptr = strtok(path, "|");
        char* size_ptr = strtok(NULL, "|");
        char* date_ptr = strtok(NULL, "|");
        //Converts the extracted size to integers.
        int size = atoi(size_ptr);
        //Converts the extracted date strings to integers.
        time_t date = atoi(date_ptr);
        //used to store the string containing the file name.
        char print_filename[BUFFER_SIZE];
        strcpy(print_filename, "File Name: ");//Copies the string File Name  to the print_filename buffer.
        strcat(print_filename, filename_ptr);//appends the contents of the filename_ptr to the print_filename
        strcat(print_filename, "\n"); 
        
        char print_size[BUFFER_SIZE];
        strcpy(print_size, "File Size: ");
        strcat(print_size, size_ptr); // appends the size_ptr to print_size.
        strcat(print_size, "\n");
        
        char print_created[BUFFER_SIZE];
        strcpy(print_created, "File Created At: ");
        strcat(print_created, ctime(&date));//appends the date to print_created
        strcat(print_created, "\n");
       
       // Copies the contents of the print_filename buffer into the str_appended buffer.
        strcpy(str_appended, print_filename);
        // Appends the contents of the print_size buffer to the end of the str_appended buffer. 
        strcat(str_appended, print_size);
        //appends the contents of the print_created buffer to the end of the str_appended buffer. 
        strcat(str_appended, print_created);
    }//If it does not able to read line then, 
    else {//file not found
        printf("File not Found.\n");
        strcpy(str_appended, "File not Found.\n");
    }
    //pipe will be closed.
    pclose(fp);
    char *ptr_client_str = str_appended;
    //return the string containg information.0
    return ptr_client_str;
}
 //send_tar_file function used for sending a TAR file over a network socket to a client. 
void send_tar_file(int socket_fd) {
    int break_flag = 0;
    //In binary read mode tar_file_name is opened.
    FILE* fp = fopen(TAR_FILE_NAME, "rb");
    // If the file cannot be opened,
    if (!fp) {
        //an error message is printed.
        perror("Error opening tar file");
        return;
    }
    //The function determines the size of the TAR file by seeking to the end of the file 
    fseek(fp, 0, SEEK_END);
    long tar_file_size = ftell(fp);
    //and then back to the beginning.
    fseek(fp, 0, SEEK_SET);
    char size_buffer[BUFFER_SIZE];
    // converts the file size to a string using sprintf
    sprintf(size_buffer, "%ld", tar_file_size);
    //checking whether the send function successfully sent the file size information to the client or not.
    if (send(socket_fd, size_buffer, strlen(size_buffer), 0) != strlen(size_buffer)) {
        //condition is not true then error msg is printed.
        perror("An error sending tar file size to the client");
        // close the TAR file 
        fclose(fp);
        return;
    }
    //checking whether the recv function successfully received an acknowledgment message from the client  
    if (recv(socket_fd, size_buffer, BUFFER_SIZE, 0) == -1) {
         //condition is not true then error msg is printed.
        perror("An error sending tar file size to client");
        // close the TAR file 
        fclose(fp);
        return;
    }
    printf("Client Acknowledged.\n");
    printf("File of size %ld is being sent.\n", tar_file_size);
    //to send contents of tar file to client.
    char buffer[BUFFER_SIZE];
    size_t n;
    //uses a loop to read data from the TAR file into the buffer
    while ((n = fread(buffer, sizeof(char), BUFFER_SIZE, fp)) > 0) {
        //data is sent to the client using the send function.
        if (send(socket_fd, buffer, n, 0) != n) {
            //If there's an error sending the data, throws an error.
            perror("An error sending tar file contents to client");
            //a flag break_flag is set to 1.
            break_flag = 1;
            //the loop breaks.
            break;
        }
    }
    //If break_flag is set, an error message is printed
    if (break_flag) {
        //the file could not be sent.
        printf("Unable to send tar.gz file to client\n");
    }
    //If break_flag is not set, a success message is printed. 
    else {
        //the file was sent successfully.
        printf("File sent successfully\n");
    }
    // close the TAR file using
    fclose(fp);
}
//find and compress files within a specific size range from the user's home directory 
void sgetfiles(int socket_fd, int size1, int size2) {
    char command[BUFFER_SIZE];
    //constructs a shell command using the sprintf function. 
    sprintf(command, "find ~/ -type f -size +%d -size -%d -print0 | tar -czvf %s --null -T -", size1, size2, TAR_FILE_NAME);
    //open in read mode.
    FILE* fp = popen(command, "r");
    //pauses for 20 sec.
    sleep(20);
    // send the compressed TAR file to the client through the specified socket.
    send_tar_file(socket_fd);
}
//find and compress files modified within a specific date range from the user's home directory 
void dgetfiles(int socket_fd, char* date1, char* date2) {
    char command[BUFFER_SIZE];
    //constructs a shell command using the sprintf function. 
    sprintf(command, "find ~/ -type f -newermt \"%s\" ! -newermt \"%s\" -print0 | tar -czvf %s --null -T -", date1, date2, TAR_FILE_NAME);
    FILE* fp = popen(command, "r");
    //pauses for 30 sec.
    sleep(30);
    // send the compressed TAR file to the client through the specified socket.
    send_tar_file(socket_fd);
}
//searches for a specific filename within a given directory 
int find_files(const char *dir_name, const char *filename, char *tar_file) {
    int found = 0;
    DIR *dir;
    struct dirent *entry;
    struct stat file_info;
    char path[PATH_MAX];
   // open the specified directory using opendir.
    if ((dir = opendir(dir_name)) == NULL) {
        //if cannot be opened then, throws an error.
        perror("opendir");
        return 0;
    }
    // continues through the entries in the directory using readdir
    while ((entry = readdir(dir)) != NULL) {
        //It skips the . and .. entries commonly found in directories.
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        //For each entry, it constructs the full path using snprintf
        snprintf(path, PATH_MAX, "%s/%s", dir_name, entry->d_name);
        //checks whether it is a directory or a regular file using lstat
        if (lstat(path, &file_info) < 0) {
            perror("lstat");
            continue;
        }
        //If the entry is a directory, the function recursively calls itself on that directory
        if (S_ISDIR(file_info.st_mode)) {
            //searching subdirectories.
            find_files(path, filename, tar_file);
        }
        //If the entry is a regular file with the desired filename
         else if (S_ISREG(file_info.st_mode)) {
            if (strcmp(entry->d_name, filename) == 0) {
                //the path of the file is appended to the TAR file string.
                strncat(tar_file, " ", BUFFER_SIZE - strlen(tar_file) - 1);
                strncat(tar_file, path, BUFFER_SIZE - strlen(tar_file) - 1);
                printf("File found at: %s\n", path);
                //used to keep track of whether any files matching the filename were found.
                found = 1;
            }
        }
    }
    //the directory is closed.
    closedir(dir);
    //indicates whether any files were found with the desired filename.
    return found;
}
 //search for a list of specified filenames within the user's home directory.
char* getfiles(int socket_fd, char files[MAX_FILES][MAX_FILENAME_LEN], int num_files) {
    //retrieves the user's home directory path
    char *dir_path = getenv("HOME");
    //If it cannot get the path.
    if (dir_path == NULL) {
        //error message for not getting home directory. 
        fprintf(stderr, "An error getting HOME directory path\n");
        return NULL;
    }
    // constructs a TAR command using the filename specified in TAR_FILE_NAME 
    char tar_cmd[BUFFER_SIZE] = "tar -czvf ";
    strcat(tar_cmd, TAR_FILE_NAME);
    int file_found = 0;
    //loop continues through the list of filenames provided in files
    for (int i = 0; i < num_files; i++) {
        printf("Finding Filename: %s\n", files[i]);
        //For each filename, it constructs the full path.
        char file_path[BUFFER_SIZE];
        snprintf(file_path, BUFFER_SIZE, "%s/%s", dir_path, files[i]);
        const char *homedir = getenv("HOME");
        //if home directory is null then,
        if (homedir == NULL) {
            //displays an error stat that home dir is not found.
            printf("Could not get HOME directory\n");
            return 0;
        }
        char path[PATH_MAX];
        snprintf(path, PATH_MAX, "%s/%s", homedir, files[i]);
        //uses the find_files function to search for the file within the home directory 
        file_found += find_files(homedir, files[i], tar_cmd);
    }
    //If any files are found, 
    if (file_found) {
        printf("File(s) found\n");
        //creates a TAR archive using the tar_cmd
        system(tar_cmd);
        //created TAR file is opened
        FILE *tar_file = fopen(TAR_FILE_NAME, "r");
        //ensure it is valid
        if (tar_file == NULL) {
            //if not then prints an error
            fprintf(stderr, "Error opening tar file\n");
            return NULL;
        }
        //tar_file is closed.
        fclose(tar_file);
        //delay before calling the send_tar_file function 
        sleep(20);
        //send the TAR archive to the client.
        send_tar_file(socket_fd);
    }
    //If no files are found,
     else {
        printf("No file found.\n");
        // sends a message 0 to the client indicate that no file was found.
        if (send(socket_fd, "0", strlen("0"), 0) != strlen("0")) {
            //there's an error sending the message
            perror("Error sending tar file size to client");
            return NULL;
        }
        //no file is found.
        return "No file found.";
    }
    return NULL;
}
// used to recursively search through a directory and its subdirectories to find files with specific extensions.
void find_gettargz_files(const char *dir_path, char extensions[MAX_FILES][MAX_FILENAME_LEN], int num_extensions, FILE *temp_list) {
    //open the directory specified by dir_path.
    DIR *dir = opendir(dir_path);
    //If the directory cannot be opened
    if (!dir) {
        //directory cannot be opnend.
        printf("Error: could not open directory %s\n", dir_path);
        return;
    }
    struct dirent *entry;
    //countinue through the entries in the directory using readdir.
    while ((entry = readdir(dir)) != NULL) {
        //for each entry,if it is a regular file then,
        if (entry->d_type == DT_REG) {
           
            char *name = entry->d_name;
            // compares its name against each of the provided extensions using a loop. 
            for (int i = 0; i < num_extensions; i++) {
                char *extension = extensions[i];
                int len_ext = strlen(extension);
                int len_name = strlen(name);
                //If a matching extension is found.
                if (len_name >= len_ext && strcmp(name + len_name - len_ext, extension) == 0) {
                    // file's full path is written to the temp_list
                    fprintf(temp_list, "%s/%s\n", dir_path, name);
                    break;
                }
            }
        } 
        //If the entry is a subdirectory
        else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            char subdir_path[BUFFER_SIZE];
            //the function recursively calls itself on the subdirectory path,
            snprintf(subdir_path, sizeof(subdir_path), "%s/%s", dir_path, entry->d_name);
            // searching subdirectories.
            find_gettargz_files(subdir_path, extensions, num_extensions, temp_list);
        }
    }
    // the directory is closed
    closedir(dir);
}
//search for files with specific extensions.
char* gettargz(int socket_fd, char extensions[MAX_FILES][MAX_FILENAME_LEN], int num_extensions) {
    // keep track of the number of files found.
    int file_found = 0;
    //opens a temporary file using tmpfile to store the paths of the found files.
    FILE *temp_list = tmpfile();
    //if there is no temporary file then,
    if (!temp_list) {
        //could not create temp file.
        printf("Error: could not create temporary file\n");
        return NULL;
    }
    // to search for files with the specified extensions
    find_gettargz_files(getenv("HOME"), extensions, num_extensions, temp_list);
    // writes their paths to the temporary file.
    rewind(temp_list);
    char filename[BUFFER_SIZE];
    // loops through the temporary file,
    while (fgets(filename, sizeof(filename), temp_list) != NULL) {
       //newline characters from the path are removed
        filename[strcspn(filename, "\n")] = 0;
        //increments the file_found 
        file_found++;
    }
    //f files are found
    if (file_found) {
        printf("Atleast 1 file found\n");
        
        rewind(temp_list);
        char command[BUFFER_SIZE] = "tar -czvf ";
        strcat(command, TAR_FILE_NAME);
        char filename[BUFFER_SIZE];
        // it constructs a tar command using the filenames stored in the temporary file
        while (fgets(filename, sizeof(filename), temp_list) != NULL) {
            filename[strcspn(filename, "\n")] = 0;
            //appending a space and a filename to the existing command.
            strcat(command, " ");
            strcat(command, filename);
        }
        //creates a TAR archive 
        int result = system(command);
        //pauses for 20 sec.
        sleep(20);
        // send the TAR archive to the client over the network socket.
        send_tar_file(socket_fd);
        //closes the temp_list.
        fclose(temp_list);
    } 
    //If no files are found
    else {
        printf("No file found.\n");
        //sends a message to the client indicating that no files were found 
        if (send(socket_fd, "0", strlen("0"), 0) != strlen("0")) {
            //If there's an error sending the message
            perror("Error sending tar file size to client");
            return NULL;
        }
        //The temporary file is closed 
        fclose(temp_list);
        return "No file found.";
    }
    return NULL;
}
//extract filenames from a given buffer.
void read_filenames(char* buffer, char filenames[MAX_FILES][MAX_FILENAME_LEN], int* num_files) {
    char* token;
    char delim[] = " ";
    //keep track of the number of filenames read.
    int i = 0;
    //separate the buffer using a space delimiter
    token = strtok(buffer, delim);
    token = strtok(NULL, delim);
   // loop continues through the remaining tokens.
    while (token != NULL && i < MAX_FILES) {
        //If a separate word is equal to "-u"
        if (strcmp(token, "-u") == 0) {
            
        }
        //loop continues until there are no more tokens 
        else {
            strncpy(filenames[i], token, MAX_FILENAME_LEN);
            i++;
        }
        token = strtok(NULL, delim);
    }
    //num_files counter is updated.
    *num_files = i;
}
//to handle incoming commands from a client over a TCP connection.
int processClient(int socket_fd) {
    //a pointer result to store the response to be sent to the client
    char *result;
    char buffer[BUFFER_SIZE];
    int n, fd;
    // an infinite loop to continuously read and process client commands.
    while(1) {
        result = NULL;
        bzero(buffer, BUFFER_SIZE);
        //to read data from the socket into the buffer.
        n = read(socket_fd, buffer, BUFFER_SIZE - 1);
        //If the read operation encounters an error
        if (n < 0) {
            perror("TCP Server-Read Error");
            return 1;
        }
        //if no data is read , the loop is exited.
        if (n == 0) {
            break;
        }
        //The received data is null-terminated to create a valid string.
        buffer[n] = '\0';
        printf("\n\n");
        printf("Command received: %s\n", buffer);
        printf("-----------------------------------------------------\n");
        printf("Processing the command...\n");
        // Process the command based on its type
        if (strncmp(buffer, FIND_FILE, strlen(FIND_FILE)) == 0) {
           // process FIND_FILE command
            char filename[BUFFER_SIZE];
            sscanf(buffer, "%*s %s", filename);
            printf("Filename: %s\n", filename);
            result = findfile(filename);
        }// executed when the received command matches the S_GET_FILES command. 
        else if (strncmp(buffer, S_GET_FILES, strlen(S_GET_FILES)) == 0) {
            int min_value, max_value;
            // used to extract the minimum and maximum values for file size from the buffer.
            sscanf(buffer, "%*s %d %d", &min_value, &max_value);
            // called with the extracted minimum and maximum values,
            sgetfiles(socket_fd, min_value, max_value);
            result = NULL;
            //used to skip the rest of the loop iteration
            continue;
        }// executed when the received command matches the D_GET_FILES command. 
         else if (strncmp(buffer, D_GET_FILES, strlen(D_GET_FILES)) == 0) {
            char min_date[BUFFER_SIZE];
            char max_date[BUFFER_SIZE];
             // used to extract the minimum and maximum dates from the buffer.
            sscanf(buffer, "%*s %s %s", min_date, max_date);
            // called with the extracted minimum and maximum dates.
            dgetfiles(socket_fd, min_date, max_date);
            result = NULL;
            //used to skip the rest of the loop iteration
            continue;
        }// when the received command matches the GET_FILES command, 
         else if (strncmp(buffer, GET_FILES, strlen(GET_FILES)) == 0) {
            char filenames[MAX_FILES][MAX_FILENAME_LEN];
            int num_files;
            //the filenames are read from the buffer
            read_filenames(buffer, filenames, &num_files);
            //called with the extracted filenames and the corresponding number of files
            result = getfiles(socket_fd, filenames, num_files);
            //If getfiles returns a non-NULL result
            if (result == NULL) {
                continue;
            }
        }//when the received command matches the GET_TAR_GZ command,
         else if (strncmp(buffer, GET_TAR_GZ, strlen(GET_TAR_GZ)) == 0) {
            char extensions[MAX_FILES][MAX_FILENAME_LEN];
            int num_extensions;
            //extensions are read from the buffer
            read_filenames(buffer, extensions, &num_extensions);
            //gettargz function is called with the extracted extensions and the corresponding number of extensions.
            result = gettargz(socket_fd, extensions, num_extensions);
            //If files were found and sent to the client successfully.
            if (result == NULL) {
                continue;
            }
        } //handles the QUIT command. 
        else if (strcmp(buffer, QUIT) == 0) {
           // If the received command is "QUIT," the server prints a message 
            printf("Client is quitting.\n");
            break;
        } //If none of the previous conditions match
        else {
            //result is set to the original command, indicating that the command is not recognized.
            result = buffer;
        }
        //to send the response back to the client.
        if (send(socket_fd, result, strlen(result), 0) != strlen(result)) {
            //If there's an error sending the response, it prints an error message,
            perror("TCP Server-Send Error");
            //closes the socket, and returns an error status.
            close(socket_fd);
            return 1;
        }
        // prints the response sent to the client 
        printf("Sent response to client: %s\n", result);
    }
    //Once the loop is exited, the socket is closed
    close(socket_fd);
    return 0;//indicates successful opereation.
}
//, the server handles incoming client connections, redirects clients to a mirror server 
int main(int argc, char const *argv[]) {
    //The server initializes variables such as num_of_clients and num_of_server_clients to keep track of connected clients.
    int num_of_clients = 0, num_of_server_clients = 0;
    
    int server_fd, client_fd;
   
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int optval = 1;
    pid_t childpid;
    //The server creates a socket using the socket function
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        //if there is an issue to establish connection, it will throw an error.
        perror("TCP Server-Socket Error");
        //terminate the program with a failure status.
        exit(EXIT_FAILURE);
    }
    //if there is an issue to establish connection between server-setsocket, it will throw an error.
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("TCP Server-setsockopt Error");
        exit(EXIT_FAILURE);
    }
    memset(&address, '0', sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
   //The server binds the socket to a specific address and port using the bind function.
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("TCP Server-Bind Error");
        exit(EXIT_FAILURE);
    }
    //The server sets the socket to listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS - 1) < 0) {
        perror("TCP Server-Listen Error");
        //terminate the program with a failure status.
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);
    //The server enters a loop that listens for incoming connections indefinitely.
    while (1) {
        //When a new client connection is accepted,
        client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        //the server checks whether the number of clients has reached a certain threshold.
        if ((num_of_clients >= MAX_CLIENTS && num_of_clients < 2*MAX_CLIENTS) || (num_of_clients >= 2*MAX_CLIENTS && num_of_clients % 2 != 0)) {
            //If the above conditions are met, the client is redirected to a mirror server.
            char mirror_port[PORT_LENGTH];
            sprintf(mirror_port, "%d", MIRROR_PORT);
            char mirror_address[IP_LENGTH + PORT_LENGTH + 1] = MIRROR_SERVER_IP_ADDR;
            strcat(mirror_address, ":");
            strcat(mirror_address, mirror_port);
            printf("Redirecting client to mirror server\n");
            //The mirror server's IP address and port are sent to the client using the send function.
            if (send(client_fd, mirror_address, strlen(mirror_address), 0) < 0) {
                perror("TCP Server-Mirror Address Send failed");
                exit(EXIT_FAILURE);
            }
            //The num_of_clients count is incremented.
            num_of_clients++;   
        } 
        //If the conditions are not met, the client is managed as a regular client
        else {
            //If accept fails to obtain a valid client socket descriptor.
            if (client_fd < 0) {
                //an error is printed, and the loop continues.
                perror("TCP Server-Accept Error");
                continue;
            }
            //A connection success acknowledgment is sent to the client
            if (send(client_fd, CONN_SUCCESS, strlen(CONN_SUCCESS), 0) < 0) {
                perror("TCP Server-Connection Acknowledgement Send failed");
                exit(EXIT_FAILURE);
            }
            //client is connected.
            printf("Client connected.------------\n");
            //A child process is created to handle the client's requests
            childpid = fork();
            //if cprocess id of child is less than 0
            if (childpid < 0) {
                //server fork error.
                perror("TCP Server-Fork Error");
                exit(EXIT_FAILURE);
            }
            if (childpid == 0) {
               //The server socket (server_fd) is closed in the child process 
                close(server_fd);
               //The processClient function is called to handle the client's requests
                int exit_status = processClient(client_fd);
                // the exit status determines the client disconnected with a success code.
                if (exit_status == 0) {
                    printf("Client Disconnected with Success Code-------------\n");
                    exit(EXIT_SUCCESS);
                } // the exit status determines the client disconnected with a error code.
                else {
                    printf("Client Disconnected with Error Code---------------\n");
                    exit(EXIT_FAILURE);
                }
            }
            //In the parent process
             else {
                //The number of connected clients is incremented, 
                num_of_clients++;
                //the number of server clients is also updated.
                num_of_server_clients++;
                //message indicating the current client count is printed.
                printf("The no of clients currently connected with server and mirror is: %d\n", num_of_clients);
                printf("The no of clients connected to the server is : %d\n", num_of_server_clients);
                //The client socket is closed.
                close(client_fd);
                //finished child processes to avoid zombie processes.
                while (waitpid(-1, NULL, WNOHANG) > 0);
            }
        }
    }
    //the server_fd socket is closed
    close(server_fd);
    //exiting successfully.
    return 0;
}