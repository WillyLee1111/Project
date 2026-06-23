#ifndef UTILS_H
#define UTILS_H

#include "dictionary.h"
#include <conio.h>

#define ARROW "\xE2\x86\x92"

int selectMenu(char *title, char *options[], int numOptions, void (*printHeaderCallback)(void));
void clearScreen();
void resetCursor();
void pauseScreen();
void printHeader(char title[]);
void setColor(int color);
void showMiniPlayerCard();
void showWordCard(Word *word);
void showWordCardBoxed(Word *word);
void printSuccess(char *text);
void printError(char *text);
void displayDictionaryBoxed(HashTable *ht);
int getVisualWidth(const char *str);

#endif // UTILS_H
