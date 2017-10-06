#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


const char *LOG_FILE = "log_file.log";
int fd_server, fd_client;
int on = 1;

struct sockaddr_in server_addr, client_addr;
socklen_t  sin_len = sizeof(client_addr);
char buff[4096];

char web[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"
"<!DOCTYPE html>\r\n"
"<html><head><title>Raquelita</title>\r\n"
"<style>body { background-color: #81A558} </style></head>\r\n"
"<body><center><h1>Testing the new server</h1><br>\r\n";


void sighandler() {
    // if SIGINT signal (Ctrl-C) then print
    fprintf(stdout, "\nShutting down...\n");
    // close connection
    shutdown(fd_client, SHUT_RDWR);
    close(fd_client);
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

    const int sighandler;
    signal(SIGINT,  sighandler);

    int r;
    struct sockaddr_in server_addr, client;
    char message[512];

    // Create and bind a TCP socket.
    fd_server = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_server <  0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    
    setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));


    // Network functions need arguments in network byte order instead of
    // host byte order. The macros htonl, htons convert the values.
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);
    r = bind(fd_server, (struct sockaddr *) &server_addr, (socklen_t) sizeof(server_addr));
    if (r == -1) {
        perror("bind");
        close(fd_server);
        exit(EXIT_FAILURE);
    }

    // Before the server can accept messages, it has to listen to the
    // welcome port. A backlog of one connection is allowed.
    // set to 10 so we have enough connections simultaneously
    r = listen(fd_server, 10);
    if (r == -1) {
        perror("listen");
        close(fd_server);
        exit(EXIT_FAILURE);
    }

    for (;;) {
        fprintf(stdout, "Waiting for connection...\n");

        // We first have to accept a TCP connection, fd_client is a fresh
        // handle dedicated to this connection.
        socklen_t len = (socklen_t) sizeof(client);
        fd_client = accept(fd_server, (struct sockaddr *) &client_addr, &len);
        if (fd_client == -1) {
            perror("Connection failed.. \n");
            continue;
        }

        fprintf(stdout, "TCP connection accepted\n");

        // Receive from fd_client, not fd_server.
        ssize_t n = recv(fd_client, message, sizeof(message) - 1, 0);
        if (n == -1) {
            perror("recv");
            exit(EXIT_FAILURE);
        }

	fprintf(stdout, "received from fd_client\n");
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

        //gchar **headers = g_strsplit(message, "\n", -1);


        // Send the message back.

        char response[4096];
        memset(response, 0, sizeof(char) * 4096);
        strcat(response, "GET HTTP/1.1\r\nContent-Type: text/html\r\nContent-Length: 35\r\n<html>http://www.raquelita.com ");
        strcat(response, "127.0.0.1:6525");
        strcat(response, "</html>");

        r = send(fd_client, response, (size_t) sizeof(char) * strlen(response), 0);
        if (r == -1) {
            perror("send");
            exit(EXIT_FAILURE);
        }

        // Close the connection.
        r = shutdown(fd_client, SHUT_RDWR);
        if (r == -1) {
            perror("shutdown");
            exit(EXIT_FAILURE);
        }
        r  = close(fd_client);
        if (r == -1) {
            perror("close");
            exit(EXIT_FAILURE);
        }
    }
}
