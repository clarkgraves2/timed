/**
 * @file test_client.c
 * @brief Enhanced RFC 867 Daytime Protocol client for testing
 *
 * This client reads a format string from stdin and sends it to the server.
 * It includes improved error handling, timeout support, and command-line options.
 * 
 * Usage: ./test_client [options]
 * Options:
 *   -a <address>  Server address (default: 127.0.0.1)
 *   -p <port>     Server port (default: 13)
 *   -t <timeout>  Timeout in milliseconds (default: 5000)
 *   -f <file>     Read format string from file instead of stdin
 *   -v            Verbose output
 *   -h            Show help
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <errno.h>
 #include <fcntl.h>
 #include <poll.h>
 #include <getopt.h>
 
 #define DEFAULT_SERVER_ADDR "127.0.0.1"
 #define DEFAULT_SERVER_PORT 13
 #define DEFAULT_TIMEOUT_MS 5000
 #define BUFFER_SIZE 4096
 
 // Exit codes
 #define EXIT_SUCCESS 0
 #define EXIT_ARGS_ERROR 1
 #define EXIT_NETWORK_ERROR 2
 #define EXIT_TIMEOUT_ERROR 3
 #define EXIT_IO_ERROR 4
 
 // Global flags
 static int verbose = 0;
 
 /**
  * Print usage information
  */
 void print_usage(const char *program_name) {
     fprintf(stderr, "Usage: %s [options]\n", program_name);
     fprintf(stderr, "Options:\n");
     fprintf(stderr, "  -a <address>  Server address (default: %s)\n", DEFAULT_SERVER_ADDR);
     fprintf(stderr, "  -p <port>     Server port (default: %d)\n", DEFAULT_SERVER_PORT);
     fprintf(stderr, "  -t <timeout>  Timeout in milliseconds (default: %d)\n", DEFAULT_TIMEOUT_MS);
     fprintf(stderr, "  -f <file>     Read format string from file instead of stdin\n");
     fprintf(stderr, "  -v            Verbose output\n");
     fprintf(stderr, "  -h            Show this help\n");
 }
 
 /**
  * Parse command line arguments
  */
 int parse_args(int argc, char *argv[], char **server_addr, int *server_port, 
                int *timeout_ms, char **format_file) {
     int opt;
     
     *server_addr = DEFAULT_SERVER_ADDR;
     *server_port = DEFAULT_SERVER_PORT;
     *timeout_ms = DEFAULT_TIMEOUT_MS;
     *format_file = NULL;
     
     while ((opt = getopt(argc, argv, "a:p:t:f:vh")) != -1) {
         switch (opt) {
             case 'a':
                 *server_addr = optarg;
                 break;
             case 'p':
                 *server_port = atoi(optarg);
                 if (*server_port <= 0 || *server_port > 65535) {
                     fprintf(stderr, "Error: Invalid port number\n");
                     return EXIT_ARGS_ERROR;
                 }
                 break;
             case 't':
                 *timeout_ms = atoi(optarg);
                 if (*timeout_ms <= 0) {
                     fprintf(stderr, "Error: Invalid timeout value\n");
                     return EXIT_ARGS_ERROR;
                 }
                 break;
             case 'f':
                 *format_file = optarg;
                 break;
             case 'v':
                 verbose = 1;
                 break;
             case 'h':
                 print_usage(argv[0]);
                 exit(EXIT_SUCCESS);
             default:
                 print_usage(argv[0]);
                 return EXIT_ARGS_ERROR;
         }
     }
     
     return EXIT_SUCCESS;
 }
 
 /**
  * Read format string from file or stdin
  */
 int read_format_string(char *buffer, size_t buffer_size, const char *format_file) {
     FILE *input;
     
     if (format_file != NULL) {
         input = fopen(format_file, "r");
         if (input == NULL) {
             fprintf(stderr, "Error: Cannot open format file: %s\n", strerror(errno));
             return EXIT_IO_ERROR;
         }
     } else {
         input = stdin;
     }
     
     if (fgets(buffer, buffer_size, input) == NULL) {
         if (feof(input)) {
             // Empty input is valid - will use server default format
             buffer[0] = '\0';
         } else {
             if (format_file != NULL) {
                 fclose(input);
             }
             fprintf(stderr, "Error reading format string: %s\n", strerror(errno));
             return EXIT_IO_ERROR;
         }
     } else {
         // Remove trailing newline if present
         size_t len = strlen(buffer);
         if (len > 0 && buffer[len - 1] == '\n') {
             buffer[len - 1] = '\0';
         }
     }
     
     if (format_file != NULL) {
         fclose(input);
     }
     
     return EXIT_SUCCESS;
 }
 
 int main(int argc, char *argv[]) {
     int sock;
     struct sockaddr_in server_addr;
     char buffer[BUFFER_SIZE];
     char format_buffer[BUFFER_SIZE];
     ssize_t bytes_received;
     struct pollfd pfd;
     int result;
     
     // Parse command line arguments
     char *server_addr_str;
     int server_port;
     int timeout_ms;
     char *format_file;
     
     result = parse_args(argc, argv, &server_addr_str, &server_port, &timeout_ms, &format_file);
     if (result != EXIT_SUCCESS) {
         return result;
     }
     
     // Read format string
     result = read_format_string(format_buffer, sizeof(format_buffer), format_file);
     if (result != EXIT_SUCCESS) {
         return result;
     }
     
     if (verbose) {
         printf("Format string: '%s'\n", format_buffer[0] == '\0' ? "(empty)" : format_buffer);
     }
     
     // Create TCP socket
     sock = socket(AF_INET, SOCK_STREAM, 0);
     if (sock < 0) {
         fprintf(stderr, "Socket creation failed: %s\n", strerror(errno));
         return EXIT_NETWORK_ERROR;
     }
     
     // Set up server address
     memset(&server_addr, 0, sizeof(server_addr));
     server_addr.sin_family = AF_INET;
     server_addr.sin_port = htons(server_port);
     
     // Convert IP address from text to binary
     if (inet_pton(AF_INET, server_addr_str, &server_addr.sin_addr) <= 0) {
         fprintf(stderr, "Invalid address: %s\n", strerror(errno));
         close(sock);
         return EXIT_NETWORK_ERROR;
     }
     
     if (verbose) {
         printf("Connecting to %s:%d...\n", server_addr_str, server_port);
     }
     
     // Connect to server
     if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
         fprintf(stderr, "Connection failed: %s\n", strerror(errno));
         close(sock);
         return EXIT_NETWORK_ERROR;
     }
     
     if (verbose) {
         printf("Connected. Sending format string...\n");
     }
     
     // Send format string to server
     if (send(sock, format_buffer, strlen(format_buffer), 0) < 0) {
         fprintf(stderr, "Send failed: %s\n", strerror(errno));
         close(sock);
         return EXIT_NETWORK_ERROR;
     }
     
     if (verbose) {
         printf("Format string sent. Shutting down write side...\n");
     }
     
     // Shutdown the write side of the connection to signal we're done sending
     if (shutdown(sock, SHUT_WR) < 0) {
         fprintf(stderr, "Shutdown failed: %s\n", strerror(errno));
         close(sock);
         return EXIT_NETWORK_ERROR;
     }
     
     if (verbose) {
         printf("Waiting for response (timeout: %d ms)...\n", timeout_ms);
     }
     
     // Set up poll for timeout
     pfd.fd = sock;
     pfd.events = POLLIN;
     
     // Wait for data or timeout
     int poll_result = poll(&pfd, 1, timeout_ms);
     
     if (poll_result < 0) {
         fprintf(stderr, "Poll failed: %s\n", strerror(errno));
         close(sock);
         return EXIT_NETWORK_ERROR;
     } else if (poll_result == 0) {
         fprintf(stderr, "Timeout waiting for server response\n");
         close(sock);
         return EXIT_TIMEOUT_ERROR;
     }
     
     // Receive response from server
     memset(buffer, 0, BUFFER_SIZE);
     bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
     if (bytes_received < 0) {
         fprintf(stderr, "Receive failed: %s\n", strerror(errno));
         close(sock);
         return EXIT_NETWORK_ERROR;
     }
     
     // Print the received response
     if (verbose) {
         printf("Response from server (%zd bytes):\n", bytes_received);
     }
     printf("%s", buffer);
     
     // Close socket
     close(sock);
     return EXIT_SUCCESS;
 }