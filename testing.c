#include "server.h"

void test_recv_all(char* test_data) {
    struct message* msg = calloc(1, sizeof(struct message));
    int pipefd[2];
    
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    // Write test data to the pipe
    write(pipefd[1], test_data, strlen(test_data));
    close(pipefd[1]);
    
    // Read the message from the pipe
    recv_all(msg, pipefd[0]);

    if (msg->len_front <= 0) {
        printf("Failed to read message\n");
    } else {
        printf("Front: %s\n", msg->contents_front);
        if (msg->contents_back) {
            printf("Back: %s\n", msg->contents_back);
        }
        free(msg->contents_front);
        if (msg->contents_back) {
            free(msg->contents_back);
        }
    }
    close(pipefd[0]);
    free(msg);
}

void test_recv_all_stdin(void) {
    struct message* msg = calloc(1, sizeof(struct message));

    recv_all(msg, 0);

    if (msg->len_front <= 0) {
        printf("Failed to read message\n");
    } else {
        printf("Front: %s\n", msg->contents_front);
        if (msg->contents_back) {
            printf("Back: %s\n", msg->contents_back);
        }
        free(msg->contents_front);
        if (msg->contents_back) {
            free(msg->contents_back);
        }
    }

    free(msg);
}

void test_recv_all_gpt_generated(void) {
    test_recv_all("12 Hello World!");
    test_recv_all("5 Hello");
    test_recv_all("15 Partial ");
    test_recv_all("8 aaaabbbbccccddddeeee");

}


int main(void) {
    test_recv_all_gpt_generated();

    test_recv_all_stdin();

    return 0;
}
