#ifndef DICTIONARY_H
#define DICTIONARY_H

typedef struct Word{
    char word[50];
    char meaning[200];
    char pronunciation[50];
    char type[20];

    int learned;
    int wrongCount;
    int isFavorite;

    struct Word *next;
}Word;

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
} User;

extern User currentUser;

void addWord(
    Word** head
);

void editWord(
    Word *head
);

void deleteWord(
    Word **head
);

void saveDictionary(
    Word *head
);
Word*getWordByType(Word *head, char *type);
Word* getWeakWord(Word *head);
void insertWord(Word **head, Word *newWord);
void displayDictionary(Word *head);
Word* searchWord(Word *head, char *target);
int suggestWords(Word *head, char *prefix, Word *suggestions[20]);
Word* getRandomWord(Word *head);
void freeList(Word *head);
Word* getAdaptiveWord(Word *head);
void showStats(Word *head);
void saveProgress(Word *head);
void loadProgress(Word *head);

#endif // DICTIONARY_H