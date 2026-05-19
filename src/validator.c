#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/validator.h"

int isDuplicateWord(Word *head, char *word) {
    Word *temp = head;
    while (temp != NULL) {
        if (strcmp(temp->word, word) == 0) {
            return 1; // Duplicate found
        }
        temp = temp->next;
    }
    return 0; // No duplicate
}

int isValidUsername(char *username) {
    if (strlen(username) < 3 || strlen(username) > 50) {
        return 0; // Invalid length
    }
    for (int i = 0; username[i] != '\0'; i++) {
        if (!isalnum(username[i]) && username[i] != '_') {
            return 0; // Invalid character found
        }
    }
    return 1; // Valid username
}

int isStrongPassword(char *password) {
    if (strlen(password) < 6) {
        return 0; // Too short
    }
    int hasUpper = 0, hasLower = 0, hasDigit = 0;
    for (int i = 0; password[i] != '\0'; i++) {
        if (isupper(password[i])) hasUpper = 1;
        else if (islower(password[i])) hasLower = 1;
        else if (isdigit(password[i])) hasDigit = 1;
    }
    return hasUpper && hasLower && hasDigit; // Must have all three
}