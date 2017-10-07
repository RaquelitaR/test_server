#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>

const char *LOG_FILE = "log_file.log";
FILE *log_fd;

void error(char *msg) {
    perror(msg);
    exit(1);
}

void write_get(int client_sock, struct sockaddr_in *client_addr, char *webpage) {
    char term[] = "\r\n";
    char body[4098];
    memset(&body, 0, sizeof(char) * 4098);
    strcat(body, "<!DOCTYPE html><html><head>");
    strcat(body, "<title>Small Server</title>");
    strcat(body, "</head>");
    strcat(body, "<body>");
    strcat(body, "<p>");

    strcat(body, webpage);
    strcat(body, " ");

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, client_addr, client_ip, INET_ADDRSTRLEN);
    strcat(body, client_ip);

    char client_port[16];
    memset(&client_port, 0, sizeof(char) * 16);
    sprintf(client_port, "%i", ntohs(client_addr->sin_port));
    strcat(body, ":");
    strcat(body, client_port);

    strcat(body, "</p>");
    strcat(body, "</body>");
    strcat(body, "</html>");

    char len[16];
    memset(&len, 0, sizeof(char) * 16);
    sprintf(len, "%i", (int)strlen(body));

    char response[4098];
    memset(&response, 0, sizeof(char) * 4098);
    strcat(response, "HTTP/1.1 200 OK");
    strcat(response, term);
    strcat(response, "Content-Type: ");
    strcat(response, "text/html");
    strcat(response, term);
    strcat(response, "Content-Length: ");
    strcat(response, len);
    strcat(response, term);
    strcat(response, "\n");
    strcat(response, body);
    strcat(response, term);

    printf("\nResponse\n%s\n", response);

    if (write(client_sock, response, (int) strlen(response)) == -1) {
        error("ERROR writing to socket");
    }
}
// TODO:
void write_put(int client_sock) { // Add any extra parameter
    // for put is similar to the get one but instead of the body
    // you have to put the body of the request they sent
}

// TODO:
void write_head(int client_sock) { // Add any extra parameter
    // Return: HTTP/1.1 200 OK
}

int main(int argc, char *argv[]) {

    // check number of arguments
    /*if (argc != 2) {
        fprintf(stderr, "expected usage: %s <port> \n", argv[0] );
        exit(0);
    }*/
    // set portnumber: test
    //int PORT = atoi(argv[1]);
    //fprintf(stdout, "Listening on port %d\n", PORT);

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Failed to create a socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(61278);

    if (bind(server_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
        perror("ERROR: Failed to bind socket\n");
        exit(EXIT_FAILURE);
    }

    listen(server_sock, 10);

    int client_sock;
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_size);
    if (client_sock == -1) {
        perror("ERROR: Connection not accpted\n");
        exit(EXIT_FAILURE);
    }

    char request[4098];
    memset(&request, 0, sizeof(char) * 4098);

    if (read(client_sock, request, 4097) == -1) {
        perror("ERROR: Failed to read from socket");
        exit(EXIT_FAILURE);
    }
    printf("\nRequest\n%s\n", request);

    gchar **split = g_strsplit(request, "\n", -1);
    gchar **first = g_strsplit(*split, " ", -1);

    if (strcmp(first[0], "GET") == 0) {
        write_get(client_sock, &client_addr, first[1]);
    } else if (strcmp(first[0], "PUT") == 0) {
        write_put(client_sock); // Add any extra parameter
    } else if (strcmp(first[0], "HEAD") == 0) {
        write_head(client_sock); // Add any extra parameter
    }

    // Close the connection.
    if (shutdown(client_sock, SHUT_RDWR) == -1) {
        perror("Closing socket");
        exit(EXIT_FAILURE);
    }
    if (close(client_sock) == -1) {
        perror("Closing socket");
        exit(EXIT_FAILURE);
    }

    /*if (0 && shutdown(server_sock, 2) == -1) { // Ignore this, I was trying to find out how to close everything
        perror("ERROR shutdown socket");
    }*/

    return 0;
}
