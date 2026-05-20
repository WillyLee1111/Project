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
#include "../include/ui.h"

Mission dailyMission = {0,0,0};
Streak studyStreak = {"", 0};
PlayStats playStats = {0, 0};
User currentUser;

void dictionaryMenu(Word **head){
    int choice;
    do {

        clearScreen();

        printf("============= DICTIONARY MENU =============");
        printf("\n");
        printf("=============================\n");
        setColor(11);
        printf("[1] ");
        setColor(7);
        printf("Show Dictionary\n");
        setColor(11);
        printf("[2] ");
        setColor(7);
        printf("Search Word\n");
        setColor(11);
        printf("[3] ");
        setColor(7);
        printf("Random Word\n");
        setColor(11);
        printf("[4] ");
        setColor(7);
        printf("Add Word\n");
        setColor(11);
        printf("[5] ");
        setColor(7);
        printf("Edit Word\n");
        setColor(11);
        printf("[6] ");
        setColor(7);
        printf("Delete Word\n");
        setColor(12);
        printf("[0] ");
        setColor(7);
        printf("Back\n");
        printf("=============================\n");

        printf("Enter your choice: ");

        scanf("%d", &choice);
        getchar();
        clearScreen();

        switch(choice){

            case 1:

                printf("============= DICTIONARY =============\n");

                displayDictionary(*head);

                pauseScreen();

                break;

            case 2: {

                char target[50];

                Word* suggestions[20];

                printf("============= SEARCH WORD =============\n");

                printf("Enter the word to search: ");

                scanf("%s", target);

                int count = suggestWords(
                    *head,
                    target,
                    suggestions
                );

                printf("\n");

                if (count > 0) {

                    printf("Suggestions for '%s':\n", target);

                    for (int i = 0; i < count; i++) {

                        printf(
                            "%d. %s\n",
                            i + 1,
                            suggestions[i]->word
                        );
                    }

                    int choose;

                    printf(
                        "\nChoose a word (0 to cancel): "
                    );

                    scanf("%d", &choose);

                    if (
                        choose > 0 &&
                        choose <= count
                    ) {

                        Word* selected =
                            suggestions[choose - 1];

                        clearScreen();

                        printf(
                            "============= WORD DETAIL =============\n\n"
                        );

                        printf(
                            "Word          : %s\n",
                            selected->word
                        );

                        printf(
                            "Meaning       : %s\n",
                            selected->meaning
                        );

                        printf(
                            "Pronunciation : %s\n",
                            selected->pronunciation
                        );

                        printf(
                            "Type          : %s\n",
                            selected->type
                        );
                    }
                }

                else {

                    printf("Word not found.\n");
                }

                printf("\n");

                pauseScreen();

                break;
            }

            case 3: {

                Word *randomWord =
                    getRandomWord(*head);

                if (randomWord != NULL) {

                    printf("============= RANDOM WORD =============\n\n");

                    printf("Word          : %s\n",randomWord->word);
                    printf("Meaning       : %s\n",randomWord->meaning);
                    printf("Pronunciation : %s\n",randomWord->pronunciation);
                    printf("Type          : %s\n",randomWord->type);
                }
                else {
                    printf(
                        "No words available.\n"
                    );
                }
                pauseScreen();
                break;
            }
            case 4:
                addWord(head);
                saveDictionary(*head);
                pauseScreen();
                break;
            case 5:
                editWord(*head);
                saveDictionary(*head);
                pauseScreen();
                break;
            case 6:
                deleteWord(head);
                saveDictionary(*head);
                pauseScreen();
                break;
        }
    } while(choice != 0);
}


void gameMenu(Word *head) {

    int gameChoice;

    do {
        clearScreen();
        showGameMenu();
        printf("\nChoose: ");
        scanf("%d", &gameChoice);
        getchar();
        switch(gameChoice){
            case 1:
                englishToVietnameseGame(head);
                break;
            case 2:
                vietnameseToEnglishGame(head);
                break;
            case 3:
                playMissingLetterGame(head);
                break;
            case 0:
                return;
            default:
                printError("Invalid choice!");
                pauseScreen();
        }
    } while(gameChoice != 4);

}

int main(){
    showIntroScreen();
    SetConsoleOutputCP(CP_UTF8);
        clearScreen();
    srand(time(NULL));
    int option;
    printf("Welcome to the English Dictionary App!\n");
    printf("1. Login\n");
    printf("2. Register\n");
    printf("Enter your choice: ");
    scanf("%d", &option);
    getchar();
    if (option == 1) {
        if (!login()) {
            printf("Login failed. Exiting...\n");
            pauseScreen();
            return 0;
        }
    }
    else if (option == 2) {
        if (!registerUser()) {
            printf("Registration failed. Exiting...\n");
            pauseScreen();
            return 0;
        }
    }
    else {
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
        printf("=============== DAILY MISSION ===============\n");
        printf("Words Learned     : %d /10\n", dailyMission.wordsLearnedToday);
        printf("Flashcards Review   : %d /5\n", dailyMission.flashcardsReviewed);
        printf("🔥 Study Streak     : %d days\n", studyStreak.streakDays);
        printHeader("ENGLISH DICTIONARY");
        showMiniPlayerCard();
        printf("\n");
        setColor(11);
        printf("[1] ");
        setColor(7);
        printf("Dictionary Menu\n");

        setColor(11);
        printf("[2] ");
        setColor(7);
        printf("Flashcard Mode\n");

        setColor(11);
        printf("[3] ");
        setColor(7);
        printf("Game Center\n");

        setColor(11);
        printf("[4] ");
        setColor(7);
        printf("Show Stats\n");

        setColor(12);
        printf("[5] ");
        setColor(7);
        printf("Exit\n");


        printf("\nChoose: ");

        scanf("%d", &choice);
        clearScreen();
        switch(choice){
            case 1:
                dictionaryMenu(&head);
                break;
            case 2:
                flashcardMode(head);
                break;
            case 3:
                gameMenu(head);
                break;
            case 4:
                showStats(head);
                pauseScreen();
                break;
            case 5:
                saveProgress(head);
                saveUserData();
                saveDictionary(head);
                printf("Data saved successfully.\n");
                pauseScreen();
                break;
            default:
                printf("Invalid choice.\n");
                pauseScreen();
                break;
        }
    } while(choice != 5);
    freeList(head);
    return 0;
}