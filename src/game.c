#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "../include/game.h"
#include "../include/dictionary.h"
#include "../include/utils.h"

void playMissingLetterGame(HashTable *ht){
    dailyMission.gamesPlayed++;
    int choice = 1;
    while (choice != 2){
        clearScreen();
        Word* word = getAdaptiveWord(ht);
        if (word == NULL) {
            printf("No words available.\n");
            pauseScreen();
            return;
        }
        char hidden[50];
        strncpy(hidden, word->word, sizeof(hidden) - 1);
        hidden[sizeof(hidden) - 1] = '\0';
        int randomIndex = rand() % strlen(word->word);
        char missingLetter = hidden[randomIndex];
        hidden[randomIndex] = '_';
        printf("===============MISSING LETTER GAME==================\n");
        printf("Word: %s\n", hidden);   
        char answer[50];
        printf("Enter the missing letter: ");
        scanf("%49s", answer);
        
        char formedWord[50];
        strcpy(formedWord, hidden);
        formedWord[randomIndex] = answer[0];
        
        Word* foundWord = searchWord(ht, formedWord);
        
        if (foundWord != NULL) {
            printf("Correct! The word is %s.\n", foundWord->word);
            foundWord->learned = 1;
            printf("Meaning: %s\n", foundWord->meaning);
            printf("Pronunciation: %s\n", foundWord->pronunciation);
            playStats.exp += 20;
            updateLevel();
        } else {
            printf("Không có từ này trong tiếng Anh\n");
            printf("Wrong! The correct letter was '%c'. The word we thought of was %s.\n", missingLetter, word->word);
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

void englishToVietnameseGame(HashTable *ht){
    clearScreen();
    dailyMission.gamesPlayed++;
    Word *word = getRandomWord(ht);
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
    if (strstr(answer, word->meaning) != NULL) {
        printf("\nCorrect!\n");
        playStats.exp += 20;
    }
    else {
        printf("\nWrong answer.\n");
        printf("Correct meaning: %s\n", word->meaning);
    }
    pauseScreen();
}


void vietnameseToEnglishGame(HashTable *ht){
    clearScreen();
    dailyMission.gamesPlayed++;
    Word *word = getRandomWord(ht);
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
    }
    else {
        printf("\nWrong answer.\n");
        printf("Correct word: %s\n", word->word);
    }
    pauseScreen();
}
