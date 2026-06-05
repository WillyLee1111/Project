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
    ht->bstRoot = NULL;  // BST starts empty
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

// ============================================================
// Hash Table core operations
// hashFunction: djb2 algorithm (Dan Bernstein)
//   hash(i) = hash(i-1) * 33 + char[i]
//   Seed 5381 is prime; multiplier 33 gives good bit distribution.
//   Complexity: O(L) where L = word length; lookup/insert O(1) avg.
// ============================================================

// ============================================================
// BST Implementation - used by suggestWords for O(log n) prefix search
// BST ordering: case-insensitive alphabetical (_stricmp)
// In-order traversal (left->root->right) yields words in A-Z order
// Prefix search complexity: O(log n + k), k = number of matches
// ============================================================

BSTNode* bstInsert(BSTNode *root, const char *word) {
    if (root == NULL) {
        BSTNode *node = (BSTNode*)malloc(sizeof(BSTNode));
        strncpy(node->word, word, sizeof(node->word) - 1);
        node->word[sizeof(node->word) - 1] = '\0';
        node->left = node->right = NULL;
        return node;
    }
    int cmp = _stricmp(word, root->word);
    if      (cmp < 0) root->left  = bstInsert(root->left,  word);
    else if (cmp > 0) root->right = bstInsert(root->right, word);
    // cmp == 0: duplicate word, skip
    return root;
}

static BSTNode* bstMinNode(BSTNode *node) {
    while (node->left != NULL) node = node->left;
    return node;
}

BSTNode* bstDelete(BSTNode *root, const char *word) {
    if (root == NULL) return NULL;
    int cmp = _stricmp(word, root->word);
    if (cmp < 0) {
        root->left  = bstDelete(root->left,  word);
    } else if (cmp > 0) {
        root->right = bstDelete(root->right, word);
    } else {
        // Found the node to delete
        if (root->left == NULL) {
            BSTNode *tmp = root->right; free(root); return tmp;
        }
        if (root->right == NULL) {
            BSTNode *tmp = root->left;  free(root); return tmp;
        }
        // Two children: replace with in-order successor (min of right subtree)
        BSTNode *successor = bstMinNode(root->right);
        strncpy(root->word, successor->word, sizeof(root->word) - 1);
        root->word[sizeof(root->word) - 1] = '\0';
        root->right = bstDelete(root->right, successor->word);
    }
    return root;
}

void freeBST(BSTNode *root) {
    if (root == NULL) return;
    freeBST(root->left);
    freeBST(root->right);
    free(root);
}

// Recursive in-order BST traversal for prefix suggestions.
// Pruning: if current word < prefix, go right only;
//          if current word > prefix+*, go left only;
//          if matches prefix, collect and go both.
static void bstCollect(BSTNode *node, const char *prefix, int prefixLen,
                        Word **suggestions, int *count, HashTable *ht) {
    if (node == NULL || *count >= 20) return;
    int cmp = _strnicmp(node->word, prefix, prefixLen);
    if (cmp < 0) {
        // This word comes before the prefix alphabetically -> only right subtree can match
        bstCollect(node->right, prefix, prefixLen, suggestions, count, ht);
    } else if (cmp > 0) {
        // This word comes after the prefix alphabetically -> only left subtree can match
        bstCollect(node->left, prefix, prefixLen, suggestions, count, ht);
    } else {
        // This word starts with the prefix -> collect it and check both subtrees
        bstCollect(node->left,  prefix, prefixLen, suggestions, count, ht);
        Word *found = searchWord(ht, node->word);
        if (found != NULL && *count < 20) suggestions[(*count)++] = found;
        bstCollect(node->right, prefix, prefixLen, suggestions, count, ht);
    }
}

// ============================================================

void insertWord(HashTable* ht, Word *newWord) {
    unsigned int index = hashFunction(newWord->word);
    newWord->next = ht->buckets[index];
    ht->buckets[index] = newWord;
    // Keep BST in sync for O(log n) prefix suggestions
    ht->bstRoot = bstInsert(ht->bstRoot, newWord->word);
}

// Helper: print meanings split by ';' as numbered list
void printWordMeanings(const char *meaning) {
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

// In-order BST traversal to print all words in alphabetical order (A-Z).
// This leverages the BST's sorted property - no extra sorting needed.
static void bstPrintInOrder(BSTNode *node, HashTable *ht, int *count) {
    if (node == NULL) return;
    bstPrintInOrder(node->left, ht, count);
    Word *w = searchWord(ht, node->word);
    if (w != NULL) {
        (*count)++;
        printf("=================================\n");
        printf("%-14s: %s\n", "Word", w->word);
        printf("%-14s:\n", "Meaning(s)");
        printWordMeanings(w->meaning);
        printf("%-14s: %s\n", "Pronunciation", w->pronunciation);
        printf("%-14s: %s\n", "Type", w->type);
        printf("%-14s: %s | Wrong: %d\n", "Status",
               w->learned ? "Learned" : "Not learned", w->wrongCount);
        printf("=================================\n");
    }
    bstPrintInOrder(node->right, ht, count);
}

void displayDictionary(HashTable* ht) {
    // Count total words
    int total = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *tmp = ht->buckets[i];
        while (tmp) { total++; tmp = tmp->next; }
    }
    if (total == 0) { printf("Dictionary is empty.\n"); return; }

    // Display via BST in-order = alphabetically sorted A-Z
    int count = 0;
    bstPrintInOrder(ht->bstRoot, ht, &count);
    printf("\nTotal: %d word(s) [sorted A-Z via BST in-order]\n", count);
}

Word* searchWord(HashTable* ht, char *target) {
    _strlwr(target); // Ensure case-insensitive hash lookup
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

// suggestWords: uses the embedded BST for O(log n) prefix search.
// Results are returned in alphabetical order (BST in-order property).
int suggestWords(HashTable* ht, char *prefix, Word *suggestions[20]) {
    _strlwr(prefix);
    int count = 0;
    int prefixLen = (int)strlen(prefix);
    bstCollect(ht->bstRoot, prefix, prefixLen, suggestions, &count, ht);
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
    freeBST(ht->bstRoot);  // free the BST
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
    int totalWords = 0, learnedWords = 0, weakWords = 0;
    // Word type counters
    int cntNoun = 0, cntVerb = 0, cntAdj = 0, cntAdv = 0, cntOther = 0;
    // Collect top-5 hardest words (sorted by wrongCount desc)
    Word *top5[5] = {NULL, NULL, NULL, NULL, NULL};

    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            totalWords++;
            if (temp->learned == 1) learnedWords++;
            if (temp->wrongCount >= 3 && temp->learned == 0) weakWords++;

            // Count by type
            if      (_stricmp(temp->type, "noun")      == 0) cntNoun++;
            else if (_stricmp(temp->type, "verb")      == 0) cntVerb++;
            else if (_stricmp(temp->type, "adjective") == 0) cntAdj++;
            else if (_stricmp(temp->type, "adverb")    == 0) cntAdv++;
            else cntOther++;

            // Insert into top-5 if wrongCount is high enough (insertion sort)
            for (int k = 0; k < 5; k++) {
                if (top5[k] == NULL || temp->wrongCount > top5[k]->wrongCount) {
                    for (int m = 4; m > k; m--) top5[m] = top5[m-1];
                    top5[k] = temp;
                    break;
                }
            }
            temp = temp->next;
        }
    }


    clearScreen();
    printf("============== PLAYER STATISTICS ==============\n");
    printf("========================================\n");
    setColor(11); printf("LEVEL      "); setColor(7); printf(": %d\n", playStats.level);
    setColor(11); printf("EXP        "); setColor(7); printf(": %d / %d (next level)\n",
        playStats.exp, (playStats.level) * 100);
    setColor(11); printf("STREAK     "); setColor(7); printf(": %d days\n", studyStreak.streakDays);
    printf("========================================\n\n");

    // Dictionary progress
    setColor(14); printf("DICTIONARY PROGRESS\n"); setColor(7);
    printf("----------------------------------------\n");
    printf("Total Words   : %d\n", totalWords);
    printf("Learned       : %d / %d\n", learnedWords, totalWords);
    printf("\n");

    // By word type breakdown
    setColor(14); printf("WORDS BY TYPE\n"); setColor(7);
    printf("----------------------------------------\n");
    printf("  Noun      : %d\n", cntNoun);
    printf("  Verb      : %d\n", cntVerb);
    printf("  Adjective : %d\n", cntAdj);
    printf("  Adverb    : %d\n", cntAdv);
    printf("  Other     : %d\n", cntOther);
    printf("\n");

    // Daily mission with dynamic targets
    setColor(13); printf("DAILY MISSION\n"); setColor(7);
    printf("----------------------------------------\n");
    printf("Words Learned : %d / %d\n", dailyMission.wordsLearnedToday,
           dailyMission.targetWords > 0 ? dailyMission.targetWords : 10);
    printf("Flashcards    : %d / %d\n", dailyMission.flashcardsReviewed,
           dailyMission.targetFlashcards > 0 ? dailyMission.targetFlashcards : 5);
    printf("Games Played  : %d / %d\n", dailyMission.gamesPlayed,
           dailyMission.targetGames > 0 ? dailyMission.targetGames : 1);
    printf("\n");

    // Top-5 hardest words sorted by wrongCount
    setColor(12); printf("TOP 5 HARDEST WORDS (sorted by wrong count)\n"); setColor(7);
    printf("----------------------------------------\n");
    int anyHard = 0;
    for (int k = 0; k < 5; k++) {
        if (top5[k] != NULL && top5[k]->wrongCount > 0) {
            anyHard = 1;
            printf("  %d. %-15s wrong: %d  status: %s\n", k + 1,
                   top5[k]->word, top5[k]->wrongCount,
                   top5[k]->learned ? "learned" : "not learned");
        }
    }
    if (!anyHard) { setColor(10); printf("  No wrong answers yet! Keep it up!\n"); setColor(7); }
    printf("----------------------------------------\n");
    pauseScreen();
}

void saveProgress(HashTable* ht) {
    char path[100];
    snprintf(path, sizeof(path), "data/Users/%s/progress.txt", currentUser.username);
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
    snprintf(path, sizeof(path), "data/Users/%s/progress.txt", currentUser.username);
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

Word* getStudiedWord(HashTable* ht) {
    Word* studiedWords[1000];
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            // Include words that have been learned or answered wrong
            if (temp->learned == 1 || temp->wrongCount > 0) {
                if (count < 1000) studiedWords[count++] = temp;
            }
            temp = temp->next;
        }
    }
    if (count == 0) return getRandomWord(ht); // fallback
    int randomIndex = rand() % count;
    return studiedWords[randomIndex];
}

void addWord(HashTable* ht) {
    char word[50], meaning[500], pronunciation[50], type[20];
    char confirm = 'n';

    // Fix 3: confirmation loop - allow correction before submitting
    do {
        printf("Enter the word: ");
        scanf("%49s", word);
        getchar();
        _strlwr(word); // Standardize to lowercase
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
    _strlwr(type);

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
    _strlwr(type);
    
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
    _strlwr(target);
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
    // Keep BST in sync
    ht->bstRoot = bstDelete(ht->bstRoot, temp->word);
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
