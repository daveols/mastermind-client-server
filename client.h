/*****************|
|** David Olsen **|
|**** dolsen *****|
|**** 641219 *****|
|*****************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int handle_response(char *response);// do stuff based on the given response from the server, return -1 for failure otherwise 0
// print out human readable server responses
void print_success(void);
void print_failure(char *code);
void print_invalid(void);
void print_hint(char correct, char wrong_pos);
void print_conn_lost(void);
