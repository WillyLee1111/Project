#ifndef DICTIONARY_H
#define DICTIONARY_H

#define HASH_SIZE 10007

typedef struct Word{
    char word[50];
    char meaning[500]; // Supports multiple meanings separated by ';'
    char pronunciation[50];
    char type[20];

    int learned;
    int wrongCount;
    int isFavorite;

    struct Word *next;
}Word;

// BST node used for O(log n) prefix-based word suggestion
typedef struct BSTNode {
    char word[50];
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

// Dictionary storage: Hash Table (O(1) lookup) + BST (O(log n) prefix suggestion)
typedef struct HashTable {
    Word *buckets[HASH_SIZE];
    BSTNode *bstRoot;   // BST for sorted prefix suggestions
} HashTable;

HashTable* createHashTable();
unsigned int hashFunction(char* word);

Word* createWord(
    char*word, 
    char *meaning, 
    char *pronunciation, 
    char *type
);

typedef struct Mission {
    int wordsLearnedToday;
    int flashcardsReviewed;
    int gamesPlayed;
    int targetWords;      // randomized target each day
    int targetFlashcards;
    int targetGames;
} Mission;

extern Mission dailyMission;

typedef struct Streak {
    char lastStudyDate[20]; // Format: YYYY-MM-DD
    int streakDays;
} Streak;

extern Streak studyStreak;

typedef struct User {
    char username[50];
    char password[50];
    int id;
} User;

extern User currentUser;

void addWord(HashTable* ht);
void editWord(HashTable* ht);
void deleteWord(HashTable* ht);
void saveDictionary(HashTable* ht);

typedef struct PlayStats {
    int exp;
    int level;
} PlayStats;
extern PlayStats playStats;

Word* getWordByType(HashTable* ht, char *type);
Word* getWeakWord(HashTable* ht);
void insertWord(HashTable* ht, Word *newWord);
void displayDictionary(HashTable* ht);
Word* searchWord(HashTable* ht, char *target);
int suggestWords(HashTable* ht, char *prefix, Word *suggestions[20]);
// BST operations (used internally by suggestWords)
BSTNode* bstInsert(BSTNode *root, const char *word);
BSTNode* bstDelete(BSTNode *root, const char *word);
void     freeBST(BSTNode *root);
Word* getRandomWord(HashTable* ht);
void freeHashTable(HashTable* ht);
Word* getAdaptiveWord(HashTable* ht);
Word* getAdaptiveWordMinLen(HashTable* ht, int minLen);
void showStats(HashTable* ht);
void printWordMeanings(const char *meaning);
void saveProgress(HashTable* ht);
void loadProgress(HashTable* ht);
void updateLevel();

#endif // DICTIONARY_H