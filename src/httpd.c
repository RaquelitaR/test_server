#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <glib.h>


const char *LOG_FILE = "log_file.log";
int sockfd, connfd;

void sighandler() {
    // if SIGINT signal (Ctrl-C) then print
    fprintf(stdout, "\nShutting down...\n");
    // close connection
    shutdown(connfd, SHUT_RDWR);
    close(connfd);
}

int main(int argc, char *argv[]) {

    // check number of arguments
    if (argc != 2) {
        fprintf(stderr, "expected usage: %s <port> \n", argv[0] );
        exit(0);
    }
    // set portnumber:test
    int PORT = atoi(argv[1]);
    fprintf(stdout, "Listening on port %d\n", PORT);

    signal(SIGINT, sighandler);

    int r;
    struct sockaddr_in server, client;
    char message[512];

    // Create and bind a TCP socket.
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <  -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Network functions need arguments in network byte order instead of
    // host byte order. The macros htonl, htons convert the values.
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);
    r = bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));
    if (r == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Before the server can accept messages, it has to listen to the
    // welcome port. A backlog of one connection is allowed.
    r = listen(sockfd, 1);
    if (r == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        fprintf(stdout, "Waiting for connection...\n");

        // We first have to accept a TCP connection, connfd is a fresh
        // handle dedicated to this connection.
        socklen_t len = (socklen_t) sizeof(client);
        connfd = accept(sockfd, (struct sockaddr *) &client, &len);
        if (connfd == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        fprintf(stdout, "TCP connection accepted\n");

        // Receive from connfd, not sockfd.
        ssize_t n = recv(connfd, message, sizeof(message) - 1, 0);
        if (n == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

	fprintf(stdout, "received from connfd\n");
        message[n] = '\0';
        fprintf(stdout, "Received:\n%s\n", message);


        /*
         split message into lines
         each line is going to contain an http line (header, content type, content length, etc...)
         search for the http method which is always the first line and see which one is (get, post, etc...)
         construct the response depending on what method you got

         example for get write

         "GET HTTP/1.1
         Content-Type: text/html
         Content-Length: 35

         <html>http://www.raquelita.com ip</html>"
        */

        gchar **headers = g_strsplit(message, "\n", -1);
        parseRe

        // Send the message back.

        char response[4096];
        memset(response, 0, sizeof(char) * 4096);
        strcat(response, "GET HTTP/1.1\r\nContent-Type: text/html\r\nContent-Length: 35\r\n<html>http://www.raquelita.com ");
        strcat(response, "127.0.0.1:6525");
        strcat(response, "</html>");

        r = send(connfd, response, (size_t) sizeof(char) * strlen(response), 0);
        if (r == -1) {
            perror("send");
            exit(EXIT_FAILURE);
        }

        // Close the connection.
        r = shutdown(connfd, SHUT_RDWR);
        if (r == -1) {
            perror("shutdown");
            exit(EXIT_FAILURE);
        }
        r  = close(connfd);
        if (r == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
    }
}
