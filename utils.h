#include <unistd.h>
#include <stdlib.h> 
#include <string.h>
#include <stdio.h>
#include <poll.h>



#include <sys/socket.h>
#include <netinet/in.h>

#include <time.h>



#define MAXDATASIZE 256



// len of zero means the connection closed
// negative len is an error
// in both cases contents is null
struct message {
	int fd;
	int len;
	char* contents;
	int sent;
};

void *get_in_addr(struct sockaddr *sa);

void recv_all(struct message *msg, int fd);
void print_message(struct message *msg);
void send_all(struct message *msg, int dest_fd);
