#include "utils.h"

// what's version control lmao

// honestly this doesn't work but i'm making shaky assumptions under which it probably works
void recv_all_WIP(struct message *msg, int fd)
{

	// cases:
	// new message -> read in length of message then read message
	// partial recv -> try to read in as much as possible and append to message
	//	note: this read should be capped by the amount left to send
	char buf[MAXDATASIZE];	
	char* contents;
	// already initialized to zero
	int received;
	int total = 0;

	msg->fd = fd;
	
	if (msg->sent == 1) {
		msg->contents = NULL;
		msg->len = 0;
	}

	// if new message
	if (msg->contents == 0) {
		if (!read(fd, &buf, MAXDATASIZE - 1)) {
			msg->len = -1;
			return;
		}
		
		// reads in the length of the map
		msg->len = (int) strtol(buf, &contents, 10);
		char* tmp = malloc(sizeof(char) * msg->len + 1 );

		contents++; // skip the leading space.


		received = strlen(contents);
		
		// copy minimal of received bytes and length
		strncpy(tmp, contents, received < msg->len ? received : msg->len);


		total += received;

		msg->contents = tmp;
	}

	// eat up the rest of our currently waiting packets
	while (total < msg->len) {
		int ret = read(fd, &buf, MAXDATASIZE-1);
		if (ret == 0) {
			return;
		}
		if (ret == -1) {
			msg->len = -1;
			return; 
		}
		received = strlen(buf);

		// don't think this line is right
		total += received;
		strncat(msg->contents, buf, total < msg->len ? total : msg->len);
	}
}

void recv_all_working(struct message *msg, int fd)
{
	char buf[MAXDATASIZE] = {0};
	int numbytes = read(fd, buf, MAXDATASIZE-1);
	msg->contents = buf;
	msg->len = numbytes;
}

void recv_all(struct message *msg, int fd) 
{
	recv_all_working(msg, fd);
}




void send_all(struct message *msg, int dest_fd) {
	// error checking? What's that
	send(dest_fd, msg->contents, strlen(msg->contents), 0);
}

// note: should always be IPv4
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		// IPv4
		return &(((struct sockaddr_in*) sa)->sin_addr); // cast to IPv4 and get address
	}
	
	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}


void print_message(struct message* msg)
{
	// idk why getting the time messes this up, probably something about the buffer
	time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    // Print the formatted time
    printf("[%s]: Sender: %3d\tContents: %s\n", buffer, msg->fd, msg->contents);
}

