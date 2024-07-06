#include "utils.h"

#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <arpa/inet.h>

#define PORT "1240"




int main(void) 
{

	struct pollfd pfds[2];

	int sockfd;
	// char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	memset(&hints, 0, sizeof(hints));
	hints.ai_family=AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}


	for (p = servinfo; p != NULL; p = p->ai_next) {
		// attempts to run socket()
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		// attempts to bind
		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}


		// exits on first successful bind
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof(s));
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo);

	pfds[0].fd = STDIN_FILENO;
	pfds[0].events = POLLIN;
	pfds[1].fd = sockfd;
	pfds[1].events = POLLIN;
	
	char buf[MAXDATASIZE];

	struct message msg;

	while (1) {
		int poll_count = poll(pfds, 2, -1); // infinite timeout

		if (poll_count == -1) {
			perror("poll");
			exit(1);
		}

		// user input
		if (pfds[0].revents & POLLIN) {
			memset(buf, 0, MAXDATASIZE);
			fgets(buf, MAXDATASIZE-1, stdin);
			if (strcmp("EXIT", buf) == 0) {
				break;
			}

			msg.fd = 0;
			msg.contents = buf;
			msg.len = (int) strlen(buf);

			send_all(&msg, sockfd);
		}


		// read in from server
		if (pfds[1].revents & POLLIN) {
			memset(buf, 0, MAXDATASIZE);
			msg.fd = sockfd;
			msg.contents = NULL;
			msg.len = 0;

			recv_all(&msg, sockfd);

			print_message(&msg);
		}
	}



	return 0;
}
