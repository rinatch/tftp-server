#ifndef SERVER_TFTP_
#define SERFER_TFTP_

#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>
#include <stdio.h>
#include <netdb.h>
#include <stdint.h>
#include <signal.h>


//given in the question
#define WAIT_FOR_PACKET_TIMEOUT 3
#define NUMBER_OF_FAILURES 7

//opcodes
enum opcode {
     WRQ=2,
     DATA,
     ACK
};

//for  transfering  modes we only use octet
enum mode {
     OCTET=2
};

// tftp message structure 
typedef union {

     uint16_t opcode;
     struct {
          uint16_t opcode; 
          uint16_t block_number;
          uint8_t data[512];
     } data;  // DATA

    struct {
        uint16_t opcode;
        uint8_t filename_and_mode[514];
    } request;    // WRQ

     struct {
          uint16_t opcode;              
          uint16_t block_number;
     } ack;   // ACK

} tftp_message;
////////////////////////////////////          METHODS           ///////////////////////////////////////////////////
/**
 * ttftp_handle_request method. like it's name it handles a request
 * @param m
 * @param len
 * @param client_sock
 * @param slen
 */
void ttftp_handle_request(tftp_message *m, ssize_t len,struct sockaddr_in *client_sock, socklen_t slen);
/**
 * ttftp_recv_message method is responsible on receiving a message
 * @param s
 * @param m
 * @param sock
 * @param slen
 * @return
 */
ssize_t ttftp_recv_message(int s, tftp_message *m, struct sockaddr_in *sock, socklen_t *slen);
/**
 * ttftp_send_ack method is responsible on sending an ACK
 * @param s
 * @param block_number
 * @param sock
 * @param slen
 * @return
 */
ssize_t ttftp_send_ack(int s, uint16_t block_number,struct sockaddr_in *sock, socklen_t slen);


#endif 