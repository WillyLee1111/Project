#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/flashcard.h"
#include "../include/dictionary.h"
#include "../include/utils.h"

void flashcardMode(HashTable *ht) {
    int mode;

    do {
        int choice = 1;

        clearScreen();
        printf("=============================================\n");
        setColor(14); printf("              FLASHCARD MODE\n"); setColor(7);
        printf("=============================================\n\n");

        setColor(11); printf("  [1] "); setColor(7); printf("Adaptive (Spaced Repetition)\n");
        setColor(11); printf("  [2] "); setColor(7); printf("Nouns Only\n");
        setColor(11); printf("  [3] "); setColor(7); printf("Verbs Only\n");
        setColor(11); printf("  [4] "); setColor(7); printf("Adjectives Only\n");
        setColor(11); printf("  [5] "); setColor(7); printf("Adverbs Only\n");
        setColor(11); printf("  [6] "); setColor(7); printf("Weak Words Only\n");
        setColor(12); printf("  [0] "); setColor(7); printf("Back to Main Menu\n");

        printf("\nEnter your choice: ");
        scanf("%d", &mode);
        getchar();

        if (mode == 0) return;

        do {
            clearScreen();
            Word* card = NULL;
            if (mode == 1) {
                card = getAdaptiveWord(ht);
            } else if (mode == 2) {
                card = getWordByType(ht, "noun");
            } else if (mode == 3) {
                card = getWordByType(ht, "verb");
            } else if (mode == 4) {
                card = getWordByType(ht, "adjective");
            } else if (mode == 5) {
                card = getWordByType(ht, "adverb");
            } else if (mode == 6) {
                card = getWeakWord(ht);
            }

            if (card == NULL) {
                setColor(12); printf("\n  No words found for this mode.\n"); setColor(7);
                pauseScreen();
                break; // Back to mode selection menu
            }

            // --- FRONT OF FLASHCARD ---
            clearScreen();
            printf("=============================================\n");
            setColor(14); printf("              FLASHCARD (FRONT)\n"); setColor(7);
            printf("=============================================\n\n");

            setColor(11);
            printf("           >>> %s <<< \n\n", strupr(card->word));
            setColor(7);
            _strlwr(card->word);

            printf("=============================================\n");
            printf("Press ENTER to flip...");
            getchar();

            // --- BACK OF FLASHCARD ---
            clearScreen();
            printf("=============================================\n");
            setColor(14); printf("               FLASHCARD (BACK)\n"); setColor(7);
            printf("=============================================\n\n");

            printf("Word    : "); setColor(11); printf("%s\n", card->word); setColor(7);
            printf("Meaning :\n");
            printWordMeanings(card->meaning);
            printf("Type    : %s\n", card->type);
            printf("Pronun  : %s\n", card->pronunciation);
            printf("\n=============================================\n\n");

            // --- REVIEW SYSTEM (Spaced Repetition Simulation) ---
            int review;
            printf("How well did you remember this word?\n\n");
            setColor(12); printf("  [1] "); setColor(7); printf("Again (Forgot it completely)\n");
            setColor(14); printf("  [2] "); setColor(7); printf("Good  (Remembered with effort)\n");
            setColor(10); printf("  [3] "); setColor(7); printf("Easy  (Remembered instantly)\n");
            printf("\nEnter your choice: ");
            scanf("%d", &review);
            getchar();

            dailyMission.flashcardsReviewed++;

            if (review == 1) {
                card->wrongCount += 2;
                card->learned = 0;
                setColor(12); printf("\n-> Marked as difficult. You will see it more often.\n"); setColor(7);
            } else if (review == 2) {
                if (card->wrongCount > 0) card->wrongCount--;
                playStats.exp += 10;
                updateLevel();
                setColor(14); printf("\n-> Good job! Keep practicing.\n"); setColor(7);
            } else if (review == 3) {
                if (card->learned == 0) {
                    card->learned = 1;
                    dailyMission.wordsLearnedToday++;
                }
                card->wrongCount = 0;
                playStats.exp += 15;
                updateLevel();
                setColor(10); printf("\n-> Marked as fully learned!\n"); setColor(7);
            }

            printf("\n---------------------------------------------\n");
            setColor(11); printf("  [1] "); setColor(7); printf("Next Flashcard\n");
            setColor(12); printf("  [2] "); setColor(7); printf("Back to Flashcard Menu\n");
            printf("Choose: ");
            scanf("%d", &choice);
            getchar();

        } while (choice != 2);

    } while (1); // Always return to mode selection after a session ends
}