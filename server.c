#include "server.h"


#define PORT "1240"



int get_listener_socket(void) 
{
	int listener;

	int yes = 1;
	int rv;
	struct addrinfo hints, *servinfo, *p;

	// prepare hints for getaddrinfo()
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE; // use my IP

	// getaddrinfo()
	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// socket() and bind calls()
	// loops and fbinds to first one
	for (p = servinfo; p != NULL; p = p->ai_next) {
		// attempts to run socket()
		if ((listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		// deals with port already in use
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		// attempts to bind
		if (bind(listener, p->ai_addr, p->ai_addrlen) == -1) {
			close(listener);
			perror("server: bind");
			continue;
		}


		// exits on first successful bind
		break;
	}
	
	freeaddrinfo(servinfo); // done with servinfo

	// check for failure to bidn
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		return -1;
	}

	// attempts to listen
	if (listen(listener, 10) == -1) {
		perror("listen");
		return -1;
	}
	
	return listener;
}

void add_to_pfds(struct pollfd *pfds[], struct message *message_buffers[], int new_fd, int* num_fds, int* size_fds)
{
	// printf("Adding to pfds...\n");
	// realloccate if necessary
	if (*num_fds == *size_fds) {
		*size_fds *= 2;

		*pfds = realloc(*pfds, sizeof(**pfds) * (*size_fds));
		*message_buffers = realloc(*message_buffers, sizeof(**message_buffers) * (*size_fds));
	}
	// printf("Before assigning: %d\n", (*pfds)[*num_fds].fd);
	(*pfds)[*num_fds].fd = new_fd;
	// printf("After assigning: %d\n", (*pfds)[*num_fds].fd );
	(*pfds)[*num_fds].events = POLLIN;
	(*message_buffers)[*num_fds].fd = new_fd;
	(*num_fds)++;
}

void del_from_pfds(struct pollfd pfds[], struct message message_buffers[], int i, int *num_fds)
 {
	pfds[i] = pfds[*num_fds -1];
	message_buffers[i] = message_buffers[*num_fds -1];

	(*num_fds)--;
}

void handle_connection(struct pollfd *pfds[], struct message *message_buffers[], int listener, int* fd_count, int* fd_size)
{
	char remoteIP[INET6_ADDRSTRLEN];
	
	struct sockaddr_storage remote;
	socklen_t addr_len = sizeof(remote);
	

	int newfd = accept(listener, (struct sockaddr*)&remote, &addr_len);

	if (newfd == -1) {
		perror("accept");
	} else {
		add_to_pfds(pfds, message_buffers, newfd, fd_count, fd_size);
		printf("pollserver: new connection from %s on socket %d\n",
				inet_ntop(remote.ss_family,
				get_in_addr((struct sockaddr*)&remote),
				remoteIP, INET6_ADDRSTRLEN),
				newfd);
	}
}




void broadcast_message(struct message msg, int listener, struct pollfd* pfds[], int fd_count) {
	for (int i = 0; i < fd_count; i++) {
		if ((*pfds)[i].fd != listener && (*pfds)[i].fd != msg.fd) {
			send_all(&msg, (*pfds)[i].fd);
		}
	}

}



int main(void) {


	int listener = get_listener_socket();

	if (listener == -1) {
		fprintf(stderr, "error getting listening socket\n");
		exit(1);
	}


	// set up room for listeners
	int fd_count = 1;
	int fd_size = 5;
	struct pollfd *pfds = malloc(sizeof(struct pollfd) *fd_size);
	struct message *message_buffers = calloc(fd_size, sizeof(struct message)); 

	pfds[0].fd = listener;
	pfds[0].events = POLLIN;


	// polling main loop
	// while true
	//   check that our polling is working correctly
	//   loop through fds and try to read
	//		if ready:
	//			if listener is ready, then we have new connection
	//		otherwise:
	//			check for error or disconnect
	//			read in data and broadcast to everyone

	printf("Waiting for connections...\n");
	while (1) {
		int poll_count = poll(pfds, fd_count, -1); // infinite timeout

		if (poll_count == -1) {
			perror("poll");
			exit(1);
		}


		for (int i = 0; i < fd_count; i++) {
			if (pfds[i].revents & POLLIN) { // checks if fd is ready
				if (pfds[i].fd == listener) {
					handle_connection(&pfds, &message_buffers, listener, &fd_count, &fd_size);
				} else {
					// receive bytes
					int fd = pfds[i].fd;
					recv_all(&message_buffers[i], fd);


					struct message msg = message_buffers[i];

					printf("Msg: %s", msg.contents);

					// store errors in the message struct
					if (msg.len == 0) {
						printf("pollserver: socket %d hung up\n", msg.fd);
						close(msg.fd);
						del_from_pfds(pfds, message_buffers, i, &fd_count);
						continue;
					} 
					if (msg.len <= 0) {
						perror("recv");
						close(msg.fd);
						del_from_pfds(pfds, message_buffers, i, &fd_count);
						continue;
					}

					printf("Msg now: %s", msg.contents);

					print_message(&msg);

					
					printf("Msg and now: %s", msg.contents);


					// check if ready to broadcast and broadcasts if ready.

					// broadcast to everyone
					broadcast_message(msg, listener, &pfds, fd_count);					
				}
			}
		}

	}





	return 0;
}
