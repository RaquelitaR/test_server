
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>


//61278
const char *LOG_FILE = "log_file.log";
FILE *log_fd;
struct sockaddr_in serv_addr;
struct sockaddr_in client_addr;
int client_sock;
int port;

// FUNCTIONS
void print_logfile();
void closeConnection();

// GET request has be generated in memory, and should be a HTML 5 page.
// Content should include:
// 1. URL of the requested page
// 2. IP address and port number of the requested client
// When you visit a browser it should display this format:
// http://foo.com/page 123.123.123.123:45678
void write_get(int client_sock, struct sockaddr_in *client_addr, char *webpage) {

    char term[] = "\r\n";
    char body[4098];
    memset(&body, 0, sizeof(char) * 4098);
    strcat(body, "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">");
    strcat(body, "<title>PA2_SERVER</title>");
    strcat(body, "</head>");
    strcat(body, "<body>");
    strcat(body, "<p>");
    strcat(body, "<h1>");
    strcat(body, "<b>");

    char string_port[16];
    memset(&string_port, 0, sizeof(char) * 16);
    sprintf(string_port, "%i", port);
    strcat(body, "http://localhost:");
    strcat(body, string_port);
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

    strcat(body, "</b>");
    strcat(body, "</h1>");
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
        perror("ERROR writing to socket");
    }
}

// HEAD request, just generate the header of the requested page.
// TODO: Return: HTTP/1.1 200 OK
void write_head(int client_sock) { // Add any extra parameter
    /*char response[1024];
    (void)file;
    strcat(response, "HTTP/1.0 200 OK\r\n");
    printf(client_sock, response, strlen(response), 0);*/
}

// PUT requests, the page has to be generated in memory and should be a HTML 5 page.
// Content should include:
// 1. URL of the requested page
// 2. IP address and PORT of the requesting client,
// 3. the data in the body of the PUT request
// TODO:
void write_put(int client_sock) { // Add any extra parameter
    // PUT is similar to the GET but instead of the body
    // you have to put the body of the request they sent
}

// print the data to a single line into a log file
void print_logfile(){

    int status = 200;
    GTimeVal time;
    g_get_current_time(&time);
    time.tv_usec = 0;
    gchar *now = g_time_val_to_iso8601(&time);

    // Print to the log file
    fprintf(log_fd, "%s : %s:%d\n", now, inet_ntoa(client_addr.sin_addr), status);
    printf("%s : %s:%d\n", now, inet_ntoa(client_addr.sin_addr), status);
    g_free(now);
    fprintf(log_fd, "%s", inet_ntoa(client_addr.sin_addr));
    fclose(log_fd);
    //close(client_sock);

}

// Close the connection.
void closeConnection(){
    if (close(client_sock) == -1) {
        perror("Closing socket");
        exit(EXIT_FAILURE);
    }
    if (0 && shutdown(client_sock, SHUT_RDWR) == -1) {
        //I wrote 0 && shutdown so the shutdown function does not run because it always fails.
        // I don't know how to use this properly
        perror("Shutdown socket");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {

    // check number of arguments
    if (argc != 2) {
        fprintf(stderr, "expected usage: %s <port> \n", argv[0] );
        exit(0);
    }
    // set portnumber: test
    port = atoi(argv[1]);
    fprintf(stdout, "Listening on port %d\n", port);

    // Open the log file
    log_fd = fopen(LOG_FILE, "a");
    if(log_fd == NULL){
        printf("Failed to open the log file\n");
        exit(EXIT_FAILURE);
    }

    print_logfile();

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("Failed to create a socket");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port); // Connect to server

    if (bind(server_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {
        perror("ERROR: Failed to bind socket\n");
        exit(EXIT_FAILURE);
    }

    listen(server_sock, 10);


    memset(&client_addr, 0, sizeof(client_addr));
    socklen_t client_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &client_size);
    if (client_sock == -1) {
        perror("ERROR: Connection not accepted\n");
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

    // Checking which response the server gets
    if (strcmp(first[0], "GET") == 0) {
        write_get(client_sock, &client_addr, first[1]);
    } else if (strcmp(first[0], "PUT") == 0) {
        write_put(client_sock); // Add any extra parameter
    } else if (strcmp(first[0], "HEAD") == 0) {
        write_head(client_sock); // Add any extra parameter
    }
    closeConnection();
}
