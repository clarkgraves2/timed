# Timed - RFC 867 Daytime Protocol Server

## Overview

Timed is a C implementation of the RFC 867 Daytime Protocol with extended functionality to allow custom time formats. This server provides the current date and time to clients over both TCP and UDP connections on port 13 (by default).

The server responds to client requests as follows:

- If the client sends no data (empty payload), the server responds with the current time in a default format.
- If the client sends a format string, the server responds with the current time formatted according to that string.

Format strings should match `strftime()` requirements.

## Building the Project

### Prerequisites

- Linux environment (tested on Ubuntu 22.04 LTS)
- GCC compiler
- Make build system
- Check library (version 0.15 or later) for unit testing

# **IMPORTANT NOTE:** 
Since the server uses port 13, which is a privileged port (below 1024), you must run the server and all commands with sudo.
### Building

To build the standard version:

```bash
sudo make
```

To build a debug version with debugging symbols:

```bash
sudo make debug
```

To build a profile version with profiling symbols:

```bash
sudo make profile
```

All build commands will generate an executable named `timed` in the project's root directory.

### Cleaning

To clean all generated files:

```bash
sudo make clean
```

## Running the Server

Start the server by running:

```bash
sudo ./timed
```

The server will listen on port 13 for both TCP and UDP connections by default. The server logs its activity to `server.log` in the current directory. Because port 13 is a well-known port you must run as root.

To stop the server, use Ctrl+C (sends SIGINT).
## Testing

### Running Tests

The project includes a comprehensive test suite that can be run with:

```bash
sudo make check
```

This command will:

1. Build the server and test clients
2. Start the server in the background
3. Run TCP protocol tests
4. Run UDP protocol tests
5. Run UDP specific tests
6. Stop the server

To run tests with memory leak detection using Valgrind:

```bash
sudo make valgrind
```

### Test Utilities

The test directory includes several test clients and scripts:

- `test/bin/test_client` - TCP client for testing
- `test/bin/test_client_udp` - UDP client for testing

These clients can be used manually for troubleshooting:

```bash
echo "%Y-%m-%d %H:%M:%S" | ./test/bin/test_client
echo "%a, %d %b %Y %H:%M:%S %z" | ./test/bin/test_client_udp
```

## Interacting with the Server

### TCP Connection Examples

Using netcat to send a format string:

```bash
echo "%Y-%m-%d %H:%M:%S" | nc localhost 13
```

Using telnet for interactive connection:

```bash
telnet localhost 13
# Type format string and press Enter
```

### UDP Connection Examples

Using netcat in UDP mode:

```bash
echo "%I:%M:%S %p" | nc -u localhost 13
```

## Project Structure

- `src/` - Source code files
- `include/` - Header files
- `test/` - Test suite and test clients
- `doc/` - Documentation files
- `obj/` - Compiled object files (created during build)

## Implementation Details

The server is implemented with a modular architecture:

- `config.c/h` - Server configuration
- `server.c/h` - Server lifecycle management
- `socket.c/h` - TCP and UDP socket management
- `poll.c/h` - Socket polling and event handling
- `signal_handler.c/h` - Signal handling for graceful shutdown
- `syslog.c/h` - Thread-safe logging system
- `server_main.c` - Main entry point

## Security Considerations

- The server handles malformed input safely
- Format strings are limited to a reasonable size to prevent buffer overflow
- Input validation is performed to prevent format string vulnerabilities
- The server properly handles connection timeouts

## Known Limitations
- The server does not support IPv6
