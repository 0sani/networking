#include "utils.h"

#include <errno.h>
#include <signal.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>




int get_listener_socket(void);
void add_to_pfds(struct pollfd *pfds[], struct message *message_buffers[], int new_fd, int* num_fds, int* size_fds);
void del_from_pfds(struct pollfd pfds[], struct message message_buffers[], int i, int *num_fds);
void handle_connection(struct pollfd *pfds[], struct message *message_buffers[], int listener, int* fd_count, int* fd_size);
void broadcast_message(struct message msg, int listener, struct pollfd* pfds[], int fd_count);
