#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "net.h"
#include "jbod.h"

/////////////////////////implement code from the slides///////////////////////////////


/* the client socket descriptor for the connection to the server */
int cli_sd = -1;

/* attempts to read n (len) bytes from fd; returns true on success and false on failure. 
It may need to call the system call "read" multiple times to reach the given size len. 

//first 5 bytes are fixed but 6-261 will be optiional
if jbod operation returns -1 -> packet contains no data block
if jbod operation returns 1 -> packet contains data bock
*/


static bool nread(int fd, int len, uint8_t *buf) {

  int count = 0; 
  while (count < len) { //Repeats reading fd while count is less than len
    int n = read(fd,buf,len-count);
    if (n <= 0) {
      return false;
    }
    count += n;
  };
  return true; 

}

/* attempts to write n bytes to fd; returns true on success and false on failure 
It may need to call the system call "write" multiple times to reach the size len.
*/

//set the buffer with length and the buffer will be in the array form
static bool nwrite(int fd, int len, uint8_t *buf) {
  int count = 0;
  while (count < len) {
    int n = write(fd,buf, len-count); //Repeats writing fd while count is less than len
    if (n <= 0) {
      return false;
    }
    count += n;
  }
  return true;
}

/* Through this function call the client attempts to receive a packet from sd 
(i.e., receiving a response from the server.). It happens after the client previously 
forwarded a jbod operation call via a request message to the server.  
It returns true on success and false on failure. 
The values of the parameters (including op, ret, block) will be returned to the caller of this function: 

op - the address to store the jbod "opcode"  
ret - the address to store the info code (lowest bit represents the return value of the server side calling the corresponding jbod_operation function. 2nd lowest bit represent whether data block exists after HEADER_LEN.)
block - holds the received block content if existing (e.g., when the op command is JBOD_READ_BLOCK)

In your implementation, you can read the packet header first (i.e., read HEADER_LEN bytes first), 
and then use the length field in the header to determine whether it is needed to read 
a block of data from the server. You may use the above nread function here.  
*/
static bool recv_packet(int sd, uint32_t *op, uint8_t *ret, uint8_t *block) {
  u_int8_t header[HEADER_LEN]; //creating header
  int off = 0; // offset for header

  if (nread(sd,HEADER_LEN,header) == false){ //reads header from the socket
    return false;
  }; 


  memcpy(op,&header[off],4); //store opcode to op
  *op = ntohl(*op);
  off = off + sizeof(*op);
  memcpy(ret, &header[off],1); //store infocode to ret

  int command = (*op << 14); //extracting command from op
  command = command >> 26;

  if ((*ret == 2) || (*ret == 3)) { //check if there is any block to read
    if (nread(sd,256,block) == false) { 
      return false;
    }
  } else {
    block = NULL;
  } 
  return true;
}



/* The client attempts to send a jbod request packet to sd (i.e., the server socket here); 
returns true on success and false on failure. 

op - the opcode. 
block- when the command is JBOD_WRITE_BLOCK, the block will contain data to write to the server jbod system;
otherwise it is NULL.

The above information (when applicable) has to be wrapped into a jbod request packet (format specified in readme).
You may call the above nwrite function to do the actual sending.  
*/
static bool send_packet(int sd, uint32_t op, uint8_t *block) { 
  int infocode; //creates variable for infocode
  int len = 0;
  uint8_t* Packet;
  int command = (op << 14); //extracting command from op
  command = command >> 26;
  if (command == JBOD_WRITE_BLOCK) {
    len = JBOD_BLOCK_SIZE + HEADER_LEN;
  } else {
    len = HEADER_LEN;
  }
  Packet = malloc(len);
  uint32_t networkop = htonl(op);
  if (command == JBOD_WRITE_BLOCK) {//check op code if it's "JBOD_WRITE_BLOCK"
    infocode = 3;
    memcpy(&Packet[0], &networkop, sizeof(networkop)); //creates packet with op, infocode and block
    memcpy(&Packet[4], &infocode,sizeof(infocode));
    memcpy(&Packet[HEADER_LEN],block, 256);
  } else {
    infocode = 1;
    memcpy(&Packet[0], &networkop, sizeof(networkop)); //creaetes packet with op and infocode
    memcpy(&Packet[4], &infocode, sizeof(infocode));
  }
  bool status = nwrite(sd, len, Packet);
  free(Packet);
  Packet = NULL;
  if (status == false) { //write send packet on the sd
      return false;
  };
  return true;
}


/* attempts to connect to server and set the global cli_sd variable to the
 * socket; returns true if successful and false if not. 
 * this function will be invoked by tester to connect to the server at given ip and port.
 * you will not call it in mdadm.c
*/
bool jbod_connect(const char *ip, uint16_t port) {
  cli_sd = socket(AF_INET, SOCK_STREAM, 0);
  if (cli_sd == -1) {
    return false;
  }

  struct sockaddr_in caddr;

  caddr.sin_family = AF_INET;
  caddr.sin_port = htons(port);
  if (inet_aton(ip, &caddr.sin_addr) == 0 ){
    return false;
  }

  if (connect(cli_sd , (const struct sockaddr *)&caddr,sizeof(caddr)) == -1){
    return false;
  }
  return true;
}




/* disconnects from the server and resets cli_sd */
void jbod_disconnect(void) {
  close(cli_sd);
  cli_sd = -1; //set handles to -1 to avoid use after close
}



/* sends the JBOD operation to the server (use the send_packet function) and receives 
(use the recv_packet function) and processes the response. 

The meaning of each parameter is the same as in the original jbod_operation function. 
return: 0 means success, -1 means failure.
*/
int jbod_client_operation(uint32_t op, uint8_t *block) {
  uint8_t ret;
  uint32_t receive_op;

  if (send_packet(cli_sd, op,block) != true) { //call send_packet function to send
    return -1;
  };
  
  if (recv_packet(cli_sd, &receive_op, &ret, block) != true) { //call recv-packet fucntion to receive
    return -1;
  };
  if (op == receive_op) {
    if (ret == 0 || ret == 2)  {
      return 0;
    } else {
      return -1;
    }
  }
  return -1;
}

//network programming slide
//start with connect and disconnect -> nread and nwrite
