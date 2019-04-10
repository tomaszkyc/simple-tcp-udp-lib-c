/* TCP and UDP client-server functions */

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_BUFFER 128


/**
 * 
 *  General functions
 *  
 */



void error(char* msg){
    perror(msg); //print error message to stderr
    exit(EXIT_FAILURE); //quit
}





int create_socket(int family_type){

    int socket_id;

    //create socket
    socket_id = socket(AF_INET, family_type, 0);

    //if error when creating socket
    if ( socket_id == -1 ){
        
        error("ERROR: can't create socket");
        return socket_id;

    }

    //print the status and socket id
    printf("[+] Created socket: %d\n", socket_id);

    return socket_id;
}

int create_udp_socket(){
    return create_socket( SOCK_DGRAM );
}

int create_tcp_socket(){
    return create_socket( SOCK_STREAM );
}



int close_connection(int socket_id){

    //close connection to socket
    int status = close(socket_id);

    //if error when closing socket
    if ( status == -1 ){
        char buffer[MAX_BUFFER];
        sprintf(buffer, "ERROR: can't close connection for socket: %d", socket_id);
        error(buffer);
        return status;
    }

    //print success message
    printf("[+] Connection successfully closed for socket: %d\n", socket_id);
    return status;
}


int bind_port(int socket, int port_number){

    //create structure for AF_INET family
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port_number);
    memset( &( address.sin_zero ), '\0', 8 );

    //binding protocol address with socket
    int status = bind(socket, (struct sockaddr *) & address, sizeof(struct sockaddr));

    //if error when binding
    if (status == -1) {

        char buffer[MAX_BUFFER];
        sprintf(buffer, "ERROR: Can't to bind port: %d", port_number);
        error(buffer);
        return status;
    }

    //print success message
    printf("[+] Port: %d successfully binded.\n", port_number);

    return status;
}


void demonize(){

    pid_t pid, sid;
    int chdir_func_status, file_descriptor;

    //check, if daemon already exists
    if ( getppid() == 1 ) {
        error("ERROR: unable to demonize, becouse is already daemon\n");
    }

    //fork current process
    pid = fork();

    //when can't fork process
    if (pid < 0) {
        error("ERROR: Can't fork process\n");
    }

    if (pid > 0) {
        //parent dead
        exit(EXIT_SUCCESS);
    }

    //new SID
    sid = setsid();

    signal(SIGHUP,SIG_IGN);

    pid = fork();

    if (pid != 0) {
        //first child is dead
        exit(EXIT_SUCCESS);
    }

    if (sid < 0) {
        error("ERROR: can't create new SID.\n");
    }

    //need to change current dir
    chdir_func_status = chdir("/");

    //if error when changing dir
    if (chdir_func_status < 0) {
        error("ERROR: unable to change directory to /: %s\n");
    }

    //close other descriptors
    file_descriptor = open("/dev/null", O_RDWR, 0);

    if (file_descriptor != -1) {
        dup2 (file_descriptor, STDIN_FILENO);
        dup2 (file_descriptor, STDOUT_FILENO);
        dup2 (file_descriptor, STDERR_FILENO);

        if (file_descriptor > 2) {
            close (file_descriptor);
        }
    }

    //resetting file creation mask
    umask(027);

}


void handle_incoming_client(int socket, void (*func)(), int use_syslog) {
    int fresh_socket, close_status;
    struct sockaddr_in address;
    socklen_t address_len = sizeof(struct sockaddr_in);
    struct in_addr client_ip;
    char * ip;
    unsigned short port_number;

    //waiting for action
    while ( 1 ) {
        
        //accept client request from queue
        fresh_socket = accept(socket, (struct sockaddr *) & address, & address_len);
        client_ip = address.sin_addr;
        ip = inet_ntoa(client_ip);
        //translate from network to host notation
        port_number = ntohs(address.sin_port);

        if (fresh_socket == -1) {
            char buffer[MAX_BUFFER];
            sprintf(buffer, "ERROR: Can't accept client request from %s and port: %d\n", ip, port_number );
            error(buffer);
        }

        printf("[+] Client request successfully accepted from %s and port: %d\n", ip, port_number);

        /* send message to client */
        func(fresh_socket);

        //if we want to use syslog
        if (use_syslog == 1) {
            syslog(LOG_NOTICE, "[+] Client request successfully accepted from %s and port: %d\n", ip, port_number);
        }

        //close connection
        close_status = close(fresh_socket);

        //if error when closing connection
        if (close_status == -1) {
            char buffer[MAX_BUFFER];
            sprintf(buffer, "ERROR: Can't close client connection from %s and port: %d\n", ip, port_number );
            error(buffer);
        }

        printf("[+] Client connection from %s and port: %d successfully closed.\n", ip, port_number);
    }
}

void receive_from_server(int socket_id) {
    int in;
    char buffer[MAX_BUFFER + 1];

    //read data from socket
    while ((in = read(socket_id, buffer, MAX_BUFFER)) > 0) {
        buffer[in] = 0;
        printf("\n%s", buffer);
    }

    //when error 
    if (in < 0){
        char buffer[MAX_BUFFER];
        sprintf(buffer, "ERROR: can't read from socket: %d or connection closed too early\n", socket_id);
        error(buffer);
    }
}

/**
 * 
 * TCP functions
 * 
 * 
 */



int create_connection_tcp(int socket_id, const char * ip, int port) {

    int connect_status;
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons(port);
    memset( &( address.sin_zero ), '\0', 8 );

    printf("[-] Trying to connect: %s and port: %d ...\n", ip, port);

    //creating connection
    connect_status = connect(socket_id, (struct sockaddr *) & address, sizeof(struct sockaddr));

    //when can't create connection
    if (connect_status == -1) {
        char buffer[MAX_BUFFER];
        sprintf(buffer, "ERROR: can't create connection to socket: %d\n", socket_id);
        error(buffer);
        return connect_status;
    }

    //show success message
    printf("[+] Connected with socket %d successfully created.\n", socket_id);

    return connect_status;
}



int listen_for_client_tcp(int socket_id, int backlog) {

    //change socket to accept requests
    int listen_status = listen(socket_id, backlog);

    //if error occured
    if (listen_status == -1) {
        char buffer[MAX_BUFFER];
        sprintf(buffer, "ERROR: can't bind for socket %d with backlog %d\n", socket_id, backlog);
        error(buffer);
        return listen_status;
    }

    //print success message
    printf("[+] Listening for socket %d with backlog %d\n", socket_id, backlog);

    return listen_status;
}



/**
 * 
 * Custom functions 
 * 
 * 
 */


void send_time_to_socket(int socket_id) {
    char buffer[MAX_BUFFER + 1];
    time_t current_time;
    int write_status;

    current_time = time(NULL);

    //format time and copy to buffer
    snprintf(buffer, MAX_BUFFER, "%s\n", ctime(&current_time));


    //write to socket
    write_status = write(socket_id, buffer, strlen(buffer));

    //handle write errors
    if (write_status == 0){
        char buffer[MAX_BUFFER];
        sprintf(buffer, "ERROR: buffer data didn't send to socket: %d\n", socket_id);
        error(buffer);
    }
    else if (write_status == -1){
        char buffer[MAX_BUFFER];
        sprintf(buffer, "ERROR: there was a problem when sending data to socket: %d\n", socket_id);
        error(buffer);
    }


}

int shellcmd(char *command){
    int status;

    //execute command
    status = system(command);

    //when error occured
    if (status == -1){
        char buffer[MAX_BUFFER];
        sprintf(buffer, "ERROR: there was a problem when executing a command: %s\n", command);
        error(buffer);
    }

    return status;
}

void send_uptime_to_socket(int socket_id) {
    int write_status;
    char buffer[MAX_BUFFER + 1];


    snprintf(buffer, MAX_BUFFER, "%d\n", shellcmd("uptime"));

    //write to socket
    write_status = write(socket_id, buffer, strlen(buffer));


    //handle write errors
    if (write_status == 0){
        char buffer[MAX_BUFFER];
        sprintf(buffer, "ERROR: buffer data didn't send to socket: %d\n", socket_id);
        error(buffer);
    }
    else if (write_status == -1){
        char buffer[MAX_BUFFER];
        sprintf(buffer, "ERROR: there was a problem when sending data to socket: %d\n", socket_id);        
        error(buffer);
    }
}


bool is_valid_ip_address(char *ip_address){

    struct sockaddr_in sa;
    int result;
    
    result = inet_pton(AF_INET, ip_address, &(sa.sin_addr));
    
    return result != 0;
}

