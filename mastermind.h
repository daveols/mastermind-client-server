/*****************|
|** David Olsen **|
|**** dolsen *****|
|**** 641219 *****|
|*****************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define LETTERS "ABCDEF"

char *mastermind_get_random_code(void); // get a random 4 letter code
int mastermind_validate_guess(char *guess); // check if given string is a valid guess: 0 if invalid, otherwise 1
char *mastermind_check_guess(char *guess, char *secret_code); // process a mastermind guess given its secret code and return a string reply
