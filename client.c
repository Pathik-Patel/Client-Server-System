//It is used to enable certain functions or features.
#define _XOPEN_SOURCE
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
//Used for working with date and time-related functions.
#include <time.h>
//Socket Library, used for socket programming and creating/managing network sockets.
#include <sys/socket.h>
//Used for functions related to IP address manipulation and conversion.
#include <arpa/inet.h>
//Used for defining structures and constants related to network protocols
#include <netinet/in.h>

//The program will use port number 8000 for network communication.
#define PORT 8000
//The IP address 127.0.0.1 of the server this program will connect to. 
#define SERVER_IP_ADDR "127.0.0.1"\
//Maximum length of an IP address.
#define IP_LENGTH 16\
//Mximum length of a port number.
#define PORT_LENGTH 6
//It is used for recieving message for successful connection.
#define CONN_SUCCESS "success"
//Used for sending and receiving data between the client and the server.
#define BUFFER_SIZE 1024
//Name of tar file that is used for program.
#define TAR_FILE_NAME "temp.tar.gz"
//It will handle maximum 6 files.
#define MAX_FILES 6
//Mximum length of File.
#define MAX_FILENAME_LEN 50
//This command is used to search specific file.
#define FIND_FILE "findfile"
//This command used to get temp.tar.gz that contains all file whose size is between specific bytes from the server.
#define S_GET_FILES "sgetfiles"
//This command used to get temp.tar.gz that contains all file whose date of creation is between specific bytes from the server.
#define D_GET_FILES "dgetfiles"
//Used to get files 
#define GET_FILES "getfiles"
//The server must return temp.tar.gz that contains all the files in its directory belongigs to its extension.
#define GET_TAR_GZ "gettargz"
//The command is transferred to the server and the client process is terminated 
#define QUIT "quit"

//The function receive_files that receives files from a server over a network connection, processes them, and saves them locally
int receive_files(int socket_fd) {
    FILE* fp = fopen(TAR_FILE_NAME, "wb");	// open a tar file named TAR_FILE_NAME in write-binary mode
    if (!fp) {
        //If there was an issue in creating a tar file an error msg is displayed.
        perror("There is an issue in creating tar file.");
        //It indicates failure.
        return 1;
    }

    //Then function is move forward to receive the size of the tar file from the server.
    char size_buffer[BUFFER_SIZE];
    //recv function is used to read data from the socket named socket_fd into the size_buffer array.
    if (recv(socket_fd, size_buffer, BUFFER_SIZE, 0) == -1) {
        // If the recv call encounters an error, display a message.
        perror("Error receiving tar file size from server");
        //CThe file pointer is closed.
        fclose(fp);
        //Indiactes a failure.
        return 1;
    }
    //The function converts this string to a long integer using the atol function.
    long tar_file_size = atol(size_buffer);
    //File size to be received is printed.
    printf("File size to be received: %ld \n", tar_file_size);

    //Buffer_size stored in the buffer.
    char buffer[BUFFER_SIZE];
    //It is used to convert the tar_file_size into string format.
    sprintf(size_buffer, "%ld", tar_file_size);

    //After receiving the file size, the client sends an acknowledgement back to the server.
    if (send(socket_fd, size_buffer, strlen(size_buffer), 0) != strlen(size_buffer)) {
        // If the send call does not send the expected number of bytes, an error message is printed .
        perror("Error acknowledging to server");
        //the file pointer is closed.
        fclose(fp);
        //The function returns 1.
        return 1;
    }

    // If the server indicates that no files are to be sent then,
    if (strcmp(size_buffer, "0") == 0) {
        //is used to set all the bytes in the buffer array to 0.
        memset(buffer, 0, BUFFER_SIZE);
        // uses the read function to read data from the socket into the buffer array
        int n = read(socket_fd, buffer, BUFFER_SIZE - 1);
        //If n is less than 0,
        if (n < 0) {
            //it prints an error message.
            perror("TCP Client - Read Error");
            //and exits the program with an error status.
            exit(EXIT_FAILURE);
        }
        //If n is zero, it means server is disconnected.
        if (n == 0) {
            printf("Server disconnected.\n");
        }
        //the string functions can correctly determine the end of the data in the buffer.
        buffer[n] = '\0';
        //This line prints the contents of the buffer array as a string. 
        printf("Server response: \n%s\n", buffer);
        //indicate that no files were found on the server,
        return 1;	
    }
    //keep track of the total number of bytes that have been received from the server.
    long bytes_received = 0;
    size_t n;
    // This loop that receives data from the server and writes it to a local file. 
    while (bytes_received < tar_file_size && (n = recv(socket_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        //This will check whether the number of elements written is equal to the number of elements received or not.
        if (fwrite(buffer, sizeof(char), n, fp) != n) {
            //If there was an error writing to the file
            perror("Ther is an issue in writing to tar file");
            //The loop is terminated.
            break;
        }
        //adds the number of bytes received in the current iteration.
        bytes_received += n;
    }
    // After successfully receiving and writing the entire tar file, prints message.
    printf("Successfully file received!\n");
    //closes the file pointer using fclose. 
    fclose(fp);
    //It then returns 0 to indicate success.
    return 0;
}
//validate_dates function parses the input date strings, converts them to time values and then comapre it.
int validate_dates(const char *date1, const char *date2) {
    //This will store the components of date and time.
    struct tm tm1 = {0}, tm2 = {0};
    //Time values are stored in this variables.
    time_t time1, time2;
    //If braeking down the date string process is unsuccessful,
    if (strptime(date1, "%Y-%m-%d", &tm1) == NULL) {
        //Displays an msg that it is unsuccessful.
        printf("Breaking down the date string is unsuccessful: %s\n", date1);
        return 1;

    }
    // The mktime function is then used to convert these components into time1.
    time1 = mktime(&tm1);
    //This will break down the date2,and stores the parsed values in the tm2.
    if (strptime(date2, "%Y-%m-%d", &tm2) == NULL) {
        //
        printf("Failed to break down the string: %s\n", date2);
        return 1;
    }
    //The mktime function is then used to convert these components into time2.
    time2 = mktime(&tm2);

    // difftime calculates the difference in seconds between two time_t values
    if (difftime(time1, time2) <= 0) {
        //If time1 is less than time2, indicates dates are valid.
        return 0;
    }
    //If time 1 is greater than time2 , indicates dates are not in correct order. 
    else {
        //indicate invalid dates.
        return 1;
    }
}

// the read_filenames function extracts filenames from the input string and also sets the unzip flag. 
void read_filenames(char* buffer, char filenames[MAX_FILES][MAX_FILENAME_LEN], int* num_files, int* unzip_flag) {
    //array buffer_copy used to create copy of teh buffer content.
    char buffer_copy[BUFFER_SIZE];
    // to copy the content of the original buffer into the buffer_copy array. 
    strcpy(buffer_copy, buffer);
    //The pointer is used to point tokens.
    char* token;
    // This array is used as the delimiter for splitting the string into tokens.
    char delim[] = " ";
    int i = 0;
    //as a default we are setting the value of the "unzip" flag to 0 
    *unzip_flag = 0; 
    //tokenize the buffer_copy using space delimeter.
    token = strtok(buffer_copy, delim);
    //This is done to continue tokenizing the string.
    token = strtok(NULL, delim);
    i++;

    //This loop will continue till it will reach to the limit of filenames.
    while (token != NULL && i <= MAX_FILES + 1) {
        //If the current word is -u then,
        if (strcmp(token, "-u") == 0) {
            //unzip_falg is set to 1
            *unzip_flag = 1;	
        } else {
            //If the word is not -u then, assumed to filename.
            strncpy(filenames[i], token, MAX_FILENAME_LEN);	
            // incremented to track the number of filenames read.	
            i++;
        }
        //After processing a word, call again to get the next word.
        token = strtok(NULL, delim);
    }
    // after all tokens have been processed, the total number of filenames read is stored in num_files.
    *num_files = i;
}

// The send_command function sends a command string from the client to the server over a TCP connection.
void send_command(int client_fd, char* buffer) {
    //checks if the number of bytes sent is not equal to the length of the data in the buffer or not.
    if (send(client_fd, buffer, strlen(buffer), 0) != strlen(buffer)) 
    {   //The message indicates that there was a "Send Error" on the TCP client.
        perror("TCP Client - Send Error");
        //If the    re's an error during sending, the program exits with a failure status.
        exit(EXIT_FAILURE);
    }
}

// the validate_command function takes a command string as input and prove  whether the command stick to the specified rules.
int validate_command(char *buffer) {
    //
    char *valid_commands[] = {FIND_FILE, S_GET_FILES, D_GET_FILES, GET_FILES, GET_TAR_GZ, QUIT};
    int buffer_length = strlen(buffer);
    char buffer_copy[BUFFER_SIZE];
    strcpy(buffer_copy, buffer);
    int num_commands = sizeof(valid_commands) / sizeof(valid_commands[0]);
    char *token;
    int arg_count = 0;
    token = strtok(buffer_copy, " ");
    // checks if the current token matches the "QUIT" command.
    if (strcmp(token, QUIT) == 0) {
        //Continues untill token returns null,
        while (token != NULL) {
            // used to count the number of arguments
            arg_count++;
            //retrieve the next token in the command string
            token = strtok(NULL, " ");
        }
        //If only one argument,
        if (arg_count == 1) {
            //indicates command is valid.
            return 1;	
        }
        //If more than one argument, 
        else {
            //display an msg that extra aruguments are found.
            printf("Extra arguments found after the command: %s\n", QUIT);
            //indicates command is invalid.
            return 0;
        }
    }
    //checks if the current token starts with the characters in the FIND_FILE command
    else if (strncmp(token, FIND_FILE, strlen(FIND_FILE)) == 0) {
        //continues untill tokens are null
        while (token != NULL) {
            // used to count the number of arguments
            arg_count++;
            //retrieve the next token in the command string
            token = strtok(NULL, " ");
        }
        //If there are two arguments,
        if (arg_count == 2) {
            //indicates command is valid.
            return 1;	
        }
        // arguments are less than two then,
        else if (arg_count < 2){
            //display a msg with after the command, file name is required.
            printf("File name required after the command: %s\n", FIND_FILE);
            return 0;
        }//If there are more than 2 arguments are found, 
        else {
            //display a msg that extra aruguments are found.
            printf("Extra arguments found after the command: %s\n", FIND_FILE);
            //indicates command is invalid.
            return 0;
        }
        // checks if the current token starts with the characters in the "S_GET_FILES" command. 
    } else if (strncmp(token, S_GET_FILES, strlen(S_GET_FILES)) == 0) {
        //used to store the minimum and maximum sizes for fetching files.
        int min_value, max_value;
        //retrieves the next token in the command string
        token = strtok(NULL, " ");
        //If either of these conditions is true
        if (token == NULL || strcmp(token, "-u") == 0) { 
            //it indicates that the min and max sizes were not provided 
            printf("Min and max sizes are not provided for fetching files based on sizes after %s.\n", S_GET_FILES);
            return 0;
        }
        //continue through each character in the token.
        for (int i = 0; i < strlen(token); i++) {
            // if each character is a valid digit
            if (token[i] < '0' || token[i] > '9') {
                //If any character is not a valid digit, displays a msg.
                printf("Value %s passed in argument 1 is not an integer\n", token);
                //indicate invalid command.
                return 0;
            }
        }
        //convert the token into an int.
        min_value = atoi(token);
        //separate a new word.
        token = strtok(NULL, " ");
        //checks the separated word is null or not.
        if (token == NULL) {
            //If null then display a msg.
            printf("Max size is not provided for fetching files based on sizes.\n");
            return 0;
        }
        //continue through each character in the token.
        for (int i = 0; i < strlen(token); i++) {
            //if each character is a valid digit
            if (token[i] < '0' || token[i] > '9') {
                //If any character is not a valid digit, displays a msg.
                printf("Value %s passed in argument 2 is not an integer\n", token);
                //indicate invalid command.
                return 0;
            }
        }
        //convert the token into an int and stores in max_value.
        max_value = atoi(token);
        //if the maximum size is less than the minimum size.
        if (max_value < min_value) {
            //Max size should be greater than or equal to min value.
            printf("Min value: %d is not lesser than the Max value: %d argument.\n", min_value, max_value);
            return 0;
        }
        //retrieves the next token in the command string
        token = strtok(NULL, " ");
        //If either of these conditions is true,
        if (token == NULL || strcmp(token, "-u") == 0) {
            //checks if token is not NULL and if the next token is not NULL as well. 
            if (token != NULL && strtok(NULL, " ") != NULL) {
                //display a msg that extra arguments are found.
                printf("Extra arguments found after the command: %s\n", S_GET_FILES);
                //indicate an invalid command.
                return 0;        
            }
            //indicate a valid "S_GET_FILES" command.
            return 1;
        }      
        //This line prints an error message indicating that extra arguments were found after the "S_GET_FILES" command.  
        printf("Extra arguments found after the command: %s\n", S_GET_FILES);
        return 0;
    // This condition checks if the current token starts with the characters in the "D_GET_FILES" command. 
    } else if (strncmp(token, D_GET_FILES, strlen(D_GET_FILES)) == 0) {
        // used to store the minimum dates for fetching files.
        char min_date[BUFFER_SIZE];
        //used to store the maximum dates for fetching files.
        char max_date[BUFFER_SIZE];
         //retrieves the next token in the command string
        token = strtok(NULL, " ");
        //If either of these conditions is true,
        if (token == NULL || strcmp(token, "-u") == 0) { 
            //display a msg min and max dates are not provided for fetching files
            printf("Min and max dates are not provided for fetching files based on dates after %s.\n", D_GET_FILES);
            return 0;
        }
        //copies the token into the min_date character array.
        strcpy(min_date, token);
        //retrieves the next token in the command string
        token = strtok(NULL, " ");
        // no maximum date was provided after the D_GET_FILES command.
        if (token == NULL) {
            //display a msg.
            printf("Max date is not provided for fetching files based on sizes.\n");
            return 0;
        }
        //copies the token into the max_date character array.
        strcpy(max_date, token);
        //validate_dates function to check if the dates provided are valid.
        if (validate_dates(min_date, max_date)) {
            //Do not follow the date format
            printf("Dates passed as arguments either do not follow the date format or min date is after max date\n");
            return 0;
        }
        token = strtok(NULL, " ");
        //This condition checks if the token is NULL or if it matches the "-u" flag.
        if (token == NULL || strcmp(token, "-u") == 0) {
            //checks if token is not NULL and if the next token is not NULL as well.
            if (token != NULL && strtok(NULL, " ") != NULL) {
                //extra arguments are found.
                printf("Extra arguments found after the command: %s\n", D_GET_FILES);
                return 0;        
            }
            //indicate a valid "D_GET_FILES" command.
            return 1;
        }        
        // prints an error message indicating that extra arguments were found after the "D_GET_FILES" command.
        printf("Extra arguments found after the command: %s\n", D_GET_FILES);
        return 0;
        //This condition checks if the current token starts with the characters in the "GET_FILES" command.
    } else if (strncmp(token, GET_FILES, strlen(GET_FILES)) == 0) {
        //retrieves the next token in the command string
        token = strtok(NULL, " ");
        //This condition checks if the token is NULL or if it matches the "-u" flag.
        if (token == NULL || strcmp(token, "-u") == 0) {
            // filenames to be fetched were not provided as expected after the "GET_FILES" command
            printf("Filenames to be fetched are not provided after %s.\n", GET_FILES);
            return 0;
        }
        //The loop continues until it encounters a NULL token or the "-u" flag.
        while(token != NULL && strcmp(token, "-u") != 0) {
            //keeps track of the number of filenames encountered.
            arg_count++;
            token = strtok(NULL, " ");
            //checks if the token is NULL
            if (token == NULL) {
                //checks the number of filenames are within the limit or not.
                if (arg_count <= MAX_FILES) {
                    //indicate a valid "GET_FILES" command.
                    return 1;
                } // too many filenames were provided then,
                else {
                    //print the message that too many arguments.
                    printf("Extra arguments found after the command: %s\n", GET_FILES);
                    // indicate an invalid command.
                    return 0;
                }
                // checks if the token matches the "-u" flag. 
            } else if (strcmp(token, "-u") == 0) {
                // the total number of filenames encountered is within the limit then,
                if (arg_count <= MAX_FILES && strtok(NULL, " ") == NULL) {
                    // indicate a valid GET_FILES command.
                    return 1;
                }// if the total number of filenames exceeds the limit 
                else {//too many arguments
                    printf("Extra arguments found after the command: %s\n", GET_FILES);
                    //indicate an invalid command.
                    return 0;
                }
            }
        }
        //checks if the current token starts with the characters in the GET_TAR_GZ command.
    } else if (strncmp(token, GET_TAR_GZ, strlen(GET_TAR_GZ)) == 0) {
        //retrieves the next token in the command string.
        token = strtok(NULL, " ");
        // if the token is NULL  or if the token matches the "-u" flag then,
        if (token == NULL || strcmp(token, "-u") == 0) {
            //indicates that the extensions to be fetched were not provided as expected
            printf("Extensions to fetch files are not provided after %s.\n", GET_TAR_GZ);
            // indicate an invalid command.
            return 0;
        }
        //continues until it encounters a NULL token or the -u flag.
        while(token != NULL && strcmp(token, "-u") != 0) {
            //keeps track of the number of extensions encountered.
            arg_count++;
            //retrieves the next token in the command string.
            token = strtok(NULL, " ");
            //checks if the token is NULL
            if (token == NULL) {
                //checks the number of filenames are within the limit or not.
                if (arg_count <= MAX_FILES) {
                    return 1;
                }
                // too many filenames were provided then,
                 else {
                    //print the message that too many arguments.
                    printf("Extra arguments found after the command: %s\n", GET_TAR_GZ);
                    //indicate an invalid command.
                    return 0;
                }
                // checks if the token matches the "-u" flag. 
            } else if (strcmp(token, "-u") == 0) {
                // the total number of filenames encountered is within the limit then,
                if (arg_count <= MAX_FILES && strtok(NULL, " ") == NULL) {
                    // indicate a valid GET_FILES command.
                    return 1;
                }// if the total number of filenames exceeds the limit
                 else {
                    //too many arguments
                    printf("Extra arguments found after the command: %s\n", GET_TAR_GZ);
                    return 0;
                }
            }
        }
    }
    //the default return value for any command that is not recognized or is invalid.
    return 0;	
}
//represents a TCP client that interacts with a server to perform various commands related to file operations.
int main(int argc, char const *argv[]) {
    //set the variables for client_fd,sockaddr_in
    int client_fd, n;
    struct sockaddr_in address;
    char buffer[BUFFER_SIZE] = {0};
    
	//establish a communication channel with the server.
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        //if there is an issue to establish connection, it will throw an error.
        perror("TCP Client - Socket Creation Error\n");
        //terminate the program with a failure status.
        exit(EXIT_FAILURE);
    }
    //initialize the address structure with the necessary information for the client to connect to the server.
    memset(&address, '0', sizeof(address));
    //AF_INET corresponds to IPv4 addresses.
    address.sin_family = AF_INET;
    //the port number is converted to network byte order.
    address.sin_port = htons(PORT);

    //This function converts the text representation of the server IP address into a binary format.
    if (inet_pton(AF_INET, SERVER_IP_ADDR, &address.sin_addr) <= 0) {
        //Throws an error that invalid address.
        perror("TCP Client -Invalid Address");
        //terminate the program with a failure status.
        exit(EXIT_FAILURE);
    }
    //attempts to establish a connection to the server using the information in the address structure.
    if (connect(client_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        //connection attempt fails, an error message is printed,
        perror("TCP Client - Connection Error");
        //terminate the program with a failure status.
        exit(EXIT_FAILURE);
    }
    //attempts to receive data from the server.
    if (recv(client_fd, buffer, BUFFER_SIZE, 0) == -1) {
        //Error receiving connection status from server.
        perror("TCP Client - Error receiving connection status from server");
    }
    // checks if the received data in the buffer is equal to the string CONN_SUCCESS.
    if (strcmp(buffer, CONN_SUCCESS) == 0) {
        //the initial connection to the main server was successful
        printf("Connected to server successfully.\n");
    } 
    //If the initial connection was not successful
    else {
       //The current client socket is closed.
        close(client_fd);
        client_fd = 0;
        // new socket is created using socket to prepare for the mirror server connection.
        if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            //throws an error.
            perror("TCP Client-Mirror Socket Creation Error\n");
            exit(EXIT_FAILURE);
        }
        char mirror_ip[IP_LENGTH], mirror_port[PORT_LENGTH];
        char *ip, *port;
       
        char buffer_copy[BUFFER_SIZE];	
        //The buffer content is copied to a temporary buffer buffer_copy
        strcpy(buffer_copy, buffer);
        //This buffer is tokenized to extract the mirror server IP address.
        ip = strtok(buffer_copy, ":");
         //This buffer is tokenized to extract the mirror server port.
        port = strtok(NULL, ":");
        // The extracted IP address stored in mirror_ip 
        strncpy(mirror_ip, ip, sizeof(mirror_ip));
        // The port stored in mirror_port 
        strncpy(mirror_port, port, sizeof(mirror_port));
        //A new address structure is initialized with the mirror server's IP address and port.
        memset(&address, '0', sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(atoi(mirror_port));
         //This function converts the text representation of the server IP address into a binary format.
        if (inet_pton(AF_INET, mirror_ip, &address.sin_addr) <= 0) {
            perror("TCP Client-Invalid Address");
            exit(EXIT_FAILURE);
        }
       //attempts to establish a connection to the server using the information in the address structure.
        if (connect(client_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("TCP Client-Mirror Connection Error");
            exit(EXIT_FAILURE);
        }

        memset(buffer, 0, sizeof(buffer));
        
    //attempts to receive data from the server.
        if (recv(client_fd, buffer, BUFFER_SIZE, 0) == -1) {
            //error receiving connection status from mirror server.
            perror("TCP Client-Error receiving connection status from mirror server");
        }
        // checks if the received data in the buffer is equal to the string CONN_SUCCESS.
        if (strcmp(buffer, CONN_SUCCESS) == 0) {
            printf("Connected to mirror server successfully.\n");
        }
        //If the initial connection was not successful
         else {
            //throws an error that could not connenct to mirror server.
            perror("Could not connect to main server or mirror server");
            //terminates the program with failure status.
            exit(EXIT_FAILURE);
        }
    }
    // that keeps the client running until explicitly stopped or an error occurs.
    while (1) {
       //user has to enter command.
        printf("Enter command: ");
        //reads the user's input from the standard input and stores it in the buffer array. 
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strlen(buffer)-1] = '\0';
        // checks whether the user's input is a valid command using the validate_command function.
        if (!validate_command(buffer)) {
            // If the command is invalid, an error message is displayed.
            printf("Invalid Command: %s\n", buffer);
            continue;
        }
        //This checks if the user wants to quit the program. 
        if (strcmp(buffer, QUIT) == 0) {
            // the client sends the command to the server
            send_command(client_fd, buffer);
            break;
        }//handles the case where the user's input is a findfile command. 
        else if (strncmp(buffer, FIND_FILE, strlen(FIND_FILE)) == 0) {
            //sends the command to the server 
            send_command(client_fd, buffer);
          
            memset(buffer, 0, BUFFER_SIZE);
            // reads the server's response
            n = read(client_fd, buffer, BUFFER_SIZE - 1);
            //if n less than 0 then,
            if (n < 0) {
                //client- read error display.
                perror("TCP Client-Read Error");
                exit(EXIT_FAILURE);
            }
            //if n is equal to 0 then,
            if (n == 0) {
                //display that serever disconnected.
                printf("Server disconnected.\n");
                break;
            }
            buffer[n] = '\0';
            // displays the file details.
            printf("File details: \n%s\n", buffer);
        }
        //handles the "s_getfiles" command. 
        else if (strncmp(buffer, S_GET_FILES, strlen(S_GET_FILES)) == 0) {
            char unzip_status[BUFFER_SIZE] = "";
            sscanf(buffer, "%*s %*d %*d %s", unzip_status);
            // It sends the command to the server
            send_command(client_fd, buffer);
            //receives files from the server using receive_files
            int receive_status = receive_files(client_fd);
           
            int unzip = strncmp(unzip_status, "-u", strlen("-u")) == 0 ? 1 : 0;
            //unzip is requested if not receiving any failure signal
            if (unzip && !receive_status) {
                // unzipping process is about to begin
                printf("Unzipping %s\n", TAR_FILE_NAME);
                char system_call[BUFFER_SIZE] = "tar -xzvf";
                strcat(system_call, TAR_FILE_NAME);
                //command performs the unzipping of the specified file using the tar utility with the specified options.
                system(system_call);
            }
        }// handles the "d_getfiles" command.
        else if (strncmp(buffer, D_GET_FILES, strlen(D_GET_FILES)) == 0) {
            char unzip_status[BUFFER_SIZE] = "";
            char min_date[BUFFER_SIZE];
            char max_date[BUFFER_SIZE];
            sscanf(buffer, "%*s %s %s %s", min_date, max_date, unzip_status);
            // It sends the command to the server
            send_command(client_fd, buffer);
            //receives files from the server using receive_files
            int receive_status = receive_files(client_fd);
            //checks if the unzip_status string is -u.
            int unzip = strncmp(unzip_status, "-u", strlen("-u")) == 0 ? 1 : 0;
            //unzip is requested if not receiving any failure signal
            if (unzip && !receive_status) {
                 // unzipping process is about to begin
                printf("Unzipping %s\n", TAR_FILE_NAME);
                char system_call[BUFFER_SIZE] = "tar -xzvf";
                //appends the TAR_FILE_NAME to the system_call
                strcat(system_call, TAR_FILE_NAME);
                //command performs the unzipping of the specified file
                system(system_call);
            }
        }//handles the getfiles command. 
        else if (strncmp(buffer, GET_FILES, strlen(GET_FILES)) == 0) {
            char filenames[MAX_FILES][MAX_FILENAME_LEN];
            int num_files, unzip_flag;
            //It sends the command to the server.
            send_command(client_fd, buffer);
            //reads the filenames and unzip flag using read_filenames
            read_filenames(buffer, filenames, &num_files, &unzip_flag);
            //receives files.
            int receive_status = receive_files(client_fd);
            //unzip is requested if not receiving any failure signal
            if (unzip_flag && !receive_status) {
                  // unzipping process is about to begin
                printf("Unzipping %s\n", TAR_FILE_NAME);
                char system_call[BUFFER_SIZE] = "tar -xzvf";
                //appends the TAR_FILE_NAME to the system_call
                strcat(system_call, TAR_FILE_NAME);
                 //command performs the unzipping of the specified file
                system(system_call);
            }
        }//handles the gettar command.
         else if (strncmp(buffer, GET_TAR_GZ, strlen(GET_TAR_GZ)) == 0) {
            char extensions[MAX_FILES][MAX_FILENAME_LEN];
            int num_extensions, unzip_flag;
            //It sends the command to the server.
            send_command(client_fd, buffer);
            //reads the filenames and unzip flag using read_filenames
            read_filenames(buffer, extensions, &num_extensions, &unzip_flag);
            int receive_status = receive_files(client_fd);
            //unzip is requested if not receiving any failure signal
            if (unzip_flag && !receive_status) {
                // unzipping process is about to begin
                printf("Unzipping %s\n", TAR_FILE_NAME);
                char system_call[BUFFER_SIZE] = "tar -xzvf";
                //appends the TAR_FILE_NAME to the system_call
                strcat(system_call, TAR_FILE_NAME);
                //command performs the unzipping of the specified file
                system(system_call);
            }
        } //any other input that doesn't match the predefined commands
        else {
            //sends the user's input as a command to the server
            send_command(client_fd, buffer);
            
            memset(buffer, 0, BUFFER_SIZE);
            //receives the server's response
            n = read(client_fd, buffer, BUFFER_SIZE - 1);
            //if n less than 0 
            if (n < 0) {
                //client read error.
                perror("TCP Client-Read Error");
                exit(EXIT_FAILURE);
            }//if n is equal to 0 
            if (n == 0) {
                //then server disconnected.
                printf("Server disconnected.\n");
                break;
            }
            buffer[n] = '\0';
            //display the server response.
            printf("Server response: \n%s\n", buffer);
        }
    }
    // when the user inputs the quit command
    //the client closes the connection to the server using close(client_fd).
    close(client_fd);
    // indicate successful execution.
    return 0;
}