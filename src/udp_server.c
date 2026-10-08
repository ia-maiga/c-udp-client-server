/*
 * ============================================================
 * UDP server
 * Datagram-oriented communication with the BSD sockets API
 * ============================================================
 *
 * What the server does:
 *  - create a UDP socket
 *  - bind it to a well-known port (9600 by default)
 *  - wait for datagrams sent by clients
 *  - print each message with the address of its sender
 *
 * Important:
 *  - UDP is CONNECTIONLESS: there is no listen() and no accept()
 *  - every datagram is received independently with recvfrom(),
 *    which also tells us who sent it
 *
 * Usage: udp_server [port]
 *
 * Author: Ibrahim Aboubakarine Maiga
 */

#define _POSIX_C_SOURCE 200809L // sigaction, inet_ntop with -std=c11

#include <stdio.h>      // printf, perror
#include <stdlib.h>     // exit, strtol
#include <string.h>     // memset
#include <unistd.h>     // close
#include <signal.h>     // signal, SIGINT
#include <sys/types.h>  // system types
#include <sys/socket.h> // socket, bind, recvfrom
#include <netinet/in.h> // sockaddr_in, htons
#include <arpa/inet.h>  // inet_ntop

#define DEFAULT_PORT 9600 // port used by the lab
#define SIZE         100  // buffer size (the lab asks for >= 20 chars)

/* Set to 0 by the Ctrl+C handler to leave the main loop cleanly. */
static volatile sig_atomic_t running = 1;

static void on_sigint(int sig)
{
    (void) sig;
    running = 0;
}

/* Reads a port number from a string; exits on invalid input. */
static unsigned short parse_port(const char *s)
{
    char *end;
    long p = strtol(s, &end, 10);
    if (*s == '\0' || *end != '\0' || p < 1 || p > 65535) {
        fprintf(stderr, "Invalid port: %s (expected 1-65535)\n", s);
        exit(1);
    }
    return (unsigned short) p;
}

int main(int argc, char *argv[])
{
    int sockfd;                   /* socket descriptor            */
    struct sockaddr_in serv_addr; /* local address of the server  */
    struct sockaddr_in cli_addr;  /* address of the sender        */
    socklen_t cli_len;            /* size of the sender address   */
    char buffer[SIZE];            /* received message             */
    unsigned short port = DEFAULT_PORT;

    if (argc > 2) {
        fprintf(stderr, "Usage: %s [port]\n", argv[0]);
        exit(1);
    }
    if (argc == 2)
        port = parse_port(argv[1]);

    /* ------------------------------------------------------------
     * Create the socket
     * PF_INET    : TCP/IP protocol family
     * SOCK_DGRAM : datagram socket (UDP)
     * 0          : default protocol (UDP for SOCK_DGRAM)
     * ------------------------------------------------------------ */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    /* ------------------------------------------------------------
     * Fill in the local address of the server
     * ------------------------------------------------------------ */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;                 // Internet family
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // all interfaces
    serv_addr.sin_port = htons(port);               // network byte order

    /* ------------------------------------------------------------
     * Attach the socket to this address (bind)
     * ------------------------------------------------------------ */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    /* Stop cleanly on Ctrl+C. Without SA_RESTART, the blocking
     * recvfrom() is interrupted and returns -1 with errno = EINTR. */
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigint;
    sigaction(SIGINT, &sa, NULL);

    printf("UDP server listening on port %hu (Ctrl+C to stop)\n", port);
    fflush(stdout);

    /* ------------------------------------------------------------
     * Main loop: wait for datagrams
     * ------------------------------------------------------------ */
    while (running) {
        cli_len = sizeof(cli_addr);

        /* Blocking call: waits until a datagram arrives.
         * We keep one byte for the final '\0'. */
        ssize_t n = recvfrom(sockfd, buffer, SIZE - 1, 0,
                             (struct sockaddr *) &cli_addr, &cli_len);
        if (n < 0) {
            if (!running)
                break;          /* interrupted by Ctrl+C */
            perror("recvfrom");
            continue;
        }
        buffer[n] = '\0';

        /* Remove the trailing newline sent by the client, if any. */
        if (n > 0 && buffer[n - 1] == '\n')
            buffer[n - 1] = '\0';

        /* Who sent it? Convert the binary IP address to text. */
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &cli_addr.sin_addr, ip, sizeof(ip));

        printf("[%s:%hu] %s\n", ip, ntohs(cli_addr.sin_port), buffer);
        fflush(stdout);
    }

    printf("\nServer stopped.\n");
    close(sockfd);
    return 0;
}
