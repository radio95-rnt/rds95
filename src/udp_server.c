#include "udp_server.h"

#define BUF_SIZE 512
#define UDP_READ_TIMEOUT_MS 500

static int sockfd = -1;
static struct pollfd poller;
static struct sockaddr_in client_addr;
static socklen_t client_len = sizeof(client_addr);

int open_udp_server(int port) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return -1;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        sockfd = -1;
        return -1;
    }

    poller.fd = sockfd;
    poller.events = POLLIN;

    return 0;
}

void poll_udp_server() {
    static char buf[BUF_SIZE];
    static char cmd_buf[BUF_SIZE];
    static char cmd_output[BUF_SIZE];

    if (poll(&poller, 1, UDP_READ_TIMEOUT_MS) <= 0) return;
    if (!(poller.revents & POLLIN)) return;

    memset(buf, 0, BUF_SIZE);
    client_len = sizeof(client_addr);
	ssize_t bytes_read = recvfrom(sockfd, buf, BUF_SIZE - 1, 0, (struct sockaddr *)&client_addr, &client_len);
    if (bytes_read <= 0) return;

    size_t start = 0;

    for (ssize_t i = 0; i < bytes_read; i++) {
        if (buf[i] == '\n' || (unsigned char)buf[i] == 0xFF) {
            size_t len = i - start;
            if((unsigned char)buf[i] == 0xFF) len += 1;

            if (len > 0 && len < BUF_SIZE) {
                memset(cmd_buf, 0, BUF_SIZE);
                memcpy(cmd_buf, buf + start, len);
                memset(cmd_output, 0, BUF_SIZE);

                size_t out_len = 0;
                run_lua(cmd_buf, len, cmd_output, &out_len);

                if (out_len > 0 &&
                    sendto(sockfd, cmd_output, out_len, 0,
                        (struct sockaddr *)&client_addr, client_len) == -1) {
                    perror("sendto");
                }
            }

            start = i + 1;
        }
    }

    if (start < (size_t)bytes_read) {
        size_t len = bytes_read - start;

        if (len > 0 && len < BUF_SIZE) {
            memcpy(cmd_buf, buf + start, len);

            size_t out_len = 0;
            run_lua(cmd_buf, len, cmd_output, &out_len);

            if (out_len > 0 &&
                sendto(sockfd, cmd_output, out_len, 0,
                    (struct sockaddr *)&client_addr, client_len) == -1) {
                perror("sendto");
            }
        }
    }
}

void close_udp_server() {
    if (sockfd >= 0) close(sockfd);
    sockfd = -1;
    destroy_lua();
}
