#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <time.h>

#include "../include/file.h"
#include "../include/utils.h"
#include "../include/validator.h"

static int usernameExists(const char *username) {
    FILE *file = fopen("data/users.txt", "r");
    if (file == NULL) {
        return 0;
    }

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        char storedUsername[50];
        if (sscanf(line, "%49[^|]|%*s", storedUsername) == 1 &&
            strcmp(username, storedUsername) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

static void ensureUserFolder(const char *username) {
    char folderPath[100];

    mkdir("data/Users");
    snprintf(folderPath, sizeof(folderPath), "data/Users/%s", username);
    mkdir(folderPath);
}

void loadDictionary(HashTable *ht) {
    FILE* file = fopen("data/dictionary.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file\n");
        exit(EXIT_FAILURE);
    }
    char line[400];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = '\0'; // Remove newline character
        char *word = strtok(line, "|");
        char *meaning = strtok(NULL, "|");
        char *pronunciation = strtok(NULL, "|");
        char *type = strtok(NULL, "|");
        
        if (word && meaning && pronunciation && type) {
            _strlwr(word);
            _strlwr(type);
            Word *newWord = createWord(word, meaning, pronunciation, type);
            insertWord(ht, newWord);
        }
    }
    fclose(file);
}

void saveUserData() {
    char path[100];
    ensureUserFolder(currentUser.username);
    snprintf(path, sizeof(path), "data/Users/%s/userdata.txt", currentUser.username);
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        fprintf(stderr, "Could not save user data for %s\n", currentUser.username);
        return;
    }
    // Save 6 mission fields including randomized targets
    fprintf(file, "MISSION %d %d %d %d %d %d\n",
        dailyMission.wordsLearnedToday, dailyMission.flashcardsReviewed, dailyMission.gamesPlayed,
        dailyMission.targetWords, dailyMission.targetFlashcards, dailyMission.targetGames);
    fprintf(file, "STREAK %s %d\n", studyStreak.lastStudyDate, studyStreak.streakDays);
    fprintf(file, "STATS %d %d\n", playStats.level, playStats.exp);
    fclose(file);
}

void loadUserData() {
    dailyMission.wordsLearnedToday = 0;
    dailyMission.flashcardsReviewed = 0;
    dailyMission.gamesPlayed = 0;
    dailyMission.targetWords = 0;    // 0 = not set yet, will be generated after srand
    dailyMission.targetFlashcards = 0;
    dailyMission.targetGames = 0;

    strcpy(studyStreak.lastStudyDate, "");
    studyStreak.streakDays = 0;

    playStats.level = 1;
    playStats.exp = 0;

    char path[100];
    snprintf(path, sizeof(path), "data/Users/%s/userdata.txt", currentUser.username);
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not load user data for %s from %s\n", currentUser.username, path);
        return;
    }
    char label[50];
    while (fscanf(file, "%49s", label) == 1) {
        if (strcmp(label, "MISSION") == 0) {
            // Try reading 6 values (new format), fall back to 3 (old format)
            int r = fscanf(file, "%d %d %d %d %d %d",
                &dailyMission.wordsLearnedToday, &dailyMission.flashcardsReviewed,
                &dailyMission.gamesPlayed,
                &dailyMission.targetWords, &dailyMission.targetFlashcards, &dailyMission.targetGames);
            (void)r; // targets remain 0 if old format, resetDailyMissionIfNewDay will fill them
        } else if (strcmp(label, "STREAK") == 0) {
            fscanf(file, "%19s %d", studyStreak.lastStudyDate, &studyStreak.streakDays);
        } else if (strcmp(label, "STATS") == 0) {
            fscanf(file, "%d %d", &playStats.level, &playStats.exp);
        }
    }
    fclose(file);
}

int login() {
    clearScreen();
    printf("=============================================\n");
    setColor(11); printf("                  LOGIN\n"); setColor(7);
    printf("=============================================\n\n");
    char username[50];
    char password[50];
    printf("  Username : ");
    scanf("%49s", username);
    printf("  Password : ");
    scanf("%49s", password);

    if(
        strcmp(username, "admin") == 0 && strcmp(password, "admin") == 0
    ) {
        strcpy(currentUser.username, "admin");
        strcpy(currentUser.password, "admin");
        ensureUserFolder(currentUser.username);
        loadUserData();
        return 1; // Admin login successful
    }

    FILE *file = fopen("data/users.txt", "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open user file\n");
        return 0;
    }
    char line[100];
    while (fgets(line, sizeof(line), file)) {
        char storedUsername[50];
        char storedPassword[50];
        sscanf(line, "%49[^|]|%49s", storedUsername, storedPassword);
        if (strcmp(username, storedUsername) == 0 && strcmp(password, storedPassword) == 0) {
            strcpy(currentUser.username, storedUsername);
            strcpy(currentUser.password, storedPassword);
            ensureUserFolder(currentUser.username);
            loadUserData();
            fclose(file);
            return 1; // Login successful
        }
    }
    fclose(file);
    return 0; // Login failed
}
int registerUser() {
    int usernameValid = 0, passwordValid = 0;
    char username[50];
    char password[50];
    
    clearScreen();
    printf("=============================================\n");
    setColor(11); printf("                REGISTER\n"); setColor(7);
    printf("=============================================\n\n");

    while (!usernameValid) {
        printf("  Enter new Username : ");
        scanf("%49s", username);
        if (strlen(username) < 3) {
            setColor(12); printf("  -> Username must be at least 3 characters.\n\n"); setColor(7);
        } else if (usernameExists(username)) {
            setColor(12); printf("  -> Username already exists. Please choose another.\n\n"); setColor(7);
        } else {
            usernameValid = 1;
        }
    }

    while (!passwordValid) {
        printf("  Enter new Password : ");
        scanf("%49s", password);
        if (!isStrongPassword(password)) {
            setColor(12); printf("  -> Password invalid. Must have 6+ chars, uppercase, lowercase, and a digit.\n\n"); setColor(7);
        } else {
            passwordValid = 1;
        }
    }

    FILE *file = fopen("data/users.txt", "a");
    if (file == NULL) {
        fprintf(stderr, "Could not open user file\n");
        return 0;
    }
    fprintf(file, "%s|%s\n", username, password);
    fclose(file);
    
    ensureUserFolder(username);
    
    // Tạo file userdata.txt với dữ liệu mặc định
    char userDataPath[100];
    snprintf(userDataPath, sizeof(userDataPath), "data/Users/%s/userdata.txt", username);
    FILE *userDataFile = fopen(userDataPath, "w");
    if (userDataFile != NULL) {
        fprintf(userDataFile, "MISSION 0 0 0\n");
        fprintf(userDataFile, "STREAK 0000-00-00 0\n");
        fprintf(userDataFile, "STATS 1 0\n");
        fclose(userDataFile);
    }

    char progressPath[100];
    snprintf(progressPath, sizeof(progressPath), "data/users/%s/progress.txt", username);
    FILE *progressFile = fopen(progressPath, "w");
    if (progressFile != NULL) {
        fclose(progressFile);
    }

    strcpy(currentUser.username, username);
    strcpy(currentUser.password, password);
    loadUserData();
    
    return 1; // Registration successful
}


void checkAndCompleteMission(){
    // Use dynamic random targets (Fix 4)
    int tw = dailyMission.targetWords > 0 ? dailyMission.targetWords : 10;
    int tf = dailyMission.targetFlashcards > 0 ? dailyMission.targetFlashcards : 5;
    int tg = dailyMission.targetGames > 0 ? dailyMission.targetGames : 1;

    if (dailyMission.wordsLearnedToday >= tw &&
        dailyMission.flashcardsReviewed >= tf &&
        dailyMission.gamesPlayed >= tg) {

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        char today[20];
        strftime(today, sizeof(today), "%Y-%m-%d", &tm);
        if (strcmp(studyStreak.lastStudyDate, today) != 0){
            studyStreak.streakDays++;
            strcpy(studyStreak.lastStudyDate, today);
            printf("\n*** DAILY MISSION COMPLETE! Streak: %d days! ***\n", studyStreak.streakDays);
        }
    }
}

// Called after loadUserData(). Resets mission if it's a new day and generates new random targets.
void resetDailyMissionIfNewDay(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char today[20];
    strftime(today, sizeof(today), "%Y-%m-%d", &tm);

    int isNewDay = (strcmp(studyStreak.lastStudyDate, today) != 0 &&
                    strcmp(studyStreak.lastStudyDate, "") != 0 &&
                    strcmp(studyStreak.lastStudyDate, "0000-00-00") != 0);

    if (isNewDay) {
        // New day: reset progress counters
        dailyMission.wordsLearnedToday = 0;
        dailyMission.flashcardsReviewed = 0;
        dailyMission.gamesPlayed = 0;
        // Generate new random targets for the new day (Fix 4)
        dailyMission.targetWords      = rand() % 11 + 5;  // 5-15
        dailyMission.targetFlashcards = rand() % 6  + 3;  // 3-8
        dailyMission.targetGames      = rand() % 3  + 1;  // 1-3
    }

    // Generate targets if still 0 (first time user / old save format)
    if (dailyMission.targetWords == 0)
        dailyMission.targetWords = rand() % 11 + 5;
    if (dailyMission.targetFlashcards == 0)
        dailyMission.targetFlashcards = rand() % 6 + 3;
    if (dailyMission.targetGames == 0)
        dailyMission.targetGames = rand() % 3 + 1;
}
