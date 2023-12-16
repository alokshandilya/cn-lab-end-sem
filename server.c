#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PORT 12345
#define MAX_BUFFER_SIZE 1024

int main() {
  int serverSocket, clientSocket;
  struct sockaddr_in serverAddr, clientAddr;
  socklen_t addrLen = sizeof(clientAddr);

  // Create socket
  serverSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (serverSocket == -1) {
    perror("Error creating socket");
    exit(EXIT_FAILURE);
  }

  // Setup server address structure
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(PORT);

  // Bind the socket
  if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      -1) {
    perror("Error binding socket");
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  // Listen for incoming connections
  if (listen(serverSocket, 5) == -1) {
    perror("Error listening for connections");
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  printf("Server is listening on port %d...\n", PORT);

  // Accept a connection
  clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
  if (clientSocket == -1) {
    perror("Error accepting connection");
    close(serverSocket);
    exit(EXIT_FAILURE);
  }

  // Receive file size from client
  unsigned long fileSize;
  recv(clientSocket, &fileSize, sizeof(fileSize), 0);

  printf("Receiving a file of size %lu bytes\n", fileSize);

  // Measure the start time
  clock_t startTime = clock();

  // Open a file for writing
  FILE *file = fopen("received_file.webm", "wb");
  if (file == NULL) {
    perror("Error opening file for writing");
    close(serverSocket);
    close(clientSocket);
    exit(EXIT_FAILURE);
  }

  // Receive and write the file data
  char buffer[MAX_BUFFER_SIZE];
  size_t totalBytesReceived = 0;
  while (totalBytesReceived < fileSize) {
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
      perror("Error receiving file data");
      fclose(file);
      close(serverSocket);
      close(clientSocket);
      exit(EXIT_FAILURE);
    }
    fwrite(buffer, 1, bytesRead, file);
    totalBytesReceived += bytesRead;
  }

  clock_t endTime = clock();
  double transferTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;

  printf("File received successfully in %.2f seconds\n", transferTime);

  // Close file and sockets
  fclose(file);
  close(clientSocket);
  close(serverSocket);

  return 0;
}
