/*****************|
|** David Olsen **|
|**** dolsen *****|
|**** 641219 *****|
|*****************/
#include "client.h"

int
main(int argc, char *argv[]) {

  // check input
  char *hostname;
  int portno = -1;
  if (argc != 3) {
    fprintf(stderr,  "Usage: %s [host_name/IP_address] [port_number]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  hostname = argv[1];
  portno = atoi(argv[2]);
  if (portno == -1) {
    fprintf(stderr, "ERROR: [port_number] invalid.");
    exit(EXIT_FAILURE);
  }

  int sockfd, n, guessing = 1;
	struct hostent *hp;
	struct sockaddr_in serv_addr;
	char buffer[1024];

  // get the host
  hp = gethostbyname(hostname);
  if (!hp) {
    fprintf(stderr, "ERROR: unknown host \"%s\" \n", hostname);
    exit(EXIT_FAILURE);
  }

  // prepare socket parameters and buffer
  memset(&serv_addr, 0, sizeof(serv_addr));
  memset(buffer, 0, sizeof(buffer));
  memcpy((char *)&serv_addr.sin_addr, hp->h_addr_list[0], hp->h_length);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portno);

  // create the socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr,  "ERROR: could not create socket to server.\n");
    exit(EXIT_FAILURE);
  }

  // try to connect to socket
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    close(sockfd);
    fprintf(stderr, "ERROR: could not connect to the server.\n");
    exit(EXIT_FAILURE);
  }

  printf("Connected to server!\n");

  // read any new message from the server
  n = read(sockfd, buffer, sizeof(buffer));
  if (n < 0)	{
    fprintf(stderr, "ERROR: could not read from socket.\n");
    exit(EXIT_FAILURE);
  }
  fprintf(stdout, "%s", buffer);
  memset(buffer, 0, sizeof(buffer));


  while (guessing) {
    printf("> ");
    fgets(buffer, sizeof(buffer), stdin);
    if (buffer[0] != '\n') { // do nothing on empty input
      // send guess!
      n = write(sockfd, buffer, sizeof(buffer) - 1);
      if (n < 0)	{
        fprintf(stderr, "ERROR: could not write to socket.\n");
        exit(EXIT_FAILURE);
      }
      memset(buffer, 0, sizeof(buffer));

      // wait for answer!
      n = read(sockfd, buffer, sizeof(buffer));
      if (n < 0)	{
        fprintf(stderr, "ERROR: could not read from socket.\n");
        exit(EXIT_FAILURE);
      }
      // handle the response and check if server thinks we done
      if (handle_response(buffer) < 0) {
        guessing = 0;
      }
    }
    memset(buffer, 0, sizeof(buffer));
  }
  close(sockfd);
  return 0;
}

int
handle_response(char *response) {
  if (strlen(response) == 0) {
    print_conn_lost();
    return -1;
  } else if (strcmp(response, "SUCCESS") == 0) {
    print_success();
    return -1;
  } else if (strncmp(response, "FAILURE", 7) == 0) {
    char *correct_code = &response[8]; // response should be of form [FAILURE:CODE]
    print_failure(correct_code);
    return -1;
  } else if (strcmp(response, "INVALID") == 0) {
    print_invalid();
    return 0;
  }
  char correct = response[1]; // assuming response with format:
  char wrong_pos = response[3];  // [b:m]
  print_hint(correct, wrong_pos);
  return 0;
}

void
print_success(void) {
  printf("----------------------------------------------------------------\n");
  printf("                           YOU WON!\n");
  printf("                      You're a MASTERMIND!\n");
  printf("----------------------------------------------------------------\n");
}
void
print_failure(char *code) {
  printf("----------------------------------------------------------------\n");
  printf("                          GAME OVER!\n");
  printf("                   The correct code was %s\n", code);
  printf("----------------------------------------------------------------\n");
}
void
print_invalid(void) {
  printf("----------------------------------------------------------------\n");
  printf("     Oops! A valid guess is 4 chars in the range [ABCDEF].      \n");
  printf("        P.S. - that counted as a guess so be careful!\n");
  printf("----------------------------------------------------------------\n");
}
void
print_hint(char correct, char wrong_pos) {
  printf("----------------------------------------------------------------\n");
  printf("                 Chars in correct position:  %c\n", correct);
  printf("                  Chars in wrong position:   %c\n", wrong_pos);
  printf("                         Try again.\n");
  printf("----------------------------------------------------------------\n");
}
void
print_conn_lost(void) {
  printf("----------------------------------------------------------------\n");
  printf("                 Connection to server lost.\n");
  printf("----------------------------------------------------------------\n");
}
