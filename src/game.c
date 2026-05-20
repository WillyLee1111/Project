#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "../include/game.h"
#include "../include/dictionary.h"
#include "../include/utils.h"

void playMissingLetterGame(Word *head){
    dailyMission.gamesPlayed++;
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
            playStats.exp += 20;
            updateLevel();
        } else {
            printf("Wrong! The correct letter was '%c'. The word is %s.\n", missingLetter, word->word);
            word->wrongCount++;
            updateLevel();
        }
        printf("\n");
        printf("1. Play Again\n");
        printf("2. Exit Game\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        pauseScreen();
    }
}

void englishToVietnameseGame(Word *head){
    clearScreen();
    dailyMission.gamesPlayed++;
    Word *word = getRandomWord(head);
    if(word == NULL){
        printf("No words available.\n");
        pauseScreen();
        return;
    }
    char answer[100];
    printf("=========== ENGLISH -> VIETNAMESE ===========\n\n");
    printf("Translate this word:\n");
    printf("%s\n\n", word->word);
    printf("Your answer: ");
    getchar();
    fgets(answer, sizeof(answer), stdin);
    answer[strcspn(answer, "\n")] = '\0';
    if(strstr(answer, word->meaning) == 0){
        printf("\nCorrect!\n");
        playStats.exp += 20;
        dailyMission.gamesPlayed++;
    }
    else {
        printf("\nWrong answer.\n");
        printf("Correct meaning: %s\n", word->meaning);
    }
    pauseScreen();
}


void vietnameseToEnglishGame(Word *head){
    clearScreen();
    dailyMission.gamesPlayed++;
    Word *word = getRandomWord(head);
    if(word == NULL){
        printf("No words available.\n");
        pauseScreen();
        return;
    }
    char answer[100];
    printf("=========== VIETNAMESE -> ENGLISH ===========\n\n");
    printf("Translate this meaning:\n");
    printf("%s\n\n", word->meaning);
    printf("Your answer: ");
    getchar();
    fgets(answer, sizeof(answer), stdin);
    answer[strcspn(answer, "\n")] = '\0';
    if(_stricmp(answer, word->word) == 0){
        printf("\nCorrect!\n");
        playStats.exp += 20;
        dailyMission.gamesPlayed++;
    }
    else {
        printf("\nWrong answer.\n");
        printf("Correct word: %s\n", word->word);
    }
    pauseScreen();
}
