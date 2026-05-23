#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <time.h>

#include "../include/file.h"
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
    fprintf(file, "MISSION %d %d %d\n", dailyMission.wordsLearnedToday, dailyMission.flashcardsReviewed, dailyMission.gamesPlayed);
    fprintf(file, "STREAK %s %d\n", studyStreak.lastStudyDate, studyStreak.streakDays);
    fprintf(file, "STATS %d %d\n", playStats.level, playStats.exp);
    fclose(file);
}

void loadUserData() {
    // Reset dữ liệu global trước khi load
    dailyMission.wordsLearnedToday = 0;
    dailyMission.flashcardsReviewed = 0;
    dailyMission.gamesPlayed = 0;
    
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
    scanf("%49s", username);
    printf("Password: ");
    scanf("%49s", password);

    if(
        strcmp(username, "admin") == 0 && strcmp(password, "admin") == 0
    ) {
        strcpy(currentUser.username, "admin");
        strcpy(currentUser.password, "admin");
        ensureUserFolder(currentUser.username);
        loadUserData();
        updateStudyStreak();
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
            updateStudyStreak();
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
    
    // Nhập username cho đến khi hợp lệ
    while (!usernameValid) {
        printf("Choose a username: ");
        if (fgets(username, sizeof(username), stdin) == NULL) {
            return 0;
        }
        // Xóa ký tự newline
        username[strcspn(username, "\n")] = '\0';
        
        if (!isValidUsername(username)) {
            printf("Tên người dùng không hợp lệ. Phải có độ dài từ 3 đến 50 ký tự.\n");
        } else if (usernameExists(username)) {
            printf("Username already exists. Please choose another username.\n");
        } else {
            usernameValid = 1;
        }
    }
    
    // Nhập password cho đến khi hợp lệ
    while (!passwordValid) {
        printf("Choose a password: ");
        if (fgets(password, sizeof(password), stdin) == NULL) {
            return 0;
        }
        // Xóa ký tự newline
        password[strcspn(password, "\n")] = '\0';
        
        if (!isStrongPassword(password)) {
            printf("Mật khẩu không hợp lệ. Mật khẩu phải có ít nhất 6 ký tự và bao gồm chữ hoa, chữ thường và một chữ số.\n");
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


void updateStudyStreak(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char today[20];
    strftime(today, sizeof(today), "%Y-%m-%d", &tm);
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
