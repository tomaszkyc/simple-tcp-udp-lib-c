#include "libnetutils.h"


#define PORT 9004
#define IP_ADDRESSS "127.0.0.1"


int main(){

  int socket_id;

  printf("********* uptime TCP client example **********\n\n");


  socket_id = create_tcp_socket();

  //create connection
  create_connection_tcp(socket_id, IP_ADDRESSS, PORT);

  //get uptime
  send_uptime_to_socket(socket_id);

  //close connection
  close_connection(socket_id);



  return 0;
}