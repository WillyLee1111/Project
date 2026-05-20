#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <time.h>

#include "../include/file.h"
#include "../include/validator.h"

void loadDictionary(Word **head) {
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
            Word *newWord = createWord(word, meaning, pronunciation, type);
            insertWord(head, newWord);
        }
    }
    fclose(file);
}

void saveUserData() {
    char path[100];
    sprintf(path, "data/users/%s/userdata.txt", currentUser.username);
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        return;
    }
    fprintf(file, "MISSION %d %d %d\n", dailyMission.wordsLearnedToday, dailyMission.flashcardsReviewed, dailyMission.gamesPlayed);
    fprintf(file, "STREAK %s %d\n", studyStreak.lastStudyDate, studyStreak.streakDays);
    fprintf(file, "STATS %d %d\n", playStats.level, playStats.exp);
    fclose(file);
}

void loadUserData() {
    char path[100];
    sprintf(path, "data/users/%s/userdata.txt", currentUser.username);
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        return;
    }
    char label[50];
    while(
        fscanf(file, "%s", label) == 1
    ) {
        if (strcmp(label, "MISSION") == 0) {
            fscanf(file, "%d %d %d\n", &dailyMission.wordsLearnedToday, &dailyMission.flashcardsReviewed, &dailyMission.gamesPlayed);
        } else if (strcmp(label, "STREAK") == 0) {
            fscanf(file, "%s %d\n", studyStreak.lastStudyDate, &studyStreak.streakDays);
        } else if (strcmp(label, "STATS") == 0) {
            fscanf(file, "%d %d\n", &playStats.level, &playStats.exp);
        }
    }
    fclose(file);
}

int login() {
    char username[50];
    char password[50];
    printf("Username: ");
    scanf("%s", username);
    printf("Password: ");
    scanf("%s", password);

    if(
        strcmp(username, "admin") == 0 && strcmp(password, "admin") == 0
    ) {
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
        sscanf(line, "%[^|]|%s", storedUsername, storedPassword);
        if (strcmp(username, storedUsername) == 0 && strcmp(password, storedPassword) == 0) {
            strcpy(currentUser.username, storedUsername);
            strcpy(currentUser.password, storedPassword);
            updateStudyStreak();
            fclose(file);
            return 1; // Login successful
        }
    }
    fclose(file);
    return 0; // Login failed
}
int registerUser() {
    char username[50];
    char password[50];
    printf("Choose a username: ");
    scanf("%s", username);
    if (!isValidUsername(username)) {
        printf("Invalid username. Must be 3-50 characters.\n");
        return 0;
    }
    printf("Choose a password: ");
    scanf("%s", password);
    if (!isStrongPassword(password)) {
        printf("Invalid password. Must be at least 6 characters long and contain uppercase, lowercase, and a digit.\n");
        return 0;
    }

    FILE *file = fopen("data/users.txt", "a");
    if (file == NULL) {
        fprintf(stderr, "Could not open user file\n");
        return 0;
    }
    fprintf(file, "%s|%s\n", username, password);
    char folderPath[100];
    sprintf(folderPath, "data/users/%s", username);
    mkdir(folderPath);
    fclose(file);
    return 1; // Registration successful
}


void updateStudyStreak(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char today[20];
    sprintf(
        today,
        "%04d-%02d-%02d",
        tm.tm_year + 1900,
        tm.tm_mon + 1,
        tm.tm_mday
    );
    if(
        strcmp(
            studyStreak.lastStudyDate,
            today
        ) != 0
    ){
        studyStreak.streakDays++;
        strcpy(
            studyStreak.lastStudyDate,
            today
        );
    }
}