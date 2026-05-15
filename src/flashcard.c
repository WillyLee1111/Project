#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "../include/flashcard.h"
#include "../include/dictionary.h"
#include "../include/utils.h"

void flashcardMode(Word *head){

    int mode;
    int choice = 1;

    clearScreen();

    printf("===============FLASHCARD MODE==================\n");

    printf("1. All Words\n");
    printf("2. Noun\n");
    printf("3. Verb\n");
    printf("4. Adjective\n");
    printf("5. Adverb\n");
    printf("6. Weak Words\n");
    printf("Enter your choice: ");
    scanf("%d", &mode);
    getchar();
    do {
        clearScreen();
        Word* card = NULL;
        if (mode == 1) {
            card = getAdaptiveWord(head);
        }
        else if (mode == 2) {
            card = getWordByType(
                head,
                "noun"
            );
        }
        else if (mode == 3) {
            card = getWordByType(
                head,
                "verb"
            );
        }
        else if (mode == 4) {
            card = getWordByType(
                head,
                "adjective"
            );
        }
        else if (mode == 5) {
            card = getWordByType(
                head,
                "adverb"
            );
        }
        else if (mode == 6) {
            card = getWeakWord(head);
        }
        if (card == NULL) {
            printf("No words found.\n");
            pauseScreen();
            return;
        }
        printf("===============FLASHCARD==================\n");
        printf("Word: %s\n", card->word);
        printf("Press Enter to flip the card...");
        getchar();
        clearScreen();
        printf("===============BACK OF FLASHCARD==================\n");
        printf("Meaning: %s\n", card->meaning);
        printf("Pronunciation: %s\n", card->pronunciation);
        printf("Type: %s\n", card->type);
        printf("\n");
        int review;
        printf("=================REVIEW==================\n");
        printf("1. Again\n");
        printf("2. Good\n");
        printf("3. Easy\n");
        printf("Enter your choice: ");
        scanf("%d", &review);
        getchar();
        dailyMission.flashcardsReviewed++;
        if (review == 1) {
            card->wrongCount++;
            printf("\nMarked as difficult.\n");
        }
        else if (review == 3) {
            card->learned = 1;
            dailyMission.wordsLearnedToday++;
            printf("\nMarked as learned.\n");
        }
        int nextchoice;
        printf("\n");
        printf("1. Next Flashcard\n");
        printf("2. Exit Flashcard Mode\n");
        printf("Enter your choice: ");
        scanf("%d", &nextchoice);
        getchar();
        choice = nextchoice;
    }
    while (choice != 2);
}