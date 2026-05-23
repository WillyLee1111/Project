#ifndef FILE_H
#define FILE_H

#include "dictionary.h"
void loadDictionary(HashTable *ht);
void saveUserData();
void loadUserData();
int login();
int registerUser();
void updateStudyStreak();
#endif // FILE_H