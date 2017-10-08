# test_server
Testing httpd server
----
# Sequential HTTP server

 * HTTP is implemented on top of TCP.
 * The server needs to accept and handle HEAD, GET, and POST requests.
 * It needs to parse the request and reply with a corresponding page.
 * The server only needs to handle a requests from a single client at a time.
 * Connections need not be persistent.

The program takes the port number as an argument.
To built and run it use the following commands:
make -C ./src
./src/httpd <port>
