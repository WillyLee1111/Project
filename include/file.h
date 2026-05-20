#ifndef FILE_H
#define FILE_H

#include "dictionary.h"
void loadDictionary(Word **head);
void saveUserData();
void loadUserData();
int login();
int registerUser();
void updateStudyStreak();
#endif // FILE_H