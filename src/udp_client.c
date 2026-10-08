/*
 * ============================================================
 * UDP client
 * Datagram-oriented communication with the BSD sockets API
 * ============================================================
 *
 * What the client does:
 *  - create a UDP socket
 *  - find the IP address of the server from its name
 *  - read lines on standard input
 *  - send each line to the server as one datagram (sendto)
 *
 * The client stops at the end of input (Ctrl+D, or end of a pipe).
 *
 * Usage: udp_client <host> [port]
 *   e.g. udp_client localhost
 *        echo "hello" | udp_client localhost 9600
 *
 * Author: Ibrahim Aboubakarine Maiga
 */

#define _DEFAULT_SOURCE // gethostbyname with -std=c11

#include <stdio.h>      // printf, fprintf, fgets
#include <stdlib.h>     // exit, strtol
#include <string.h>     // memset, memcpy, strlen
#include <unistd.h>     // close, isatty
#include <sys/types.h>  // system types
#include <sys/socket.h> // socket, sendto
#include <netinet/in.h> // sockaddr_in, htons
#include <netdb.h>      // gethostbyname

#define DEFAULT_PORT 9600 // port used by the lab
#define SIZE         100  // maximum size of a message

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
    int sockfd;                   /* socket descriptor              */
    struct sockaddr_in serv_addr; /* address of the server          */
    struct hostent *server;       /* information about the server   */
    char message[SIZE];           /* line read on standard input    */
    unsigned short port = DEFAULT_PORT;

    /* ------------------------------------------------------------
     * Check the arguments
     * ------------------------------------------------------------ */
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <host> [port]\n", argv[0]);
        exit(1);
    }
    if (argc == 3)
        port = parse_port(argv[2]);

    /* ------------------------------------------------------------
     * Create the UDP socket
     * ------------------------------------------------------------ */
    sockfd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    /* ------------------------------------------------------------
     * Find the IP address of the server from its name
     * (/etc/hosts, DNS, ...)
     * ------------------------------------------------------------ */
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "Unknown host: %s\n", argv[1]);
        close(sockfd);
        exit(1);
    }

    /* ------------------------------------------------------------
     * Fill in the address of the server
     * ------------------------------------------------------------ */
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0],
           (size_t) server->h_length);
    serv_addr.sin_port = htons(port);

    /* Show a prompt only when a human is typing (not in a pipe). */
    int interactive = isatty(STDIN_FILENO);
    if (interactive)
        printf("Connected to %s:%hu. Type messages, Ctrl+D to quit.\n",
               argv[1], port);

    /* ------------------------------------------------------------
     * Read lines and send each one as a datagram
     * ------------------------------------------------------------ */
    while (1) {
        if (interactive) {
            printf("> ");
            fflush(stdout);
        }
        if (fgets(message, SIZE, stdin) == NULL)
            break;                              /* end of input */

        size_t len = strlen(message);
        ssize_t sent = sendto(sockfd, message, len, 0,
                              (struct sockaddr *) &serv_addr,
                              sizeof(serv_addr));
        if (sent < 0)
            perror("sendto");
        else if ((size_t) sent != len)
            fprintf(stderr, "Warning: only %zd of %zu bytes sent\n", sent, len);
    }

    /* Close the socket */
    close(sockfd);
    return 0;
}
