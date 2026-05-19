#ifndef VALIDOR_H
#define VALIDOR_H

#include "dictionary.h"


int isValidUsername(char *username);
int isStrongPassword(char *password);
int isDuplicateWord(Word *head, char *word);

#endif