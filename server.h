/*****************|
|** David Olsen **|
|**** dolsen *****|
|**** 641219 *****|
|*****************/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <stdarg.h>
#include <sys/resource.h>


#define LOGFILENAME	"log.txt"

#define MAX_CONNECTIONS 1
#define MAX_GUESSES 10

typedef struct thread_info {
    pthread_t tid; // kernel assigned thread id
    int sockfd; // the client's socket fd
    struct sockaddr_in cli_addr; // client details
    socklen_t clilen; // size of client
    int guess_count; // clien't guesses
    int running; // is the game in progress?
    int completed; // has / was the game completed?
    int won; // did the client win?
    char *secret_code; // the client's secret code!
} thread_info;

thread_info *create_thread_info(int connfd, struct sockaddr_in cli_addr, socklen_t clilen, char *secret_code); // create a thread info struct
void interrupt_handler(int signal); // catches SIGINT
void *game_handler(void *param); // the actions to be completed each game
void log_event(struct thread_info *t, const char *event, ... ); // log the event to file (thread safe)
void save_runtime_stats(void); // save runtime stats to the log (and close all threads)
void save_thread_stats(struct thread_info *t); // save certain stats about a thread
void save_rusage_stats(struct rusage *rusage); // save rusage stats to the log
void print_welcome_to(char *buffer); // print the welcome message to the buffer
