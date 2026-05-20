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
// noun, verb, adjective, adverb, weak words
    setColor(11); 
    printf("[1] ");
    setColor(7);
    printf("All Word\n");
    setColor(11); 
    printf("[2] ");
    setColor(7);
    printf("Noun Word\n");
    setColor(11); 
    printf("[3] ");
    setColor(7);
    printf("Verb Word\n");
    setColor(11); 
    printf("[4] ");
    setColor(7);
    printf("Adjective Word\n");
    setColor(11); 
    printf("[5] ");
    setColor(7);
    printf("Adverb Word\n");
    setColor(11); 
    printf("[6] ");
    setColor(7);
    printf("Weak Words\n");
    setColor(12);
    printf("[0] ");
    setColor(7);
    printf("Back\n");
    printf("Enter your choice: ");
    scanf("%d", &mode);
    getchar();
    if (mode == 0) {
            return;
        }
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
        clearScreen();
//Front card
        printHeader("FLASHCARD");

        printf("\n");
        printf("========================================\n");
        printf("\n");
        setColor(14);
        printf(">>> %s <<< \n",strupr(card->word));
        setColor(7);
        printf("\n");
        printf("========================================\n");
        printf("\nPress ENTER to flip...");
        getchar();
        // Back card
        clearScreen();
        printHeader("FLASHCARD ANSWER");
        printf("\n");
        printf("Meaning :%s  \n", card->meaning);
        printf("Type    :%s   \n",card->type);
        printf("Pronun  :%s  \n",card->pronunciation);
        printf("\n");
        printf("========================================\n");

        int review;
        printf("=================REVIEW==================\n");  
        printf("1. Again\n");
        printf("2. Good\n");
        printf("3. Easy\n");
        printf("Enter your choice: ");
        scanf("%d", &review);
        getchar();
        dailyMission.flashcardsReviewed++;
        playStats.exp += 10;
        updateLevel();
        if (review == 1) {
            card->wrongCount++;
            printf("\nMarked as difficult.\n");
        }
        else if (review == 3) {
            if( card -> learned == 0){
                card -> learned = 1;
                dailyMission.wordsLearnedToday++;
            }
            playStats.exp += 15;
            updateLevel();
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