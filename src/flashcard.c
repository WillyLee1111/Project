#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/flashcard.h"
#include "../include/dictionary.h"
#include "../include/utils.h"

static Word* currentFlashcard = NULL;
static char currentReviewResult[200] = "";
static int currentReviewColor = 7;

//in ra giao diện mặt sau của flashcard
//bổ trợ vẽ mặt sau
static void printFlashcardBack() {
    printf("=============================================\n");
    setColor(14); printf("               FLASHCARD (BACK)\n"); setColor(7);
    printf("=============================================\n\n");

    printf("Word    : "); setColor(11); printf("%s\n", currentFlashcard->word); setColor(7);
    printf("Meaning :\n");
    printWordMeanings(currentFlashcard->meaning);
    printf("Type    : %s\n", currentFlashcard->type);
    printf("Pronun  : %s\n", currentFlashcard->pronunciation);
    printf("\n=============================================\n");
    printf("How well did you remember this word?\n");
}
// gọi printFlashCardBack và in thêm thông báo kết quả đánh giá
//hiển thị thông báo phản hồi chuyển tiếp trước khi người dùng nhấn chuyển sang thẻ học tiếp theo.
static void printNextMenuHeader() {
    printFlashcardBack();
    setColor(currentReviewColor);
    printf("\n%s\n", currentReviewResult);
    setColor(7);
}
//quản lý vòng lặp ở chế độ học thẻ
//giúp ng dùng ghi nhớ
void flashcardMode(HashTable *ht) {
    int mode;

    do {
        int choice = 1;
        char *flashcardOptions[] = {
            "Adaptive (Spaced Repetition)",
            "Nouns Only",
            "Verbs Only",
            "Adjectives Only",
            "Adverbs Only",
            "Weak Words Only",
            "Back to Main Menu"
        };
        mode = selectMenu("FLASHCARD MODE", flashcardOptions, 7, NULL);
        if (mode == 6) mode = 0;
        else mode += 1;

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
            currentFlashcard = card;
            char *reviewOptions[] = {
                "Again (Forgot it completely)",
                "Good  (Remembered with effort)",
                "Easy  (Remembered instantly)"
            };
            int review = selectMenu("", reviewOptions, 3, printFlashcardBack);
            review += 1;

            dailyMission.flashcardsReviewed++;

            if (review == 1) {
                card->wrongCount += 2;
                card->learned = 0;
                currentReviewColor = 12;
                strcpy(currentReviewResult, "-> Marked as difficult. You will see it more often.");
            } else if (review == 2) {
                if (card->wrongCount > 0) card->wrongCount--;
                playStats.exp += 10;
                updateLevel();
                currentReviewColor = 14;
                strcpy(currentReviewResult, "-> Good job! Keep practicing.");
            } else if (review == 3) {
                if (card->learned == 0) {
                    card->learned = 1;
                    dailyMission.wordsLearnedToday++;
                }
                card->wrongCount = 0;
                playStats.exp += 15;
                updateLevel();
                currentReviewColor = 10;
                strcpy(currentReviewResult, "-> Marked as fully learned!");
            }

            char *nextOptions[] = {
                "Next Flashcard",
                "Back to Flashcard Menu"
            };
            choice = selectMenu("", nextOptions, 2, printNextMenuHeader);
            choice += 1;

        } while (choice != 2);

    } while (1); // Always return to mode selection after a session ends
}