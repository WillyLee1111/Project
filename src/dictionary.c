#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/dictionary.h"
#include "../include/validator.h"
#include "../include/utils.h"

HashTable* createHashTable() {
    HashTable *ht = (HashTable*)malloc(sizeof(HashTable));
    for (int i = 0; i < HASH_SIZE; i++) {
        ht->buckets[i] = NULL;
    }
    return ht;
}

unsigned int hashFunction(char* word) {
    unsigned int hash = 5381;
    int c;
    while ((c = *word++)) {
        hash = ((hash << 5) + hash) + tolower(c);
    }
    return hash % HASH_SIZE;
}

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
    
    newWord->learned = 0;
    newWord->wrongCount = 0;
    newWord->isFavorite = 0;
    newWord->next = NULL;
    
    return newWord;
}

void insertWord(HashTable* ht, Word *newWord) {
    unsigned int index = hashFunction(newWord->word);
    newWord->next = ht->buckets[index];
    ht->buckets[index] = newWord;
}

// Helper: print meanings split by ';' as numbered list
static void printWordMeanings(const char *meaning) {
    char copy[500];
    strncpy(copy, meaning, sizeof(copy) - 1);
    copy[sizeof(copy) - 1] = '\0';
    char *token = strtok(copy, ";");
    int num = 1;
    while (token != NULL) {
        printf("  %d. %s\n", num++, token);
        token = strtok(NULL, ";");
    }
}

void displayDictionary(HashTable* ht) {
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            count++;
            printf("=================================\n");
            printf("Word          : %s\n", temp->word);
            printf("Meaning(s)    :\n");
            printWordMeanings(temp->meaning);
            printf("Pronunciation : %s\n", temp->pronunciation);
            printf("Type          : %s\n", temp->type);
            printf("Status        : %s | Wrong: %d\n",
                   temp->learned ? "Learned" : "Not learned", temp->wrongCount);
            printf("=================================\n");
            temp = temp->next;
        }
    }
    if (count == 0) printf("Dictionary is empty.\n");
    else printf("Total: %d word(s)\n", count);
}

Word* searchWord(HashTable* ht, char *target) {
    unsigned int index = hashFunction(target);
    Word *temp = ht->buckets[index];
    while (temp != NULL) {
        if (_stricmp(temp->word, target) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

int suggestWords(HashTable* ht, char *prefix, Word *suggestions[20]) {
    int count = 0;
    size_t prefixLen = strlen(prefix);
    for (int i = 0; i < HASH_SIZE && count < 20; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL && count < 20) {
            if (_strnicmp(temp->word, prefix, prefixLen) == 0) {
                suggestions[count++] = temp;
            }
            temp = temp->next;
        }
    }
    return count;
}

void freeHashTable(HashTable* ht) {
    if (ht == NULL) return;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            Word *toFree = temp;
            temp = temp->next;
            free(toFree);
        }
    }
    free(ht);
}

Word* getRandomWord(HashTable* ht) {
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            count++;
            temp = temp->next;
        }
    }
    if (count == 0) return NULL;

    int randomIndex = rand() % count;
    count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            if (count == randomIndex) return temp;
            count++;
            temp = temp->next;
        }
    }
    return NULL;
}

Word* getAdaptiveWord(HashTable* ht) {
    int useWeakWords = rand() % 100;
    if (useWeakWords < 70) {
        Word* weakWords[1000];
        int count = 0;
        for (int i = 0; i < HASH_SIZE; i++) {
            Word *temp = ht->buckets[i];
            while (temp != NULL) {
                if (temp->wrongCount >= 3 && temp->learned == 0) {
                    if (count < 1000) weakWords[count++] = temp;
                }
                temp = temp->next;
            }
        }
        if (count > 0) {
            int randomIndex = rand() % count;
            return weakWords[randomIndex];
        }
    }
    return getRandomWord(ht);
}

// Like getAdaptiveWord but only picks words with length >= minLen.
// Used by Missing Letter game to ensure enough letters are visible.
Word* getAdaptiveWordMinLen(HashTable* ht, int minLen) {
    int useWeakWords = rand() % 100;
    if (useWeakWords < 70) {
        Word* weakWords[1000];
        int count = 0;
        for (int i = 0; i < HASH_SIZE; i++) {
            Word *temp = ht->buckets[i];
            while (temp != NULL) {
                if (temp->wrongCount >= 3 && temp->learned == 0 &&
                    (int)strlen(temp->word) >= minLen) {
                    if (count < 1000) weakWords[count++] = temp;
                }
                temp = temp->next;
            }
        }
        if (count > 0) return weakWords[rand() % count];
    }
    // Random fallback with length filter
    Word* candidates[1000];
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            if ((int)strlen(temp->word) >= minLen) {
                if (count < 1000) candidates[count++] = temp;
            }
            temp = temp->next;
        }
    }
    if (count == 0) return getRandomWord(ht); // ultimate fallback: any word
    return candidates[rand() % count];
}


void showStats(HashTable* ht) {
    int totalWords = 0;
    int learnedWords = 0;
    int weakWords = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            totalWords++;
            if (temp->learned == 1) learnedWords++;
            if (temp->wrongCount >= 3 && temp->learned == 0) weakWords++;
            temp = temp->next;
        }
    }

    float learningRate = 0;
    if (totalWords > 0) {
        learningRate = ((float)learnedWords / totalWords) * 100;
    }

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
    
    int found = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            if (temp->wrongCount >= 3 && temp->learned == 0) {
                found = 1;
                printf("%s  (%d wrongs)\n", temp->word,temp->wrongCount);
            }
            temp = temp->next;
        }
    }
    if (!found) {
        printf("No difficult words yet!\n");
    }
    printf("----------------------------------------\n");
    printf("\n");
    pauseScreen();
}

void saveProgress(HashTable* ht) {
    char path[100];
    snprintf(path, sizeof(path), "data/users/%s/progress.txt", currentUser.username);
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        fprintf(stderr, "Could not open file for writing\n");
        return;
    }
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            fprintf(file, "%s %d %d\n", temp->word, temp->learned, temp->wrongCount);
            temp = temp->next;
        }
    }
    fclose(file);
}

void loadProgress(HashTable* ht) {
    char path[100];
    snprintf(path, sizeof(path), "data/users/%s/progress.txt", currentUser.username);
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return;
    }
    char word[50];
    int learned;
    int wrongCount;
    while(fscanf(file, "%49s %d %d", word, &learned, &wrongCount) == 3) {
        Word *temp = searchWord(ht, word);
        if (temp != NULL) {
            temp->learned = learned == 1 ? 1 : 0;
            temp->wrongCount = wrongCount < 0 ? 0 : wrongCount;
        }
    }
    fclose(file);
}

Word* getWordByType(HashTable* ht, char *type) {
    Word* filtered[1000];
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            if (_stricmp(temp->type, type) == 0) {
                if (count < 1000) filtered[count++] = temp;
            }
            temp = temp->next;
        }
    }
    if (count == 0) return NULL;
    int randomIndex = rand() % count;
    return filtered[randomIndex];
}

Word* getWeakWord(HashTable* ht) {
    Word* weakWords[1000];
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            if (temp->wrongCount >= 3 && temp->learned == 0) {
                if (count < 1000) weakWords[count++] = temp;
            }
            temp = temp->next;
        }
    }
    if (count == 0) return NULL;
    int randomIndex = rand() % count;
    return weakWords[randomIndex];
}

void addWord(HashTable* ht) {
    char word[50], meaning[500], pronunciation[50], type[20];
    char confirm = 'n';

    // Fix 3: confirmation loop - allow correction before submitting
    do {
        printf("Enter the word: ");
        scanf("%49s", word);
        getchar();
        if (searchWord(ht, word) != NULL) {
            printf("Word '%s' already exists in the dictionary.\n", word);
            pauseScreen();
            return;
        }
        printf("Word entered: '%s' - Is this correct? (y/n): ", word);
        scanf(" %c", &confirm);
        getchar();
    } while (confirm != 'y' && confirm != 'Y');

    // Fix 6: multiple meanings - collect with ';' separator
    meaning[0] = '\0';
    int meaningCount = 0;
    char addMore = 'y';
    while (addMore == 'y' || addMore == 'Y') {
        char tempMeaning[200];
        printf("Enter meaning %d: ", meaningCount + 1);
        fgets(tempMeaning, sizeof(tempMeaning), stdin);
        tempMeaning[strcspn(tempMeaning, "\r\n")] = '\0';
        if (meaningCount > 0)
            strncat(meaning, ";", sizeof(meaning) - strlen(meaning) - 1);
        strncat(meaning, tempMeaning, sizeof(meaning) - strlen(meaning) - 1);
        meaningCount++;
        printf("Add another meaning? (y/n): ");
        scanf(" %c", &addMore);
        getchar();
    }

    printf("Enter the pronunciation: ");
    fgets(pronunciation, sizeof(pronunciation), stdin);
    pronunciation[strcspn(pronunciation, "\r\n")] = '\0';

    printf("Enter the type (noun/verb/adjective/adverb): ");
    fgets(type, sizeof(type), stdin);
    type[strcspn(type, "\r\n")] = '\0';

    Word *newWord = createWord(word, meaning, pronunciation, type);
    insertWord(ht, newWord);
    printSuccess("Word added successfully!");
}

void editWord(HashTable* ht) {
    char target[50];
    printf("Enter the word to edit: ");
    scanf("%49s", target);
    Word* word = searchWord(ht, target);
    if (word == NULL) {
        printf("Word not found.\n");
        return;
    }
    // Show current info
    printf("\nCurrent info for '%s':\n", word->word);
    printf("Meaning(s): "); 
    char copy[500];
    strncpy(copy, word->meaning, sizeof(copy)-1);
    char *tok = strtok(copy, ";");
    int n = 1;
    while (tok) { printf("%d. %s  ", n++, tok); tok = strtok(NULL, ";"); }
    printf("\nPronunciation: %s | Type: %s\n\n", word->pronunciation, word->type);

    char meaning[500], pronunciation[50], type[20];
    getchar();
    // Fix 6: multiple meanings when editing
    meaning[0] = '\0';
    int meaningCount = 0;
    char addMore = 'y';
    printf("Enter new meaning(s) - use ';' or enter one by one:\n");
    while (addMore == 'y' || addMore == 'Y') {
        char tempMeaning[200];
        printf("Meaning %d: ", meaningCount + 1);
        fgets(tempMeaning, sizeof(tempMeaning), stdin);
        tempMeaning[strcspn(tempMeaning, "\r\n")] = '\0';
        if (meaningCount > 0)
            strncat(meaning, ";", sizeof(meaning) - strlen(meaning) - 1);
        strncat(meaning, tempMeaning, sizeof(meaning) - strlen(meaning) - 1);
        meaningCount++;
        printf("Add another meaning? (y/n): ");
        scanf(" %c", &addMore);
        getchar();
    }
    printf("Enter the new pronunciation: ");
    fgets(pronunciation, sizeof(pronunciation), stdin);
    pronunciation[strcspn(pronunciation, "\r\n")] = '\0';
    printf("Enter the new type (noun/verb/adjective/adverb): ");
    fgets(type, sizeof(type), stdin);
    type[strcspn(type, "\r\n")] = '\0';
    strncpy(word->meaning, meaning, sizeof(word->meaning) - 1);
    word->meaning[sizeof(word->meaning) - 1] = '\0';
    strncpy(word->pronunciation, pronunciation, sizeof(word->pronunciation) - 1);
    word->pronunciation[sizeof(word->pronunciation) - 1] = '\0';
    strncpy(word->type, type, sizeof(word->type) - 1);
    word->type[sizeof(word->type) - 1] = '\0';
    printSuccess("Word updated successfully!");
}

void deleteWord(HashTable* ht) {
    char target[50];
    printf("Enter the word to delete: ");
    scanf("%49s", target);
    unsigned int index = hashFunction(target);
    Word* temp = ht->buckets[index];
    Word* prev = NULL;

    while (temp != NULL && _stricmp(temp->word, target) != 0) {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL) {
        printf("Word not found.\n");
        return;
    }
    if (prev == NULL) {
        ht->buckets[index] = temp->next;
    } else {
        prev->next = temp->next;
    }
    free(temp);
    printSuccess("Word deleted successfully!");
}

void saveDictionary(HashTable* ht) {
    FILE *file = fopen("data/dictionary.txt", "w");
    if (file == NULL) {
        fprintf(stderr, "Could not open file for writing\n");
        return;
    }
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            fprintf(file, "%s|%s|%s|%s\n", temp->word, temp->meaning, temp->pronunciation, temp->type);
            temp = temp->next;
        }
    }
    fclose(file);
}

void updateLevel (){
    playStats.level = playStats.exp / 100 + 1;
}
