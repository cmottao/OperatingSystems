# Operating Systems

Practices developed during the "Operating Systems" course. Its purpose is to organize and document the projects where theoretical concepts were applied in a practical way, including process communication, file handling, concurrency, synchronization, and client-server system design.

## Practice 1

This practice implements a client-server system that allows users to search and retrieve information about traffic fines using a dataset inspired by Colombian government records. Communication between the client and the server is performed through Inter-Process Communication (IPC) mechanisms, and efficient searches are achieved by indexing the data files using hash-based techniques.

The server is responsible for managing access to the data and responding to client requests, while the client provides an interface for sending queries and receiving results. Before running the system, an index is created to speed up record lookups and avoid full sequential scans of the data files.

### Project Structure

| File | Description |
|------|-------------|
| `server.c` | Server implementation that handles client requests and manages data access. |
| `client.c` | Client interface for sending queries and receiving results from the server. |
| `create_index.c` | Utility to create hash-based indices over data files for optimized searches. |
| `utils.h` | Helper functions and common definitions shared across the project. |
| `Makefile` | Build configuration for compiling all project executables. |

### Building

To compile, run:

```bash
make
```

This will generate the executables needed for the server, client, and index creation utility.
