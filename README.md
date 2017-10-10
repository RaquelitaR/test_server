# test_server
Testing httpd serv_addr
----
# Sequential HTTP serv_addr

 * HTTP is implemented on top of TCP.
 * The serv_addr needs to accept and handle HEAD, GET, and POST requests.
 * It needs to parse the request and reply with a corresponding page.
 * The serv_addr only needs to handle a requests from a single client_addr at a time.
 * Connections need not be persistent.

The program takes the port number as an argument.
To built and run it use the following commands:
make -C ./src
./src/httpd <port>
