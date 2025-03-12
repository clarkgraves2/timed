/**
 * @file test_client_udp.c
 * @brief Simple UDP RFC 867 Daytime Protocol client for testing
 *
 * This client reads a format string from stdin and sends it to the server using UDP.
 * Usage: ./test_client_udp < format_file.txt
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <errno.h>
 #include <poll.h>
 
 #define SERVER_ADDR "127.0.0.1"
 #define SERVER_PORT 13
 #define BUFFER_SIZE 1024
 #define TIMEOUT_MS 5000  // 5 second timeout
 
 int main(void) {
     int sock;
     struct sockaddr_in server_addr;
     socklen_t server_len;
     char buffer[BUFFER_SIZE];
     char format_buffer[BUFFER_SIZE];
     ssize_t bytes_received;
     struct pollfd pfd;
     
     // Read format string from stdin
     if (fgets(format_buffer, BUFFER_SIZE, stdin) == NULL) {
         // Empty input is valid - will use server default format
         format_buffer[0] = '\0';
     } else {
         // Remove trailing newline if present
         size_t len = strlen(format_buffer);
         if (len > 0 && format_buffer[len - 1] == '\n') {
             format_buffer[len - 1] = '\0';
         }
     }
     
     printf("Format string: '%s'\n", format_buffer[0] == '\0' ? "(empty)" : format_buffer);
     
     // Create UDP socket
     sock = socket(AF_INET, SOCK_DGRAM, 0);
     if (sock < 0) {
         perror("Socket creation failed");
         return EXIT_FAILURE;
     }
     
     // Set up server address
     memset(&server_addr, 0, sizeof(server_addr));
     server_addr.sin_family = AF_INET;
     server_addr.sin_port = htons(SERVER_PORT);
     
     // Convert IP address from text to binary
     if (inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr) <= 0) {
         perror("Invalid address");
         close(sock);
         return EXIT_FAILURE;
     }
     
     printf("Sending UDP datagram to %s:%d...\n", SERVER_ADDR, SERVER_PORT);
     
     // Send format string to server
     server_len = sizeof(server_addr);
     if (sendto(sock, format_buffer, strlen(format_buffer), 0,
               (struct sockaddr *)&server_addr, server_len) < 0) {
         perror("Send failed");
         close(sock);
         return EXIT_FAILURE;
     }
     
     printf("Format string sent. Waiting for response (timeout: %d ms)...\n", TIMEOUT_MS);
     
     // Set up poll for timeout
     pfd.fd = sock;
     pfd.events = POLLIN;
     
     // Wait for data or timeout
     int poll_result = poll(&pfd, 1, TIMEOUT_MS);
     
     if (poll_result < 0) {
         perror("Poll failed");
         close(sock);
         return EXIT_FAILURE;
     } else if (poll_result == 0) {
         fprintf(stderr, "Timeout waiting for server response\n");
         close(sock);
         return EXIT_FAILURE;
     }
     
     // Receive response from server
     memset(buffer, 0, BUFFER_SIZE);
     bytes_received = recvfrom(sock, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);
     if (bytes_received < 0) {
         perror("Receive failed");
         close(sock);
         return EXIT_FAILURE;
     }
     
     // Print the received response
     printf("\nResponse from server (%zd bytes):\n", bytes_received);
     printf("%s", buffer);
     
     // Close socket
     close(sock);
     return EXIT_SUCCESS;
 }