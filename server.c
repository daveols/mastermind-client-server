/*****************|
|** David Olsen **|
|**** dolsen *****|
|**** 641219 *****|
|*****************/
#include "server.h"
#include "queue.h"
#include "mastermind.h"

static FILE *logfile;
static node *threads; // hold thread data
static int connection_count = 0;  // count total connections to server

static struct rusage proc_rusage;

static pthread_mutex_t logging_lock;

int
main(int argc, char *argv[]) {

  // check input
  int portno = -1;
  char *secret_code;; // the server's secret code
  if (argc != 2 && argc != 3) {
    fprintf(stderr,  "Usage: %s [port_number] [default_secret_code]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  // set port number
  portno = atoi(argv[1]);
  if (portno == -1) {
    fprintf(stderr, "ERRPR: [port_number] invalid.\n");
    exit(EXIT_FAILURE);
  }

  // set the code
  if (argc == 3) {
    if (mastermind_validate_guess(argv[2])) {
      secret_code = strdup(argv[2]);
    } else {
      fprintf(stderr, "ERROR: [default_secret_code] must be four chars from { A B C D E F }.\n");
      exit(EXIT_FAILURE);
    }
  } else {
    // create seed for random codes
    srand(time(NULL));
    secret_code = NULL;
  }

  // open/create log file
  logfile = fopen(LOGFILENAME, "w");
  if (logfile == NULL) {
    fprintf(stderr, "ERROR: could not create log file.\n");
    exit(EXIT_FAILURE);
  }
  log_event(NULL, "server started");
  fprintf(stdout, "Server running...\n");

  int listenfd;
  struct sockaddr_in serv_addr;

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(portno);

  // create the socket
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) {
    fprintf(stderr, "ERROR: could not create TCP socket. Shutting down.\n");
    exit(EXIT_FAILURE);
  }

  // bind the address to the socket
  if(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
    fprintf(stderr, "ERROR: could not bind socket to port %d. Shutting down.\n", portno);
    exit(EXIT_FAILURE);
  }

  // listen to the socket with X channels
  listen(listenfd, MAX_CONNECTIONS);

  // listen for process-close as well!
  signal(SIGINT, interrupt_handler);

  // intermediate client vars
  int connfd;
  struct sockaddr_in cli_addr;
  socklen_t clilen;
  clilen = sizeof(cli_addr);

  // listen for connections
  while ((connfd = accept(listenfd, (struct sockaddr*)&cli_addr, &clilen))) {
    if (connfd < 0) {
      log_event(NULL, "ERROR: a connection failed. Shutting down.");
      exit(EXIT_FAILURE);
    }

    // create thread info struct
    thread_info *t;
    t = create_thread_info(connfd, cli_addr, clilen, secret_code);
    // save the thread info pointer globally
    threads = queue_add(threads, t);
    // try to create thread for connection
    if (pthread_create(&t->tid, NULL, game_handler, (void *)t) < 0) {
      log_event(NULL, "ERROR: failed to create a thread. Shutting down.");
      exit(EXIT_FAILURE);
    } else {
      t->running = 1;
      connection_count++;
    }
  }
  return 0;
}

void *
game_handler(void *param) {
  thread_info *t = (thread_info *)param;

  int n;
  char buffer[1024], response[1024];
  memset(buffer, 0, sizeof(buffer));
  memset(response, 0, sizeof(response));

  log_event(t, "client connected", t->sockfd);
  log_event(t, "server secret = %s.", t->secret_code);
  print_welcome_to(buffer);
  n = write(t->sockfd, buffer, sizeof(buffer) - 1);
  if (n < 0)	{
    log_event(t, "ERROR: writing to socket. Closing socket and thread.");
    close(t->sockfd);
    pthread_exit(NULL);
  }

  // IMPLEMENT TIMEOUT OR SOMETHING?
  while (!t->completed) {
    // read guesses from the client
    n = read(t->sockfd, buffer, sizeof(buffer) - 1);
    if (n < 0)	{
      log_event(t, "ERROR: reading from socket. Closing socket and thread.");
      close(t->sockfd);
      pthread_exit(NULL);
    }
    if (n > 0) {
      t->guess_count++;
      // clean the fgets newline
      char *pos;
      if ((pos = strchr(buffer, '\n')) != NULL) {
          *pos = '\0';
      }
      log_event(t, "client's guess =  %s", buffer);
      if (mastermind_validate_guess(buffer)) {
        char *temp = mastermind_check_guess(buffer, t->secret_code);
        strncpy(response, temp, strlen(temp));
        free(temp);
        memset(buffer, 0, sizeof(buffer)); // clear buffer
        if (strcmp(response, "SUCCESS") == 0) {
          // winner winner chicken dinner!
          snprintf(buffer, sizeof(buffer), "SUCCESS");
          t->completed = 1;
          t->won = 1;
        } else {
          // send the hint
          snprintf(buffer, sizeof(buffer), "%s", response);
          log_event(t, "server's hint =  %s", response);
        }
      } else {
        // invalid guess
        snprintf(buffer, sizeof(buffer), "INVALID");
        log_event(t, "client's guess is INVALID");
      }
      if (!t->won && t->guess_count == MAX_GUESSES) {
        // last guess, failed!
        snprintf(buffer, sizeof(buffer), "FAILURE:%s", t->secret_code);
        t->completed = 1;
      }
      // send the response
      n = write(t->sockfd, buffer, sizeof(buffer) - 1);
      if (n < 0)	{
        log_event(t, "ERROR: writing to socket. Closing socket and thread.");
        close(t->sockfd);
        pthread_exit(NULL);
      }
      memset(buffer, 0, sizeof(buffer)); // clear buffer
      memset(response, 0, sizeof(response)); // clear buffer
    }
  }
  if (t->won)
    log_event(t, "SUCCESS game over");
  else
    log_event(t, "FAILURE game over");

  close(t->sockfd);
  t->running = 0;
  return 0;
}

void
interrupt_handler(int signal) {
  fprintf(stdout, "\nSaving runtime stats to log...\n");

  log_event(NULL, "server stopped");
  getrusage(RUSAGE_SELF, &proc_rusage);
  save_runtime_stats();

  fclose(logfile);
  fprintf(stdout, "Done!\n");

  exit(EXIT_SUCCESS);
}

thread_info *
create_thread_info(int connfd, struct sockaddr_in cli_addr, socklen_t clilen, char *secret_code) {
  thread_info *t;
  t = (thread_info *)malloc(sizeof(thread_info));

  t->sockfd = connfd;
  t->cli_addr = cli_addr;
  t->clilen = clilen;
  t->guess_count = 0;
  t->running = 0;
  t->completed = 0;
  t->won = 0;

  if (secret_code == NULL) {
    // generate secret code just for this thread!
    char *temp = mastermind_get_random_code();
    t->secret_code = strdup(temp);
    free(temp);
  } else {
    // use provided secret code
    t->secret_code = strdup(secret_code);
  }
  return t;
}

void
log_event(struct thread_info *t, const char* event, ...) {
  pthread_mutex_lock(&logging_lock);

  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);

  fprintf(logfile, "[%d %d %d %d:%d:%d]", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900, timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  if (t != NULL)
    fprintf(logfile, "(%s)(socket %d) ", inet_ntoa(t->cli_addr.sin_addr), t->sockfd);
  else {
    fprintf(logfile, "(0.0.0.0) ");
  }

  // jam in formats args
  va_list args;
  va_start(args, event);
  vfprintf(logfile, event, args);
  va_end(args);
  fprintf(logfile, "\n");

  pthread_mutex_unlock(&logging_lock);
}

void
save_runtime_stats(void) {
  pthread_mutex_lock(&logging_lock);

  int  won = 0, interrupted = 0, completed = 0, guess_count = 0;
  float avg_guesses_to_win = 0;

  thread_info *t;
  while(((t = (thread_info *)queue_pop(&threads)) != NULL)) {
    // no locks needed as threads are killed before accessing data:
    if (t->running) pthread_cancel(t->tid);
    if (t->won) guess_count += t->guess_count;
    won += t->won;
    completed += t->completed;
    interrupted += t->running;
    fprintf(logfile,"----------------------------------------------------------------\n");
    save_thread_stats(t);
  }
  fprintf(logfile, "----------------------------------------------------------------\n");
  fprintf(logfile, "ENTIRE PROCESS RUSAGE (pid=%d):\n", getpid());
  save_rusage_stats(&proc_rusage);
  fprintf(logfile, "\nApproximate thread mem usage (KB): %d\n", (int)(((sizeof(thread_info) + 2048) * connection_count) / 1000));
  fprintf(logfile, "((sizeof(thread_info) + buffers) * connection_count)\n");

  avg_guesses_to_win = won > 0 ? ((float) guess_count / won) : 0;
  fprintf(logfile, "----------------------------------------------------------------\n");
  fprintf(logfile, "BONUS STATS:\n");
  fprintf(logfile, "%d games won, %d games completed, %d games interrupted.\n", won, completed, interrupted);
  fprintf(logfile, "Average guesses to win:              %f\n", avg_guesses_to_win);

  pthread_mutex_unlock(&logging_lock);
}

void
save_thread_stats(struct thread_info *t) {
  fprintf(logfile, "THREAD tid = %d\n", (unsigned int)t->tid);
  fprintf(logfile, "Client IP:   %s\n", inet_ntoa(t->cli_addr.sin_addr));
  fprintf(logfile, "Secret code: %s\n", t->secret_code);
  fprintf(logfile, "Completed:   %d\n", t->completed);
  fprintf(logfile, "Guesses:     %d\n", t->guess_count);
  fprintf(logfile, "Won:         %d\n", t->won);
}

void
save_rusage_stats(struct rusage *rusage) {
  struct timeval *tvu, *tvs;
  tvu = &rusage->ru_utime;
  tvs = &rusage->ru_stime;
  fprintf(logfile, "User CPU time:                %d.%ds\n", (int)tvu->tv_sec,  (int)tvu->tv_usec);
  fprintf(logfile, "System CPU time:              %d.%ds\n", (int)tvs->tv_sec,  (int)tvs->tv_usec);
  fprintf(logfile, "Page faults (soft):           %ld\n", rusage->ru_minflt);
  fprintf(logfile, "Page faults (hard):           %ld\n", rusage->ru_majflt);
}

void
print_welcome_to(char *buffer) {
  strcat(buffer, "----------------------------------------------------------------\n");
  strcat(buffer, "  Welcome to Dave's masterfully implemented Mastermind server!  \n");
  strcat(buffer, "\n");
  strcat(buffer, " RULES:\n");
  strcat(buffer, " The aim of the game is to guess a SECRET CODE!\n");
  strcat(buffer, " The code is four of the following characters: A B C D E F\n");
  strcat(buffer, " Duplicates are possible!\n");
  strcat(buffer, " You will have 10 guesses at the code before it is game over.\n");
  strcat(buffer, " Send your guesses in the form ABCD.\n");
  strcat(buffer, "\n");
  strcat(buffer, " Good luck!\n");
  strcat(buffer, "----------------------------------------------------------------\n");
}
