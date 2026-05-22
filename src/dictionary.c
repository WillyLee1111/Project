#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/dictionary.h"
#include "../include/validator.h"
#include "../include/utils.h"


Word* createWord(
    char* word, 
    char* meaning, 
    char* pronunciation, 
    char* type
) {
    Word *newWord = (Word *)malloc(sizeof(Word));
    if (newWord == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strncpy(newWord->word, word, sizeof(newWord->word) - 1);
    newWord->word[sizeof(newWord->word) - 1] = '\0';
    
    strncpy(newWord->meaning, meaning, sizeof(newWord->meaning) - 1);
    newWord->meaning[sizeof(newWord->meaning) - 1] = '\0';
    
    strncpy(newWord->pronunciation, pronunciation, sizeof(newWord->pronunciation) - 1);
    newWord->pronunciation[sizeof(newWord->pronunciation) - 1] = '\0';
    
    strncpy(newWord->type, type, sizeof(newWord->type) - 1);
    newWord->type[sizeof(newWord->type) - 1] = '\0';
    
    newWord->wrongCount = 0;
    newWord->next = NULL;
    
    return newWord;
}

void insertWord(Word **head, Word *newWord) {
    if (*head == NULL) {
        *head = newWord;
        return;
    } else {
        Word *temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newWord;
    }
}

void displayDictionary(Word *head) {
    Word *temp = head;
    while (temp != NULL) {
        printf("=================================\n");
        printf("Word: %s\n", temp->word);
        printf("Meaning: %s\n", temp->meaning);
        printf("Pronunciation: %s\n", temp->pronunciation);
        printf("Type: %s\n", temp->type);
        printf("Wrong Count: %d\n", temp->wrongCount);
        printf("=================================\n");
        temp = temp->next;
    }
}

Word* searchWord(Word *head, char *target) {
    Word *temp = head;
    while (temp != NULL) {
        if (strcmp(temp->word, target) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL; // Not found
}

int suggestWords(Word *head, char *prefix, Word *suggestions[20]) {
    int count = 0;
    Word *temp = head;
    while (temp != NULL && count < 20) {
        if (strncmp(temp->word, prefix, strlen(prefix)) == 0) {
            suggestions[count++] = temp;
        }
        temp = temp->next;
    }
    return count;
}
void freeList(Word *head) {
    Word *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

Word* getRandomWord(Word *head) {
    if (head == NULL) return NULL; // No words in the dictionary
    int count = 0;
    Word *temp = head;
    while (temp != NULL) {
        count++;
        temp = temp->next;
    }
    if (count == 0) return NULL; // No words in the dictionary

    int randomIndex = rand() % count;
    temp = head;
    for (int i = 0; i < randomIndex; i++) {
        temp = temp->next;
    }
    return temp;
}

Word* getAdaptiveWord(Word *head) {
    int useWeakWords = rand() % 100; // Randomly decide to use weak words or not
    if (useWeakWords < 70) { // 70% chance to use weak words
        Word* weakWords[100];
        int count = 0;
        Word *temp = head;
        while (temp != NULL) {
            if (temp->wrongCount >= 3 && temp->learned == 0) {
                weakWords[count++] = temp;
            }
            temp = temp->next;
        }
        if (count > 0) {
            int randomIndex = rand() % count;
            return weakWords[randomIndex];
        }
    }
    return getRandomWord(head);
}

void showStats(Word *head) {
    int totalWords = 0;
    int learnedWords = 0;
    int weakWords = 0;
    Word *temp = head;
    while (temp != NULL) {
        totalWords++;
        if (temp->learned == 1) {
            learnedWords++;
        }
        if (
            temp->wrongCount >= 3 &&
            temp->learned == 0
        ) {

            weakWords++;
        }
        temp = temp->next;
    }

    float learningRate =
        ((float)learnedWords / totalWords) * 100;

    clearScreen();

    printf("============== PLAYER STATISTICS ==============");
    printf("\n");
    printf("========================================\n");
    setColor(11);
    printf("LEVEL");
    setColor(7);
    printf("            : %d\n", playStats.level);

    setColor(11);
    printf("EXP");
    setColor(7);
    printf("              : %d\n", playStats.exp);

    setColor(11);
    printf("STREAK");
    setColor(7);
    printf("           : %d days\n",studyStreak.streakDays);

    printf("========================================\n");

    printf("\n");

    setColor(14);
    printf("DICTIONARY PROGRESS\n");
    setColor(7);

    printf("----------------------------------------\n");

    printf("Total Words     : %d\n",totalWords);

    printf("Learned Words   : %d\n",learnedWords);

    printf("Weak Words      : %d\n",weakWords);

    printf("Learning Rate   : %.2f%%\n",learningRate);

    printf("----------------------------------------\n");

    printf("\n");

    setColor(13);
    printf("DAILY MISSION\n");
    setColor(7);

    printf("----------------------------------------\n");

    printf("Words Learned   : %d / 10\n",dailyMission.wordsLearnedToday);

    printf("Flashcards      : %d / 5\n",dailyMission.flashcardsReviewed);

    printf("Games Played    : %d / 1\n",dailyMission.gamesPlayed);

    printf("----------------------------------------\n");
    printf("\n");
    setColor(12);
    printf("MOST DIFFICULT WORDS\n");
    setColor(7);
    printf("----------------------------------------\n");
    temp = head;
    int found = 0;
    while (temp != NULL) {
        if (
            temp->wrongCount >= 3 &&
            temp->learned == 0
        ) {
            found = 1;
            printf("%s  (%d wrongs)\n", temp->word,temp->wrongCount);
        }
        temp = temp->next;
    }
    if (!found) {
        printf("No difficult words yet!\n");
    }
    printf("----------------------------------------\n");
    printf("\n");
    pauseScreen();
}


void saveProgress(Word *head) {
    char path[100];
    snprintf(path, sizeof(path), "data/users/%s/progress.txt", currentUser.username);
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        fprintf(stderr, "Could not open file for writing\n");
        return;
    }
    Word *temp = head;
    while (temp != NULL) {
        fprintf(file, "%s %d %d\n", temp->word, temp->learned, temp->wrongCount);
        temp = temp->next;
    }
    fclose(file);
}


void loadProgress(Word *head) {
    char path[100];
    snprintf(path, sizeof(path), "data/users/%s/progress.txt", currentUser.username);
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file for reading\n");
        return;
    }
    char word[50];
    int learned;
    int wrongCount;
    while(
        fscanf(file, "%49s %d %d", word, &learned, &wrongCount) == 3
    ) {
        Word *temp = head;
        while (temp != NULL) {
            if (strcmp(temp->word, word) == 0) {
                temp->learned = learned;
                temp->wrongCount = wrongCount;
                break;
            }
            temp = temp->next;
        }
    }
    fclose(file);
}

Word* getWordByType(Word *head, char *type) {
    Word* filtered[1000];
    int count = 0;
    Word *temp = head;
    while (temp != NULL) {
        if (strcmp(temp->type, type) == 0) {
            filtered[count] = temp;
            count ++;
        }
        temp = temp->next;
    }
    if (count ==0){
        return NULL;
    }
    int randomIndex = rand() % count;
    return filtered[randomIndex];
}

Word* getWeakWord(Word *head) {
    Word* weakWords[1000];
    int count = 0;
    Word *temp = head;
    while (temp != NULL) {
        if (temp->wrongCount >= 3 && temp->learned == 0) {
            weakWords[count++] = temp;
        }
        temp = temp->next;
    }
    if (count == 0) {
        return NULL; // No weak words found
    }
    int randomIndex = rand() % count;
    return weakWords[randomIndex];
}

void addWord(Word** head) {
    char word[50], meaning[200], pronunciation[50], type[20];
    printf("Enter the word: ");
    scanf("%49s", word);
    if (isDuplicateWord(*head, word)) {
        printf("Word already exists in the dictionary.\n");
        return;
    }
    getchar(); // Consume the newline character left by scanf
    printf("Enter the meaning: ");
    fgets(meaning, sizeof(meaning), stdin);
    meaning[strcspn(meaning, "\r\n")] = '\0'; // Remove newline character
    printf("Enter the pronunciation: ");
    fgets(pronunciation, sizeof(pronunciation), stdin);
    pronunciation[strcspn(pronunciation, "\r\n")] = '\0'; // Remove newline character
    printf("Enter the type (noun/verb/adjective/adverb): ");
    fgets(type, sizeof(type), stdin);
    type[strcspn(type, "\r\n")] = '\0'; // Remove newline character

    Word *newWord = createWord(word, meaning, pronunciation, type);
    insertWord(head, newWord);
    printSuccess("Word added successfully!");
}

void editWord(Word* head) {
    char target[50];
    printf("Enter the word to edit: ");
    scanf("%49s", target);
    Word* word = searchWord(head, target);
    if (word == NULL) {
        printf("Word not found.\n");
        return;
    }
    char meaning[200], pronunciation[50], type[20];
    printf("Enter the new meaning: ");
    getchar(); // Consume the newline character left by scanf
    fgets(meaning, sizeof(meaning), stdin);
    meaning[strcspn(meaning, "\r\n")] = '\0'; // Remove newline character
    printf("Enter the new pronunciation: ");
    fgets(pronunciation, sizeof(pronunciation), stdin);
    pronunciation[strcspn(pronunciation, "\r\n")] = '\0'; // Remove newline character
    printf("Enter the new type (noun/verb/adjective/adverb): ");
    fgets(type, sizeof(type), stdin);
    type[strcspn(type, "\r\n")] = '\0'; // Remove newline character
    strncpy(word->meaning, meaning, sizeof(word->meaning) - 1);
    word->meaning[sizeof(word->meaning) - 1] = '\0';
    strncpy(word->pronunciation, pronunciation, sizeof(word->pronunciation) - 1);
    word->pronunciation[sizeof(word->pronunciation) - 1] = '\0';
    strncpy(word->type, type, sizeof(word->type) - 1);
    word->type[sizeof(word->type) - 1] = '\0';
    printf("Word updated successfully!\n");
}

void deleteWord(Word** head) {
    char target[50];
    printf("Enter the word to delete: ");
    scanf("%s", target);
    Word* temp = *head;
    Word* prev = NULL;

    while (temp != NULL && strcmp(temp->word, target) != 0) {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL) {
        printf("Word not found.\n");
        return;
    }
    if (prev == NULL) {
        *head = temp->next; // Deleting the head
    } else {
        prev->next = temp->next; // Bypass the deleted node
    }
    free(temp);
}
void saveDictionary(Word *head) {
    FILE *file = fopen("data/dictionary.txt", "w");
    if (file == NULL) {
        fprintf(stderr, "Could not open file for writing\n");
        return;
    }
    Word *temp = head;
    while (temp != NULL) {
        fprintf(file, "%s|%s|%s|%s\n", temp->word, temp->meaning, temp->pronunciation, temp->type);
        temp = temp->next;
    }
    fclose(file);
}

void updateLevel (){
    playStats.level = playStats.exp / 100 + 1;
}