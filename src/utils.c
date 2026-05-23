#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "../include/utils.h"
#include "../include/dictionary.h"

void clearScreen() {
    system("cls"); // Use "cls" on Windows
}

void pauseScreen() {
    system("pause"); // Use "pause" on Windows
}

void printHeader(char title[]) {
    printf("╔══════════════════════════════════════╗\n");
    printf("║ %-36s ║\n", title);
    printf("╚══════════════════════════════════════╝\n");
    setColor(7); // Reset to default color
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void showMiniPlayerCard(){
    int maxExp = playStats.level * 100;
    if(maxExp <= 0) { maxExp = 100;}
    int percent = (playStats.exp * 100) / maxExp;
    int bars = percent / 10;
    setColor(11); // màu xanh dương
    printf("+--------------------+\n");
    printf("| LV : %-13d |\n",playStats.level);
    printf("| EXP: %-13d |\n",playStats.exp);
    printf("| ");
for(int i = 0; i < 14; i++){
    if(i < bars){
        printf("■");
    }
    else {
        printf(" ");
    }
}
    printf(" %2d%% |\n",percent);
    printf("+--------------------+\n");
}


void showDictionaryMenu(){
    clearScreen();
    printHeader("DICTIONARY MENU");
    printf("\n");
    printf("========================================\n");
    printf("| [1] Show Dictionary                  |\n");
    printf("| [2] Search Word                      |\n");
    printf("| [3] Random Word                      |\n");
    printf("| [4] Add Word                         |\n");
    printf("| [5] Edit Word                        |\n");
    printf("| [6] Delete Word                      |\n");
    printf("| [0] Back                             |\n");
    printf("========================================\n");
}


void showWordCard(Word *word){
    printf("========================================\n");
    printf("| Word          : %-20s |\n",word->word);
    printf("| Meaning       : %-20s |\n",word->meaning);
    printf("| Pronunciation : %-20s |\n",word->pronunciation);
    printf("| Type          : %-20s |\n", word->type);
    printf("========================================\n");
}

void showGameMenu(){
    clearScreen();
    printHeader("GAME CENTER");
    printf("========================================\n");
    setColor(11); printf("[1] "); setColor(7);
    printf("English -> Vietnamese\n");
    setColor(11); printf("[2] "); setColor(7);
    printf("Vietnamese -> English\n");
    setColor(11); printf("[3] "); setColor(7);
    printf("Missing Letter\n");
    setColor(12); printf("[0] "); setColor(7);
    printf("Back\n");
    printf("========================================\n");
}

void printSuccess(char text[]){
    setColor(10);
    printf("[SUCCESS] %s\n", text);
    setColor(7);
}


void printError(char text[]){
    setColor(12);
    printf("[ERROR] %s\n", text);
    setColor(7);
}
