#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(tcp_client, LOG_LEVEL_INF);

/* Configuration */
#define SERVER_PORT 1234
#define SERVER_ADDR "192.168.8.220"
#define BUFFER_SIZE 128
#define STACK_SIZE 2048
#define PRIORITY 7

/* Thread Resources */
K_THREAD_STACK_DEFINE(app_rx_stack, STACK_SIZE);
struct k_thread rx_thread_data;

/* Receiver Thread Function */
void rx_handler(void *p1, void *p2, void *p3) {
    int sock = POINTER_TO_INT(p1);
    char recv_buffer[BUFFER_SIZE];

    LOG_INF("Receiver thread started.");

    while (1) {
        ssize_t bytes_received = zsock_recv(sock, recv_buffer, sizeof(recv_buffer) - 1, 0);

        if (bytes_received < 0) {
            printk("Recv error: %d\n", errno);
            break;
        } else if (bytes_received == 0) {
            printk("Server closed connection\n");
            break;
        }

        recv_buffer[bytes_received] = '\0';
        printk("Received from server: %s\n", recv_buffer);
    }
}

static int sock;

int main(void) {
    while(true){
        struct sockaddr_in server_addr;
        const char *msg = "Hello from Zephyr\n";

        printk("Initializing TCP Client...\n");
        k_sleep(K_SECONDS(1));

        sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sock < 0) {
            printk("Socket creation failed: %d\n", errno);
            continue;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        if (zsock_inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr) <= 0) {
            printk("Invalid address or address not supported\n");
            zsock_close(sock);
            continue;
        }

        if (zsock_connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            printk("Connection failed: %d\n", errno);
            zsock_close(sock);
            continue;
        }
        printk("Connected to %s:%d\n", SERVER_ADDR, SERVER_PORT);

        k_thread_create(&rx_thread_data, app_rx_stack, K_THREAD_STACK_SIZEOF(app_rx_stack),
                        rx_handler, INT_TO_POINTER(sock), NULL, NULL,
                        PRIORITY, 0, K_NO_WAIT);

        while (1) {
            if (zsock_send(sock, msg, strlen(msg), 0) < 0) {
                printk("Send failed: %d\n", errno);
                break;
            }
            LOG_INF("Message sent\n");

            k_sleep(K_SECONDS(1));
        }
        zsock_close(sock);
    }
    
    return 0;
}