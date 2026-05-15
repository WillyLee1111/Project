#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#include "../include/dictionary.h"
#include "../include/flashcard.h"
#include "../include/game.h"
#include "../include/utils.h"
#include "../include/file.h"

Mission dailyMission = {0,0,0};
Streak studyStreak = {"", 0};

int main(){
    srand(time(NULL));
    int option;
    printf("Welcome to the English Dictionary App!\n");
    printf("1. Login\n");
    printf("2. Register\n");
    printf("Enter your choice: ");
    scanf("%d", &option);
    getchar(); // Consume newline character
    if (option == 1) {
        if (!login()) {
            printf("Login failed. Exiting...\n");
            pauseScreen();
            return 0;
        }
    } else if (option == 2) {
        if (!registerUser()) {
            printf("Registration failed. Exiting...\n");
            pauseScreen();
            return 0;
        }
    } else {
        printf("Invalid option. Exiting...\n");
        pauseScreen();
        return 0;
    }
    SetConsoleOutputCP(CP_UTF8);
    Word *head = NULL;
    loadDictionary(&head);
    loadProgress(head);
    loadUserData();
    int choice;
    do {
          clearScreen();
          printf("=========== DAILY MISSION ===========\n");
          printf("Words Learned : %d / 10\n",dailyMission.wordsLearnedToday
);
          printf("Flashcards Reviewed : %d / 5\n", dailyMission.flashcardsReviewed
);
          printf("Games Played : %d / 1\n",dailyMission.gamesPlayed
);
          printf(
    "🔥 Study Streak: %d days\n\n",
    studyStreak.streakDays
);
        printf("\n");
          printf("===============================\n");
          printf("     ENGLISH DICTIONARY\n");
          printf("===============================\n");
            printf("1. SHOW Dictionary\n");
            printf("2. Search Word\n");
            printf("3. Random Word\n");
            printf("4. Flashcard Mode\n");
            printf("5. Show Stats\n");
            printf("6. Missing Letter Game\n");
            printf("7. Add Word\n");
            printf("8. Edit Word\n");
            printf("9. Delete Word\n");
            printf("10. Exit\n");
            printf("===============================\n");
            printf("Enter your choice: ");
            scanf("%d", &choice);
            clearScreen();
            switch (choice) {
                case 1:
                printf("=============DICTIONARY=============\n");
                    displayDictionary(head);
                    pauseScreen();
                    break;
                case 2: {

    char target[50];

    Word* suggestions[20];

    printf("=============SEARCH WORD=============\n");

    printf("Enter the word to search: ");
    scanf("%s", target);

    int count = suggestWords(
        head,
        target,
        suggestions
    );

    printf("\n");

    if (count > 0) {
        printf("Suggestions for '%s':\n", target);
        for (int i = 0; i < count; i++) {
            printf("%d. %s\n",
                i + 1,
                suggestions[i]->word
            );
        }
        int choice;
        printf("\nChoose a word (0 to cancel): ");
        scanf("%d", &choice);
        if (choice > 0 && choice <= count) {
            Word* selected =
                suggestions[choice - 1];
            clearScreen();
            printf("=============WORD DETAIL=============\n\n");
            printf("Word          : %s\n",
                selected->word);
            printf("Meaning       : %s\n",
                selected->meaning);
            printf("Pronunciation : %s\n",
                selected->pronunciation);
            printf("Type          : %s\n",
                selected->type);
        }
    }
    else {
        printf("Word not found.\n");
    }
    printf("\n");
    pauseScreen();
    break;
}
                case 3:
                    Word *randomWord = getRandomWord(head);
                    if (randomWord != NULL) {
                        clearScreen();
                        printf("=============RANDOM WORD=============\n\n");
                        printf("Word          : %s\n",
                            randomWord->word);
                        printf("Meaning       : %s\n",
                            randomWord->meaning);
                        printf("Pronunciation : %s\n",
                            randomWord->pronunciation);
                        printf("Type          : %s\n",
                            randomWord->type);
                    } else {
                        printf("No words available in the dictionary.\n");
                    }
                    pauseScreen();
                    break;
                default:
                    printf("Invalid choice. Please try again.\n");
                    pauseScreen();
                    break;
                case 4:
                    flashcardMode(head);
                    break;
                case 5:
                    showStats(head);
                    pauseScreen();
                    break;
                case 6:
                    playMissingLetterGame(head);
                    break;
                case 7:
                    addWord(&head);
                    break;
                case 8:
                    editWord(head);
                    break;
                case 9:
                    deleteWord(&head);
                    break;
                case 10:
                saveProgress(head);
                    printf("Progress saved successfully.\n");
                    saveUserData();
                    printf("User data saved successfully.\n");
                    saveDictionary(head);
                    printf("Dictionary saved successfully.\n");
                    break;
            }
    } while (choice != 10);
    freeList(head);
    return 0;
}