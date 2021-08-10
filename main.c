#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdint.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/errno.h>
#include  <sys/socket.h>
#include <sys/types.h>
#include "server_tftp.h"




int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("illegal argument\n");
		exit(-1);
	}
	//socket
	int s;
	struct sockaddr_in server_sock;
	unsigned short echoServPort;
	
	echoServPort = atoi(argv[1]);
	
	//Creating UDP socket
	if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
	{
		perror("TTFTP_ERROR:socket()");
	}
	memset(&server_sock,0,sizeof(server_sock));
	
	server_sock.sin_family = AF_INET;  // Internet address family
	server_sock.sin_port = htons(echoServPort); //Local port
	server_sock.sin_addr.s_addr = htonl(INADDR_ANY); //incoming interface
	//binding
	if(bind(s, (const struct sockaddr *) &server_sock, sizeof(server_sock)) < 0)
	{
		perror("TTFTP_ERROR:bind()");
		close(s);
		exit(1);
	}
	while(1)
	{
		struct sockaddr_in client_sock;
		socklen_t slen = sizeof(client_sock);
		ssize_t len;

		tftp_message message;
		uint16_t opcode;

		if ((len = ttftp_recv_message(s, &message, &client_sock, &slen)) < 0)
		{
			continue;
		}
		if (len < 4)
		{
			printf("request with invalid size received\n");
			continue;
		}
		opcode = ntohs(message.opcode);
		if (opcode == WRQ) {
			ttftp_handle_request(&message, len, &client_sock, slen);
		}
		else{
			printf("RECVFAIL\n");
			exit(1);
		}
	}
	close(s); //close socket before exiting
	return 0;
}
