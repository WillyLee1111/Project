#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/file.h"

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
    FILE *file = fopen("userdata.txt", "wb");
    if (file == NULL) {
        return;}
    fprintf(file, "%d %d %d\n", dailyMission.wordsLearnedToday, dailyMission.flashcardsReviewed, dailyMission.gamesPlayed);
    fprintf(file, "%s %d\n", studyStreak.lastStudyDate, studyStreak.streakDays);
    fclose(file);
}

void loadUserData() {
    FILE *file = fopen("userdata.txt", "rb");
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
    printf("Choose a password: ");
    scanf("%s", password);

    FILE *file = fopen("data/users.txt", "a");
    if (file == NULL) {
        fprintf(stderr, "Could not open user file\n");
        return 0;
    }
    fprintf(file, "%s|%s\n", username, password);
    fclose(file);
    return 1; // Registration successful
}