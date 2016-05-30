/*****************|
|** David Olsen **|
|**** dolsen *****|
|**** 641219 *****|
|*****************/
#include "mastermind.h"

char *
mastermind_get_random_code(void) {
  char *code = malloc(5 * sizeof(char));
  if (code != NULL) {
    for (int i=0; i<4; i++) {
      code[i] = LETTERS[(rand() % strlen(LETTERS))];
    }
    code[4] = '\0';

  }
  return code;
}

char *
mastermind_check_guess(char *guess, char *secret_code) {
  int correct_chars = 0, correct_positions = 0;
  char *response = malloc(64 * sizeof(char));
  // check for correct code
  if (strcmp(guess, secret_code) == 0) {
    strcpy(response, "SUCCESS");
  } else {
    int flags[strlen(guess)]; // for marking correct positions
    for (int i=0; i<strlen(guess); i++) {
      flags[i] = 0;
    }
    // correct position
    for (int i=0; i<strlen(guess); i++) {
      if (guess[i] == secret_code[i]) {
        correct_positions++;
        flags[i] = 1;
      }
    }
    // correct char wrong position
    for (int i=0; i<strlen(secret_code); i++) {
      if (flags[i] == 0) { // code position is not correct or has been checked
        for (int j=0; j<strlen(guess); j++) {
          if (secret_code[i] == guess[j] && flags[j] == 0) {
            correct_chars++;
            flags[j] = 1; // don't count duplicates!
            break;
          }
        }
      }
    }
    // build hint
    response[0] = '[';
    response[1] = '0' + correct_positions;
    response[2] = ':';
    response[3] = '0' + correct_chars;
    response[4] = ']';
    response[5] = '\0';
  }
  return response;
}

int
mastermind_validate_guess(char *guess) {
  int valid = 1;
  if (strlen(guess) != 4) {
    valid = 0;
  } else {
    // check all chars of guess are in accepted LETTERS
    for (int i=0; i<4; i++ ) {
      int matched = 0;
      for (int j=0; j<strlen(LETTERS); j++) {
        if (guess[i] == LETTERS[j]) {
          matched = 1;
        }
      }
      if (!matched) {
        valid = 0;
        break;
      }
    }
  }
  return valid;
}
