#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <time.h>

#include "../include/file.h"
#include "../include/utils.h"
#include "../include/validator.h"
//mở file data/users.txt, duyệt và kiểm tra tên đki
//ngăn vc đki trùng tên
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
//gọi mkdir tạo thư mục data/users/ và thư mục cá nhân data/Users/username
// đảm bảo thư mục lưu trữ dulieu cá nhân
static void ensureUserFolder(const char *username) {
    char folderPath[100];

    mkdir("data/Users");
    snprintf(folderPath, sizeof(folderPath), "data/Users/%s", username);
    mkdir(folderPath);
}
//đọc file dulieu gốc, phân tách dữ liệu bằng dấu |
//tải cơ sở dữ liệu từ điển vào bộ nhớ ram khi mở chương trình
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
            if ((unsigned char)word[0] == 0xEF &&
                (unsigned char)word[1] == 0xBB &&
                (unsigned char)word[2] == 0xBF) {
                word += 3;
            }
            _strlwr(word);
            _strlwr(type);
            Word *newWord = createWord(word, meaning, pronunciation, type);
            insertWord(ht, newWord);
        }
    }
    fclose(file);
}
// Tạo đường dẫn file userdata.txt riêng của user và ghi đè thông tin cấu trúc nhiệm vụ ngày, streak, level, EXP của user đó vào file.
//Lưu trữ vĩnh viễn các thông số thành tích cá nhân của người dùng.
void saveUserData() {
    char path[100];
    ensureUserFolder(currentUser.username);
    snprintf(path, sizeof(path), "data/Users/%s/userdata.txt", currentUser.username);
    FILE* file = fopen(path, "w");
    if (file == NULL) {
        fprintf(stderr, "Could not save user data for %s\n", currentUser.username);
        return;
    }
    
    fprintf(file, "MISSION %d %d %d %d %d %d\n",
        dailyMission.wordsLearnedToday, dailyMission.flashcardsReviewed, dailyMission.gamesPlayed,
        dailyMission.targetWords, dailyMission.targetFlashcards, dailyMission.targetGames);
    fprintf(file, "MISSIONDATE %s\n", dailyMission.missionDate);
    fprintf(file, "STREAK %s %d\n", studyStreak.lastStudyDate, studyStreak.streakDays);
    fprintf(file, "STATS %d %d\n", playStats.level, playStats.exp);
    fclose(file);
}
//Khởi tạo các giá trị mặc định của người dùng, mở file userdata.txt riêng của user để đọc và gán lại các thông số nhiệm vụ ngày, streak, level, EXP tương ứng.
//Khôi phục trạng thái thành tích cá nhân của người dùng khi đăng nhập.
void loadUserData() {
    dailyMission.wordsLearnedToday = 0;
    dailyMission.flashcardsReviewed = 0;
    dailyMission.gamesPlayed = 0;
    dailyMission.targetWords = 0;    // 0 = chưa thiết lập, sẽ tạo sau 
    dailyMission.targetFlashcards = 0;
    dailyMission.targetGames = 0;
    dailyMission.missionDate[0] = '\0'; // chưa được thiết lập

    strcpy(studyStreak.lastStudyDate, "");
    studyStreak.streakDays = 0;

    playStats.level = 1;
    playStats.exp = 0;

    char path[100];
    snprintf(path, sizeof(path), "data/Users/%s/userdata.txt", currentUser.username);
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        // Người dùng lần đầu — chưa có tệp nào, cài đặt mặc định
        return;
    }
    char label[50];
    while (fscanf(file, "%49s", label) == 1) {
        if (strcmp(label, "MISSION") == 0) {
            int r = fscanf(file, "%d %d %d %d %d %d",
                &dailyMission.wordsLearnedToday, &dailyMission.flashcardsReviewed,
                &dailyMission.gamesPlayed,
                &dailyMission.targetWords, &dailyMission.targetFlashcards, &dailyMission.targetGames);
            (void)r;
        } else if (strcmp(label, "MISSIONDATE") == 0) {
            fscanf(file, "%19s", dailyMission.missionDate);
        } else if (strcmp(label, "STREAK") == 0) {
            fscanf(file, "%19s %d", studyStreak.lastStudyDate, &studyStreak.streakDays);
        } else if (strcmp(label, "STATS") == 0) {
            fscanf(file, "%d %d", &playStats.level, &playStats.exp);
        }
    }
    fclose(file);
}
//Yêu cầu nhập tài khoản/mật khẩu, kiểm tra tài khoản admin mặc định hoặc duyệt file users.txt để xác thực thông tin. Nếu khớp, nạp dữ liệu cá nhân của user đó và trả về 1 (thành công).
//Xác thực quyền đăng nhập của người dùng.
int login() {
    clearScreen();
    setColor(11); 
    printf("╔═══════════════════════════════════════════════╗\n");
    setColor(14);
    printf("║               🔐 LOGIN  🔐                    ║\n");
    setColor(11);
    printf("╚═══════════════════════════════════════════════╝\n");
    setColor(7);
    printf("\n");
    
    char username[50];
    char password[50];
    
    setColor(14);
    printf("  📝 Username : ");
    setColor(7);
    scanf("%49s", username);
    printf("\n");
    
    setColor(14);
    printf("  🔑 Password : ");
    setColor(7);
    scanf("%49s", password);
    printf("\n");

    if(
        strcmp(username, "admin") == 0 && strcmp(password, "admin") == 0
    ) {
        strcpy(currentUser.username, "admin");
        strcpy(currentUser.password, "admin");
        ensureUserFolder(currentUser.username);
        loadUserData();
        setColor(10);
        printf("  ✓ Admin login successful!\n");
        setColor(7);
        return 1; // Admin login successful
    }

    FILE *file = fopen("data/users.txt", "r");
    if (file == NULL) {
        setColor(12);
        fprintf(stderr, "  ✗ Could not open user file\n");
        setColor(7);
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
            setColor(10);
            printf("  ✓ Login successful!\n");
            setColor(7);
            return 1; // Login successful
        }
    }
    fclose(file);
    return 0; // Login failed
}
// ycau nhập username và password hợp lệ thông qua validator. ghi tk mới vào users.txt, tạo folder riêng
//Đky tkhoan ng dùng mới
int registerUser() {
    int usernameValid = 0, passwordValid = 0;
    char username[50];
    char password[50];
    
    clearScreen();
    setColor(11);
    printf("╔═══════════════════════════════════════════════╗\n");
    setColor(14);
    printf("║              📝 REGISTER 📝                  ║\n");
    setColor(11);
    printf("╚═══════════════════════════════════════════════╝\n");
    setColor(7);
    printf("\n");

    while (!usernameValid) {
        setColor(14);
        printf("  Enter new Username : ");
        setColor(7);
        scanf("%49s", username);
        printf("\n");
        if (strlen(username) < 3) {
            setColor(12);
            printf("  ✗ Username must be at least 3 characters.\n\n");
            setColor(7);
        } else if (usernameExists(username)) {
            setColor(12);
            printf("  ✗ Username already exists. Please choose another.\n\n");
            setColor(7);
        } else {
            usernameValid = 1;
        }
    }

    while (!passwordValid) {
        setColor(14);
        printf("  Enter new Password : ");
        setColor(7);
        scanf("%49s", password);
        printf("\n");
        if (!isStrongPassword(password)) {
            setColor(12);
            printf("  ✗ Password invalid. Must have 6+ chars, uppercase, lowercase, and a digit.\n\n");
            setColor(7);
        } else {
            passwordValid = 1;
        }
    }

    FILE *file = fopen("data/users.txt", "a");
    if (file == NULL) {
        setColor(12);
        fprintf(stderr, "  ✗ Could not open user file\n");
        setColor(7);
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
        fprintf(userDataFile, "MISSION 0 0 0 0 0 0\n");
        fprintf(userDataFile, "MISSIONDATE 0000-00-00\n");
        fprintf(userDataFile, "STREAK 0000-00-00 0\n");
        fprintf(userDataFile, "STATS 1 0\n");
        fclose(userDataFile);
    }

    char progressPath[100];
    snprintf(progressPath, sizeof(progressPath), "data/Users/%s/progress.txt", username);
    FILE *progressFile = fopen(progressPath, "w");
    if (progressFile != NULL) {
        fclose(progressFile);
    }

    strcpy(currentUser.username, username);
    strcpy(currentUser.password, password);
    loadUserData();
    
    printf("\n");
    setColor(10);
    printf("  ✓ Registration successful!\n");
    setColor(7);
    return 1; 
}
// so sánh tiến độ, thỏa mãn thì sẽ tăng streak lên 1, cập nhật hc lần cuối và save
//tự động kiểm tra và ghi nhận chuỗi ngày streak học tập
void checkAndCompleteMission(){
    int tw = dailyMission.targetWords > 0 ? dailyMission.targetWords : 10;
    int tf = dailyMission.targetFlashcards > 0 ? dailyMission.targetFlashcards : 5;
    int tg = dailyMission.targetGames > 0 ? dailyMission.targetGames : 1;

    saveUserData();

    if (dailyMission.wordsLearnedToday >= tw &&
        dailyMission.flashcardsReviewed >= tf &&
        dailyMission.gamesPlayed >= tg) {

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        char today[20];
        strftime(today, sizeof(today), "%Y-%m-%d", &tm);
        // Streak: only update if haven't counted today yet
        if (strcmp(studyStreak.lastStudyDate, today) != 0) {
            studyStreak.streakDays++;
            strcpy(studyStreak.lastStudyDate, today);
            setColor(10);
            printf("\n*** DAILY MISSION COMPLETE! Streak: %d days! ***\n", studyStreak.streakDays);
            setColor(7);
            saveUserData(); // Persist streak immediately
        }
    }
}

// Nếu phát hiện ngày mới, reset tiến trình ngày về 0 và khởi tạo mục tiêu ngẫu nhiên mới (từ 5-15 từ, 3-8 flashcards, 1-3 game).
//tự động làm mới nv
void resetDailyMissionIfNewDay(){
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char today[20];
    strftime(today, sizeof(today), "%Y-%m-%d", &tm);

    int isNewDay = (strlen(dailyMission.missionDate) > 0 &&
                    strcmp(dailyMission.missionDate, today) != 0 &&
                    strcmp(dailyMission.missionDate, "0000-00-00") != 0);

    if (isNewDay) {
        
        dailyMission.wordsLearnedToday = 0;
        dailyMission.flashcardsReviewed = 0;
        dailyMission.gamesPlayed = 0;
        dailyMission.targetWords      = rand() % 11 + 5;  // 5-15
        dailyMission.targetFlashcards = rand() % 6  + 3;  // 3-8
        dailyMission.targetGames      = rand() % 3  + 1;  // 1-3
        strcpy(dailyMission.missionDate, today);
        saveUserData(); 
        return;
    }

    // Lần đầu tiên (không có ngày tháng hoặc mục tiêu): tạo mục tiêu và ghi lại ngày tháng.
    if (dailyMission.targetWords == 0 || strlen(dailyMission.missionDate) == 0 ||
        strcmp(dailyMission.missionDate, "0000-00-00") == 0) {
        if (dailyMission.targetWords == 0)
            dailyMission.targetWords = rand() % 11 + 5;
        if (dailyMission.targetFlashcards == 0)
            dailyMission.targetFlashcards = rand() % 6 + 3;
        if (dailyMission.targetGames == 0)
            dailyMission.targetGames = rand() % 3 + 1;
        strcpy(dailyMission.missionDate, today);
        saveUserData();
    }
}
