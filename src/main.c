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

Mission dailyMission = {0,0,0,0,0,0};
Streak studyStreak = {"", 0};
PlayStats playStats = {0, 0};
User currentUser;

// Fix 5: Global ht for auto-save on unexpected close
static HashTable *globalHt = NULL;

BOOL WINAPI consoleHandler(DWORD signal) {
    if ((signal == CTRL_CLOSE_EVENT || signal == CTRL_C_EVENT) && globalHt != NULL) {
        saveProgress(globalHt);
        saveUserData();
        saveDictionary(globalHt);
    }
    return FALSE; // Let default handler terminate
}

// Draw a simple progress bar
static void drawBar(int current, int total) {
    int width = 10;
    int filled = (total > 0) ? (current * width / total) : 0;
    if (filled > width) filled = width;
    printf("[");
    for (int i = 0; i < width; i++) {
        if (i < filled) printf("=");
        else if (i == filled && current < total) printf(">");
        else printf(" ");
    }
    printf("]");
}

// Fix 8: show word detail inline, used after displayDictionary
static void showWordDetail(Word *w) {
    if (w == NULL) return;
    clearScreen();
    printf("============= WORD DETAIL =============\n\n");
    printf("Word          : %s\n", w->word);
    printf("Pronunciation : %s\n", w->pronunciation);
    printf("Type          : %s\n", w->type);
    printf("Status        : %s | Wrong: %d\n\n",
           w->learned ? "Learned" : "Not learned", w->wrongCount);
    printf("Meaning(s):\n");
    char copy[500];
    strncpy(copy, w->meaning, sizeof(copy)-1);
    copy[sizeof(copy)-1] = '\0';
    char *tok = strtok(copy, ";");
    int n = 1;
    while (tok) { printf("  %d. %s\n", n++, tok); tok = strtok(NULL, ";"); }
}

void dictionaryMenu(HashTable *ht){
    int choice;
    do {
        clearScreen();
        printf("============= DICTIONARY MENU =============\n");
        printf("=============================\n");
        setColor(11); printf("[1] "); setColor(7); printf("Show Dictionary\n");
        setColor(11); printf("[2] "); setColor(7); printf("Search Word\n");
        setColor(11); printf("[3] "); setColor(7); printf("Random Word\n");
        setColor(11); printf("[4] "); setColor(7); printf("Add Word\n");
        setColor(11); printf("[5] "); setColor(7); printf("Edit Word\n");
        setColor(11); printf("[6] "); setColor(7); printf("Delete Word\n");
        setColor(12); printf("[0] "); setColor(7); printf("Back\n");
        printf("=============================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();
        clearScreen();

        switch(choice){
            case 1: {
                printf("============= DICTIONARY =============\n");
                displayDictionary(ht);
                // Fix 8: inline search after listing
                printf("\n--- Type a word to view its detail, or press Enter to go back ---\n> ");
                char searchInput[50];
                if (fgets(searchInput, sizeof(searchInput), stdin)) {
                    searchInput[strcspn(searchInput, "\r\n")] = '\0';
                    if (strlen(searchInput) > 0) {
                        Word *found = searchWord(ht, searchInput);
                        if (found != NULL) {
                            showWordDetail(found);
                        } else {
                            printf("Word '%s' not found in dictionary.\n", searchInput);
                        }
                        pauseScreen();
                    }
                }
                break;
            }
            case 2: {
                char target[50];
                Word* suggestions[20];
                printf("============= SEARCH WORD =============\n");
                printf("Enter the word to search: ");
                scanf("%s", target);
                getchar();
                int count = suggestWords(ht, target, suggestions);
                printf("\n");
                if (count > 0) {
                    printf("Suggestions for '%s':\n", target);
                    for (int i = 0; i < count; i++) {
                        printf("%d. %s\n", i + 1, suggestions[i]->word);
                    }
                    int choose;
                    printf("\nChoose a word (0 to cancel): ");
                    scanf("%d", &choose);
                    getchar();
                    if (choose > 0 && choose <= count) {
                        showWordDetail(suggestions[choose - 1]);
                        pauseScreen();
                    }
                } else {
                    printf("Word not found.\n");
                    pauseScreen();
                }
                break;
            }
            case 3: {
                Word *randomWord = getRandomWord(ht);
                if (randomWord != NULL) {
                    showWordDetail(randomWord);
                } else {
                    printf("No words available.\n");
                }
                pauseScreen();
                break;
            }
            case 4:
                addWord(ht);
                saveDictionary(ht);
                pauseScreen();
                break;
            case 5:
                editWord(ht);
                saveDictionary(ht);
                pauseScreen();
                break;
            case 6:
                deleteWord(ht);
                saveDictionary(ht);
                pauseScreen();
                break;
        }
    } while(choice != 0);
}

void gameMenu(HashTable *ht) {
    int gameChoice;
    do {
        clearScreen();
        showGameMenu();
        printf("\nChoose: ");
        scanf("%d", &gameChoice);
        getchar();
        switch(gameChoice){
            case 1: englishToVietnameseGame(ht); break;
            case 2: vietnameseToEnglishGame(ht); break;
            case 3: playMissingLetterGame(ht); break;
            case 0: return;
            default:
                printError("Invalid choice!");
                pauseScreen();
        }
    } while(gameChoice != 0);
    checkAndCompleteMission();
    saveUserData();
}

int main(){
    showIntroScreen();
    SetConsoleOutputCP(CP_UTF8);
    clearScreen();
    srand((unsigned int)time(NULL));

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
    // Fix 4: generate random targets for new day / first time
    resetDailyMissionIfNewDay();
    saveUserData();

    HashTable *ht = createHashTable();
    loadDictionary(ht);
    loadProgress(ht);

    // Fix 5: register auto-save handler
    globalHt = ht;
    SetConsoleCtrlHandler(consoleHandler, TRUE);

    int tw, tf, tg;
    int running = 1;
    int choice;
    while (running) {
        clearScreen();
        tw = dailyMission.targetWords;
        tf = dailyMission.targetFlashcards;
        tg = dailyMission.targetGames;

        printf("=============== DAILY MISSION ===============\n");
        printf("Words Learned  : %2d / %2d  ", dailyMission.wordsLearnedToday, tw);
        drawBar(dailyMission.wordsLearnedToday, tw);
        printf("%s\n", dailyMission.wordsLearnedToday >= tw ? " [DONE]" : "");

        printf("Flashcards     : %2d / %2d  ", dailyMission.flashcardsReviewed, tf);
        drawBar(dailyMission.flashcardsReviewed, tf);
        printf("%s\n", dailyMission.flashcardsReviewed >= tf ? " [DONE]" : "");

        printf("Games Played   : %2d / %2d  ", dailyMission.gamesPlayed, tg);
        drawBar(dailyMission.gamesPlayed, tg);
        printf("%s\n", dailyMission.gamesPlayed >= tg ? " [DONE]" : "");

        printf("Study Streak   : %d days\n", studyStreak.streakDays);
        printf("=============================================\n");

        printHeader("ENGLISH DICTIONARY");
        showMiniPlayerCard();
        printf("\n");
        setColor(11); printf("[1] "); setColor(7); printf("Dictionary Menu\n");
        setColor(11); printf("[2] "); setColor(7); printf("Flashcard Mode\n");
        setColor(11); printf("[3] "); setColor(7); printf("Game Center\n");
        setColor(11); printf("[4] "); setColor(7); printf("Show Stats\n");
        setColor(12); printf("[5] "); setColor(7); printf("Exit\n");
        printf("\nChoose: ");
        scanf("%d", &choice);
        getchar();
        clearScreen();

        switch(choice){
            case 1:
                dictionaryMenu(ht);
                break;
            case 2:
                flashcardMode(ht);
                checkAndCompleteMission();
                saveUserData();
                break;
            case 3:
                gameMenu(ht);
                break;
            case 4:
                showStats(ht);
                break;
            case 5: {
                // Fix 5: confirm before exit + always save
                printf("Are you sure you want to exit? (y/n): ");
                char c;
                scanf(" %c", &c);
                if (c == 'y' || c == 'Y') {
                    saveProgress(ht);
                    saveUserData();
                    saveDictionary(ht);
                    setColor(10);
                    printf("\nData saved successfully. Goodbye!\n");
                    setColor(7);
                    pauseScreen();
                    running = 0;
                }
                break;
            }
            default:
                printf("Invalid choice.\n");
                pauseScreen();
                break;
        }
    }
    freeHashTable(ht);
    globalHt = NULL;
    return 0;
}
