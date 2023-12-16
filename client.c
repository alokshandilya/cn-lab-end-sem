#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PORT 12345
#define MAX_BUFFER_SIZE 1024

int main() {
  int clientSocket;
  struct sockaddr_in serverAddr;

  // Create socket
  clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (clientSocket == -1) {
    perror("Error creating socket");
    exit(EXIT_FAILURE);
  }

  // Setup server address structure
  memset(&serverAddr, 0, sizeof(serverAddr));
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr =
      inet_addr("127.0.0.1"); // Change this to the server's IP address
  serverAddr.sin_port = htons(PORT);

  // Connect to the server
  if (connect(clientSocket, (struct sockaddr *)&serverAddr,
              sizeof(serverAddr)) == -1) {
    perror("Error connecting to server");
    close(clientSocket);
    exit(EXIT_FAILURE);
  }

  // Open file for reading
  FILE *file = fopen("file.webm", "rb");
  if (file == NULL) {
    perror("Error opening file for reading");
    close(clientSocket);
    exit(EXIT_FAILURE);
  }

  // Get file size
  fseek(file, 0, SEEK_END);
  unsigned long fileSize = ftell(file);
  fseek(file, 0, SEEK_SET);

  // Send file size to the server
  send(clientSocket, &fileSize, sizeof(fileSize), 0);

  printf("Sending a file of size %lu bytes\n", fileSize);

  // Measure the start time
  clock_t startTime = clock();

  // Send file data
  char buffer[MAX_BUFFER_SIZE];
  size_t bytesRead;
  while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
    ssize_t bytesSent = send(clientSocket, buffer, bytesRead, 0);
    if (bytesSent <= 0) {
      perror("Error sending file data");
      fclose(file);
      close(clientSocket);
      exit(EXIT_FAILURE);
    }
  }

  // Measure the end time
  clock_t endTime = clock();
  double transferTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;

  printf("File sent successfully\n");

  // Close file and socket
  fclose(file);
  close(clientSocket);

  return 0;
}
