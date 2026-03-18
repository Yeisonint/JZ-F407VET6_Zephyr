#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/tls_credentials.h>
#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(tls_client_lfs, LOG_LEVEL_INF);

/* Network Configuration */
#define SERVER_ADDR "192.168.8.220"
#define SERVER_PORT 1234
#define CA_CERTIFICATE_TAG 1

/* Filesystem Configuration */
#define CA_CERT_PATH "/lfs:/ca.crt"
#define MAX_CERT_SIZE 2048

/* Thread Resources */
#define STACK_SIZE 2048
#define PRIORITY 7
K_THREAD_STACK_DEFINE(app_rx_stack, STACK_SIZE);
struct k_thread rx_thread_data;

/**
 * @brief Load CA certificate from LittleFS into the TLS credential store
 */
int load_ca_cert(void) {
    struct fs_file_t file;
    static uint8_t cert_buf[MAX_CERT_SIZE];
    fs_file_t_init(&file);

    int rc = fs_open(&file, CA_CERT_PATH, FS_O_READ);
    if (rc < 0) {
        LOG_ERR("Could not open %s (Error: %d)", CA_CERT_PATH, rc);
        return rc;
    }

    ssize_t n = fs_read(&file, cert_buf, sizeof(cert_buf) - 1);
    fs_close(&file);

    if (n <= 0) {
        LOG_ERR("Failed to read certificate or file is empty");
        return -EIO;
    }

    /* Null-terminate for PEM format requirement */
    cert_buf[n] = '\0'; 

    /* Register the certificate in the TLS stack */
    rc = tls_credential_add(CA_CERTIFICATE_TAG, 
                           TLS_CREDENTIAL_CA_CERTIFICATE, 
                           cert_buf, 
                           n + 1);

    if (rc < 0 && rc != -EEXIST) {
        LOG_ERR("Failed to register certificate: %d", rc);
        return rc;
    }

    LOG_INF("CA Certificate loaded successfully (%d bytes)", (int)n);
    return 0;
}

/**
 * @brief TLS Data Receiver Thread
 */
void rx_handler(void *p1, void *p2, void *p3) {
    int sock = POINTER_TO_INT(p1);
    char recv_buffer[128];

    LOG_INF("TLS Receiver thread started.");

    while (1) {
        ssize_t len = zsock_recv(sock, recv_buffer, sizeof(recv_buffer) - 1, 0);

        if (len < 0) {
            LOG_ERR("Receive error (errno: %d)", errno);
            break;
        } else if (len == 0) {
            LOG_WRN("Server closed connection");
            break;
        }

        recv_buffer[len] = '\0';
        printk("SERVER: %s\n", recv_buffer);
    }
}

int main(void) {
    /* Note: LittleFS is assumed to be mounted before main */
    k_sleep(K_SECONDS(2)); /* Wait for network stabilization */

    if (load_ca_cert() < 0) {
        LOG_ERR("Critical failure: No CA certificate, no TLS.");
        return -1;
    }

    while (1) {
        int sock;
        struct sockaddr_in server_addr;
        const char *msg = "Hello from Zephyr (Secure)\n";

        LOG_INF("Creating TLS socket...");
        sock = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TLS_1_2);
        if (sock < 0) {
            LOG_ERR("Socket creation failed: %d", errno);
            k_sleep(K_SECONDS(5));
            continue;
        }

        /* TLS CONFIGURATION */
        /* 1. Assign the certificate TAG */
        sec_tag_t sec_tag_list[] = { CA_CERTIFICATE_TAG };
        zsock_setsockopt(sock, SOL_TLS, TLS_SEC_TAG_LIST, 
                         sec_tag_list, sizeof(sec_tag_list));

        /* 2. Disable hostname verification (to accept any IP in 192.168.8.*) */
        int verify = TLS_PEER_VERIFY_NONE;
        zsock_setsockopt(sock, SOL_TLS, TLS_PEER_VERIFY, &verify, sizeof(verify));

        /* Server Address Setup */
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        zsock_inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);

        LOG_INF("Connecting to %s:%d via TLS...", SERVER_ADDR, SERVER_PORT);

        int nodelay = 1;
        if (zsock_setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay)) < 0) {
            LOG_WRN("Could not set TCP_NODELAY (errno: %d)", errno);
        }

        if (zsock_connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            LOG_ERR("TLS Connection failed: %d", errno);
            zsock_close(sock);
            k_sleep(K_SECONDS(5));
            continue;
        }

        LOG_INF("Secure Connection Established!");

        /* Start Receiver Thread */
        k_thread_create(&rx_thread_data, app_rx_stack, K_THREAD_STACK_SIZEOF(app_rx_stack),
                        rx_handler, INT_TO_POINTER(sock), NULL, NULL,
                        PRIORITY, 0, K_NO_WAIT);

        /* Transmission Loop */
        while (1) {
            if (zsock_send(sock, msg, strlen(msg), 0) < 0) {
                LOG_ERR("Send failed: %d", errno);
                break;
            }
            LOG_INF("Message sent");
            k_sleep(K_MSEC(10));
        }

        zsock_close(sock);
        LOG_INF("Restarting connection in 5 seconds...");
        k_sleep(K_SECONDS(5));
    }

    return 0;
}