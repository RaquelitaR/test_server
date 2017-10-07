#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <glib.h>

// Constants

// Maximum number of simultaneous connections
#define MAX_CONNECTIONS 10

// Name of this beautiful server
const char *SERVER_NAME = "TheBestServer/1.0.0 (IMO)";
// Name of the log file
const char* LOG_FILE = "access.log";
// Timeout in microseconds
const long IDLE_TIME = 30000000;
// Maximum number of messages for persistent connection
const int MAX_NUMBER_ACTIVE = 100;


// Functions

// Close the connection in slot number i
void closeConnection(int i);// BÚIN
// Handle data from already connected client
void handleData();
// Close inactive connections
void closeIdleConnections();
// Close too active connections
void closeTooActiveConnections();
// Create, bind and listen
void setup();
// Handle sigkill message
void handleExit(int sig);
// Handle new connection
void handleConnection();
// Handle incoming request
void handleRequest(int socket);//BÚIN
// Set socket to non blocking
void nonBlocking(int socket);
// Free memory within a hash set
void freeMemory(char* pointer);
// Parse an incoming http request
void parseRequest(gchar **headers, GHashTable *hash);
// Put all active socket into a set to select from
int initSet();
// Create an outgoing http response
gchar* createResponse(GHashTable *hash);

// Variables

// Log file
FILE *log_fd;
// Listening socket
int sockfd;
// Set to select sockets from
fd_set connfds;
// Client and Server structs
struct sockaddr_in server, client;
// Port number for the server
int port;
// Array of file descriptors to keep track of active connections
int connections[MAX_CONNECTIONS];
// Array of the last time when connection was active
gint64 last_active[MAX_CONNECTIONS];
// Array to keep track of how many times a connection has been active
int active_count[MAX_CONNECTIONS];


int main (int argc, char *argv[]) {
    // Register handlers
    signal(SIGINT, handleExit);

    // Must have 2 arguments, programName and port number
    if (argc != 2) {
        printf("Please specify port number\n");
        exit(0);
    }
    port = atoi(argv[1]);
    printf("Port: %d\n", port);

    // Create, bind and listen
    setup();

    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    while (1) {
        // Init the set and find the highest file descriptor
        int maxfd = initSet();

        int selected;
        if ((selected = select(maxfd + 1, &connfds, NULL, NULL, &tv)) <= 0) {
            if (selected == 0) {
                // Nothing to read
                // printf("Nothing to read\n");
            }
            else {
                printf("ERROR: Select failed\n");
                exit(0);
            }
        }
        else {
            // Something to read
            if (FD_ISSET(sockfd, &connfds)) {
                // New connection request
                handleConnection();
            }
            else {
                // New data from already connected clients
                handleData();
            }
        }
        closeIdleConnections();
        closeTooActiveConnections();
    }
}


void nonBlocking(int socket) {
    int opts = fcntl(socket,F_GETFL);
    if (opts < 0) {
        printf("ERROR: Getting socket options failed\n");
        exit(0);
    }
    // Add the nonblock flag
    opts = (opts | O_NONBLOCK);
    if (fcntl(socket,F_SETFL,opts) < 0) {
        printf("ERROR: Setting socket to nonblocking failed\n");
        exit(0);
    }
}


int initSet() {
    int max_socket = sockfd;
    // Clear the set
    FD_ZERO(&connfds);
    // Add the listening socket
    FD_SET(sockfd, &connfds);
    // Add the other sockets to the set
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i] != 0) {
            FD_SET(connections[i], &connfds);
            if (connections[i] > max_socket) {
                max_socket = connections[i];
            }
        }
    }
    return max_socket;
}


void parseRequest(gchar **headers, GHashTable *hash) {
    gchar **first_line = g_strsplit(headers[0], " ", -1);
    g_hash_table_insert(hash, g_strdup("method"), g_strdup(g_strstrip(first_line[0])));
    g_hash_table_insert(hash, g_strdup("path"), g_strdup(g_strstrip(first_line[1])));
    g_hash_table_insert(hash, g_strdup("version"), g_strdup(g_strstrip(first_line[2])));
    g_strfreev(first_line);

    int i = 1;
    while (headers[i+1] != NULL) {
        gchar **line = g_strsplit(headers[i], ":", 2);
        if (line[1] == NULL) {
            g_strfreev(line);
            break;
        }
        g_hash_table_insert(hash, g_ascii_strdown(line[0], -1), g_strdup(g_strstrip(line[1])));
        i++;
        g_strfreev(line);
    }
    // Add the body to the hash table if it's a POST request
    if (g_strcmp0(g_hash_table_lookup(hash, "method"), "POST") == 0) {
        g_hash_table_insert(hash, g_strdup("body"), g_strdup(g_strstrip(headers[++i])));
    }
}


gchar* createResponse(GHashTable *hash) {
    int extra_headers_allocated = 0, path_split_allocated = 0, cookie_pairs_allocated = 0, posted_allocated = 0;
    int status_code = 200;
    char* status_message = "OK";
    int port = (int)ntohs(client.sin_port);
    gchar* method = g_hash_table_lookup(hash, "method");
    gchar* host = g_hash_table_lookup(hash, "host");
    gchar* path = g_hash_table_lookup(hash, "path");
    gchar* version = g_hash_table_lookup(hash, "version");

    GTimeVal gtime;
    g_get_current_time(&gtime);
    gtime.tv_usec = 0;
    gchar *curr_time = g_time_val_to_iso8601(&gtime);

    // Print to the access log
    fprintf(log_fd, "%s : %s:%d %s\n%s%s : %d\n", curr_time, inet_ntoa(client.sin_addr), port, method, host, path, status_code);

    // Print the same to the screen
    printf("%s : %s:%d %s\n%s%s : %d\n", curr_time, inet_ntoa(client.sin_addr), port, method, host, path, status_code);
    g_free(curr_time);

    // HTTP formatted time
    char timebuf[50];
    time_t now = time(0);
    strftime(timebuf, sizeof(timebuf), "%a, %d %b %Y %H:%M:%S %Z", localtime(&now));

    // For posted data or appending to html
    gchar* posted = "";

    // Parse the query
    gchar* page = path;
    gchar** split;
    gchar** arguments;
    gchar* query = "";
    GHashTable *argument_hash = g_hash_table_new_full(g_str_hash, g_str_equal, (void*)freeMemory, (void*)freeMemory);
    if (g_strrstr(path, "?") != NULL) {
        // Remove the rest of the URI after the # if it's included
        gchar** remove_end = g_strsplit(path, "#", 2);
        // Split on the ?
        split = g_strsplit(remove_end[0], "?", 2);
        g_strfreev(remove_end);
        path_split_allocated = 1;
        page = split[0];
        // Save the query string
        query = g_strdup(split[1]);
        // Parse the query to a hash table
        arguments = g_strsplit(split[1], "&", -1);
        for (guint i = 0; i < g_strv_length(arguments); i++) {
            gchar** key_value = g_strsplit(arguments[i], "=", 2);
            g_hash_table_insert(argument_hash, g_strdup(key_value[0]), g_strdup(key_value[1]));
            g_strfreev(key_value);
        }
    }

    // Parse the cookies
    gchar** pairs;
    GHashTable *cookie_hash = g_hash_table_new_full(g_str_hash, g_str_equal, (void*)freeMemory, (void*)freeMemory);
    if (g_hash_table_contains(hash, "cookie")) {
        pairs = g_strsplit(g_hash_table_lookup(hash, "cookie"), ";", -1);
        cookie_pairs_allocated = 1;
        for (guint i = 0; i < g_strv_length(pairs); i++) {
            char** key_value = g_strsplit(g_hash_table_lookup(hash, "cookie"), "=", 2);
            g_hash_table_insert(cookie_hash, g_strdup(g_strstrip(key_value[0])), g_strdup(g_strstrip(key_value[1])));
            g_strfreev(key_value);
        }
    }

    // For additional headers appended to the response
    gchar* extra_headers = "";

    // Set the default color
    gchar* color = "white";

    // The final result
    gchar* result = NULL;

    // Dynamic content for each page

    // Check if it's the color page, account for the British version as well
    if (g_strcmp0(page, "/color") == 0|| g_strcmp0(page, "/colour") == 0) {
        // Read the color from the path or a cookie header
        if (g_hash_table_contains(argument_hash, "bg") && g_hash_table_lookup(argument_hash, "bg") != NULL) {
            color = g_hash_table_lookup(argument_hash, "bg");
            extra_headers = g_strdup_printf("Set-Cookie:bg=%s\r\n", color);
            extra_headers_allocated = 1;
        }
        else if (g_hash_table_contains(cookie_hash, "bg")) {
            color = g_hash_table_lookup(cookie_hash, "bg");
        }
        posted = "<form action=\"/test\" method=\"get\">Say something:<br><input type=\"text\" name=\"parameter\" placeholder=\"I'm giving up on you..\"><br><br><input type=\"submit\" value=\"Submit\"></form>";
    }
    else if (g_strcmp0(page, "/") == 0) {
        posted = "<p>This site is rather boring!</p><p>Try /color or /test</p><p>Also try using POST</p>";
    }
    else if (g_strcmp0(page, "/test") == 0) {
        posted = g_strdup_printf("<form action=\"/color\" method=\"get\">What's your favourite color?<br><input type=\"text\" name=\"bg\" placeholder=\"just admit it, it's purple..\"><br><br><input type=\"submit\" value=\"Submit\"></form><p>%s</p>", query);
        posted_allocated = 1;
    }
    else {
        char* message = "I don't know what you are looking for but you won't find it here";
        status_code = 404;
        char* status_message = "Method not allowed";
        if (g_strcmp0(method, "HEAD") == 0) {
            result = g_strdup_printf("%s %d %s\r\n\r\n", version, status_code, status_message);
        }
        else {
            result = g_strdup_printf("%s %d %s\r\n\r\n<html><head><title>Sorry</title></head><body>%d %s</body></html>", version, status_code, status_message, status_code, message);
        }
    }

    if (status_code == 200) {
        // Add posted string to the body of the response if it's a POST request
        if (g_strcmp0(method, "POST") == 0) {
            if (posted_allocated) {
                g_free(posted);
            }
            gchar* content = g_hash_table_lookup(hash, "body");
            posted = g_strdup_printf("<p>%s</p>", content);
            posted_allocated = 1;
        }
        // The body of the page
        gchar *body = g_strdup_printf("<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>C you in hell</title></head><body style=\"background-color:%s\"><h1>http://%s%s %s:%d</h1>%s</body></html>", color, host, path, inet_ntoa(client.sin_addr), port, posted);
        // The http headers
        gchar *response = g_strdup_printf("%s %d %s\r\nDate: %s\r\nServer: %s\r\nConnection: Keep-Alive\r\nKeep-Alive: timeout=30, max=100\r\nContent-Length: %d\r\nContent-Type: text/html\r\n%s\r\n", version, status_code, status_message, timebuf, SERVER_NAME, (int)strlen(body), extra_headers);

        if (g_strcmp0(method, "POST") == 0) {
            result = g_strconcat(response, body, NULL);
        }
        else if (g_strcmp0(method, "GET") == 0) {
            result = g_strconcat(response, body, NULL);
        }
        else if (g_strcmp0(method, "HEAD") == 0) {
            result = g_strdup(response);
        }
        else {
            if (status_code != 404) {
                char* message = "Method not allowed";
                status_code = 405;
                result = g_strdup_printf("%s %d %s\r\n\r\n<html><head><title>Sorry</title></head><body>%d %s</body></html>", version, status_code, message, status_code, message);
            }
        }
        g_free(response);
        g_free(body);
    }

    // Cleanup
    if (extra_headers_allocated) {
        g_free(extra_headers);
    }
    if (cookie_pairs_allocated) {
        g_strfreev(pairs);
    }
    if (path_split_allocated) {
        g_strfreev(arguments);
        g_strfreev(split);
        g_free(query);
    }
    if (posted_allocated) {
        g_free(posted);
    }

    g_hash_table_destroy(argument_hash);
    g_hash_table_destroy(cookie_hash);

    return result;
}


void handleConnection() {
    socklen_t len = (socklen_t)sizeof(client);
    int connfd = accept(sockfd, (struct sockaddr *) &client, &len);
    if (connfd < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            printf("ERROR: No connections waiting\n");
        } else {
            printf("ERROR: Accept failed with message: %s\n", strerror(errno));
        }
        return;
    }
    // Socket should not block
    nonBlocking(connfd);

    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i] == 0) {
            printf("Connection accepted in slot %d with file descriptor %d\n", i, connfd);
            connections[i] = connfd;
            last_active[i] = g_get_monotonic_time();
            active_count[i] = 0;
            connfd = -1;
            break;
        }
    }
    if (connfd != -1) {
        // No connection available
        printf("ERROR: No available slots\n");
        close(connfd);
    }
}


void handleExit(int sig) {
    fclose(log_fd);
    close(sockfd);
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i] != 0) {
            closeConnection(i);
        }
    }
    printf("\nGot signum %d, goodbye cruel world...\n", sig);
    exit(0);
}


void handleRequest(int i) {
    // Init the buffer
    char message[4096];
    memset(&message, 0, sizeof(message));
    // array indexing
    size_t n = recv(connections[i], message, sizeof(message), 0);
    if (n <= 0) {
        if (n == 0) {
            printf("ERROR: Nothing to read from socket\n");
        }
        else {
            printf("ERROR: Reading from socket failed\n");
        }
        closeConnection(i);
        return;
    }
    message[4095] = '\0';

    // Print the request
    // printf("Received request of size n = %zd:\n%s\n", n, message);

    GHashTable *hash = g_hash_table_new_full(g_str_hash, g_str_equal, (void*)freeMemory, (void*)freeMemory);
    gchar **headers = g_strsplit(message, "\n", -1);
    // Parse the http request
    parseRequest(headers, hash);
    // Create a http response
    gchar *response = createResponse(hash);
    // Send it to the client
    send(connections[i], response, sizeof(char) * strlen(response), 0);

    // Print the response
    // printf("Sending response of size: %lu\n%s\n", sizeof(char) * strlen(response), response);

    // Close the connection if the close token is in the header or if it's a HTTP/1.0 connection without a keep-alive token
    if (((g_hash_table_contains(hash, "connection") == TRUE) && (g_strcmp0(g_hash_table_lookup(hash, "connection"), "close") == 0))
        || ((g_strcmp0(g_hash_table_lookup(hash, "method"), "HTTP/1.0") == 0 && ((g_hash_table_contains(hash, "connection") == FALSE) || (g_strcmp0(g_hash_table_lookup(hash, "connection"), "keep-alive") != 0))))) {
        printf("CLOSE: Not a persistent connection or asked to close\n");
        closeConnection(i);
    }

    // Free memory
    g_free(response);
    g_hash_table_destroy(hash);
    g_strfreev(headers);
}


void setup() {
    // Init the connection array
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        connections[i] = 0;
    }

    // Open the log file, create it if it doesn't exist
    log_fd = fopen(LOG_FILE, "a");
    if (log_fd == NULL) {
        printf("ERROR: Opening log file failed\n");
    }

    // Create and TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("ERROR: Creating socket failed\n");
        exit(0);
    }

    // Set the socket to nonblocking
    nonBlocking(sockfd);

    // Bind to it
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    if ((bind(sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server))) < 0) {
        printf("Error: Binding to socket failed\n");
        exit(0);
    }

    // Put a timeout on it
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        printf("ERROR: Setting timeout on socket failed\n");
    }

    // Allow MAX_CONNECTIONS to connect
    if (listen(sockfd, MAX_CONNECTIONS) < 0) {
        printf("ERROR: listening to port failed\n");
        exit(0);
    }
    printf("Http server listening on port %d\n", port);
}


void freeMemory(char* pointer) {
    g_free(pointer);
}


void closeIdleConnections() {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i] != 0) {
            if (g_get_monotonic_time() - last_active[i] > IDLE_TIME) {
                printf("Connection in slot %d timed out\n", i);
                closeConnection(i);
            }
        }
    }
}


void closeTooActiveConnections() {
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (connections[i] != 0) {
            if (active_count[i] == MAX_NUMBER_ACTIVE) {
                printf("Connection in slot %d closed because it connected %d times\n", i, MAX_NUMBER_ACTIVE);
                closeConnection(i);
            }
        }
    }
}


void handleData() {
    // Serve all connections that have new incoming data
    for (int i = 0; i < MAX_CONNECTIONS; i++) {
        if (FD_ISSET(connections[i], &connfds)) {
            printf("Handle request for slot %d with fd = %d\n", i, connections[i]);
            // Update the time when the connection was last active
            last_active[i] = g_get_monotonic_time();
            // Update the number of times the connection has been active
            active_count[i] += 1;
            // Handle the request
            handleRequest(i);
        }
    }
}


void closeConnection(int i) {
    shutdown(connections[i], SHUT_RDWR);
    close(connections[i]);
    connections[i] = 0;
    active_count[i] = 0;
}