#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "../include/game.h"
#include "../include/dictionary.h"
#include "../include/utils.h"

void playMissingLetterGame(Word *head){
    int choice = 1;
    while (choice != 2){
        clearScreen();
        Word* word = getAdaptiveWord(head);
        char hidden[50];
        strncpy(hidden, word->word, sizeof(hidden) - 1);
        int randomIndex = rand() % strlen(word->word);
        char missingLetter = hidden[randomIndex];
        hidden[randomIndex] = '_';
        printf("===============MISSING LETTER GAME==================\n");
        printf("Word: %s\n", hidden);   
        char answer[50];
        printf("Enter the missing letter: ");
        scanf("%s", answer);
        if (answer[0] == missingLetter) {
            printf("Correct! The word is %s.\n", word->word);
            word->learned = 1;
            printf("Meaning: %s\n", word->meaning);
            printf("Pronunciation: %s\n", word->pronunciation);
        } else {
            printf("Wrong! The correct letter was '%c'. The word is %s.\n", missingLetter, word->word);
            word->wrongCount++;
        }
        printf("\n");
        printf("1. Play Again\n");
        printf("2. Exit Game\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        pauseScreen();
    }
}