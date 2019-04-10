#include "libnetutils.h"


#define PORT 9004
#define MAX_QUEUE 10


int main() {

    int socket_id;

    printf("********* uptime TCP server example **********\n\n");

    //create TCP socket
    socket_id = create_tcp_socket();

    //bind port
    bind_port(socket_id, PORT);

    //listen for requests
    listen_for_client_tcp(socket_id, MAX_QUEUE);

    //handle clients requests
    handle_incoming_client(socket_id, send_time_to_socket, 0);

    return 0;
}