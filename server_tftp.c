#include "server_tftp.h"

ssize_t ttftp_send_ack(int s, uint16_t block_number, struct sockaddr_in *sock, socklen_t slen)
{
	tftp_message ack_message;
	ssize_t c;
    ack_message.opcode = htons(ACK);
    ack_message.ack.block_number = htons(block_number);
	if ((c = sendto(s, &ack_message, sizeof(ack_message.ack), 0,(struct sockaddr *) sock, slen)) < 0)
	{
		perror("TTFTP_ERROR: sendto()");
	}
	return c;
}

ssize_t ttftp_recv_message(int s, tftp_message *m, struct sockaddr_in *sock, socklen_t *slen)
{
    ssize_t c;
    if ((c = recvfrom(s, m, sizeof(*m), 0, (struct sockaddr *) sock, slen)) < 0 && errno != EAGAIN)
    {
        perror("TTFTP_ERROR: recvfrom()");
    }
    return c;
}
void ttftp_handle_request(tftp_message *mess, ssize_t len, struct sockaddr_in *client_sock, socklen_t slen)
{
	int s,sret;
	struct protoent *pp;
	struct timeval timeout;
	////////
	char *file_name, *mode_s, *end;
	FILE *fd;
	int mode;
	// handle client request by openning new socket

     if ((pp = getprotobyname("udp")) == 0)
     {
          fprintf(stderr, "TTFTP_ERROR: getprotobyname() \n");
          exit(1);
     }
     if ((s = socket(AF_INET, SOCK_DGRAM, pp->p_proto)) == -1)
     {
          perror("TTFTP_ERROR: socket()");
          exit(1);
     }
	//client request
    file_name = (char*) mess->request.filename_and_mode;
	end = &file_name[len - 2 - 1];

	if (*end != '\0') {
		printf("invalid filename or mode\n");
		exit(1);
	}

	mode_s = strchr(file_name, '\0') + 1;

	if (mode_s > end) {
		printf("transfer mode not specified\n");
		exit(1);
	}
	fd = fopen(file_name, "w");

	if (fd == NULL) {
		perror("TTFTP_ERROR: fopen()");
		exit(1);
	}
	if((strcmp(mode_s,"octet"))==0) mode=OCTET;
	else mode =0;
	
	if (mode == 0) {
		printf("invalid transfer mode\n");
		exit(1);
	}
	printf("request received: %s '%s' %s\n","put", file_name, mode_s);
	//////
	tftp_message m;
	ssize_t c;
	uint16_t block_number = 0;
	int error_count = 0;
	fd_set readfds;
	int to_close = 0;
	//////
	printf("IN:WRQ,%s,%s\n",file_name,mode_s);
	c = ttftp_send_ack(s, block_number, client_sock, slen);
	printf("OUT:ACK,%d\n",block_number);
	if (c < 0)
	{
		printf("RECVFAIL\n");
		exit(1);
	}
	
	//to_close = 0 if the file is not completely written
	while (!to_close) {
		FD_ZERO(&readfds);
		FD_SET(s,&readfds);
		timeout.tv_sec  = WAIT_FOR_PACKET_TIMEOUT;
		timeout.tv_usec = 0;
		
		sret = select(s+1,&readfds,NULL,NULL,&timeout);
		
		// sret = -1 if error happened in select function
		if(sret == -1){
			perror("TFTP_ERROR:select()");
			exit(1);
		}
		//timeout
		else if(sret == 0){
			ssize_t l;
			l=ttftp_send_ack(s, block_number, client_sock, slen);
			if (l < 0) {
				printf("transfer killed\n");
				exit(1);
			}
			printf("OUT:ACK,%d\n",block_number);
			//printf(" timeout  \n");
			error_count++;
			if(error_count == NUMBER_OF_FAILURES){
				printf("FLOWERROR: received %d timeouts\n",NUMBER_OF_FAILURES);
				printf("RECVFAIL\n");
				exit(1);
			}
			continue;
		}
		//if no timeout
		else
		    {
			c = ttftp_recv_message(s, &m, client_sock, &slen);
			if (c >= 0 && c < 4)
			{
				printf("message with invalid size received\n");
				exit(1);
			}
			block_number++;
			if(ntohs(m.opcode) == DATA)
			{
			if(ntohs(m.data.block_number) != block_number)
			{
				printf("FLOWERROR:block_number of data is different from ACK+1\n");
				exit(1);
			}
			printf("IN:DATA,%d,%ld\n",ntohs(m.data.block_number),c);
			}
			// if the size of the data is not 512 so we have got to  the last block and we change
			// to_close to 1
			if (c < sizeof(m.data)) {
				to_close = 1;
			}
			c = fwrite(m.data.data, 1, c - 4, fd);
			if(c>512)
			{
				printf("FLOWERROR: packet size bigger than 512\nRECVFAIL\n");
				exit(1);
			}
			printf("WRITING:%ld\n",c);
			if (c < 0)
			{
				perror("TTFTP_ERROR: fwrite()");
				exit(1);
			}
			c = ttftp_send_ack(s, block_number, client_sock, slen);
			printf("OUT:ACK,%d\n",block_number);
			if (c < 0)
			{
				printf("transfer killed\n");
				exit(1);
			}
		}
	}
     printf("RECVOK\n");

     fclose(fd);
     close(s);
}