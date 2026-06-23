#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#include "../include/dictionary.h"
#include "../include/file.h"
#include "../include/flashcard.h"
#include "../include/game.h"
#include "../include/ui.h"
#include "../include/utils.h"

Mission dailyMission = {0, 0, 0, 0, 0, 0, ""};
Streak studyStreak = {"", 0};
PlayStats playStats = {0, 0};
User currentUser;

static HashTable *globalHt = NULL;
//gọi hàm lưu tiến độ trc khi thoát
//tránh mất dl khi đóng đột ngột
BOOL WINAPI consoleHandler(DWORD signal) {
  if ((signal == CTRL_CLOSE_EVENT || signal == CTRL_C_EVENT) &&
      globalHt != NULL) {
    saveProgress(globalHt);
    saveUserData();
    saveDictionary(globalHt);
  }
  return FALSE; // Let default handler terminate
}

// Tính toán tỷ lệ, vẽ ký tự = cho phần đã xong, > cho phần đang làm và khoảng trắng cho phần còn lại trong khung thanh tiến trình dài 10 ký tự.
// vẽ hthanh tiếng độ
static void drawBar(int current, int total) {
  int width = 10;
  int filled = (total > 0) ? (current * width / total) : 0;
  if (filled > width)
    filled = width;
  printf("[");
  for (int i = 0; i < width; i++) {
    if (i < filled)
      printf("=");
    else if (i == filled && current < total)
      printf(">");
    else
      printf(" ");
  }
  printf("]");
}

// in thông số chi tiết của từ
//cung cấp giao diện hiển thị thông tin từ khi tra cứu
static void showWordDetail(Word *w) {
  if (w == NULL)
    return;
  clearScreen();
  
  setColor(11);
  printf("═════════════════════════════════════════\n");
  setColor(14);
  printf("           📖 WORD DETAIL 📖              \n");
  setColor(11);
  printf("═════════════════════════════════════════\n");
  setColor(7);
  printf("\n");
  
  setColor(14);
  printf("Word          : ");
  setColor(240);
  printf(" %s", w->word);
  for (int i = getVisualWidth(w->word); i < 30; i++) printf(" ");
  printf(" ");
  setColor(7);
  printf("\n\n");
  
  setColor(10);
  printf("Pronunciation : ");
  setColor(7);
  printf("[%s]\n", w->pronunciation);
  
  setColor(10);
  printf("Type          : ");
  setColor(7);
  printf("%s\n", w->type);
  
  setColor(13);
  printf("Status        : ");
  if (w->learned) {
    setColor(10);
    printf("✓ Learned");
  } else {
    setColor(12);
    printf("✗ Not learned");
  }
  setColor(7);
  printf(" | Wrong: ");
  if (w->wrongCount >= 3) {
    setColor(12);
  } else if (w->wrongCount > 0) {
    setColor(14);
  } else {
    setColor(10);
  }
  printf("%d\n", w->wrongCount);
  setColor(7);
  printf("\n");
  
  setColor(14);
  printf("Meaning(s):\n");
  setColor(7);
  char copy[500];
  strncpy(copy, w->meaning, sizeof(copy) - 1);
  copy[sizeof(copy) - 1] = '\0';
  char *tok = strtok(copy, ";");
  int n = 1;
  while (tok) {
    printf("  %d. %s\n", n++, tok);
    tok = strtok(NULL, ";");
  }
}
//chạy vòng lặp điều hướng menu từ điển, chạy liên tục đến khi nhấn enter
// qly giao diện và điều hướng các chức năng quản lý
void dictionaryMenu(HashTable *ht) {
  int choice;
  do {
    char *dictOptions[] = {
        "🔍 Show & Search Dictionary",
        "🎲 Random Word",
        "➕ Add Word",
        "✏️  Edit Word",
        "🗑️  Delete Word",
        "Back"
    };
    choice = selectMenu("📚 DICTIONARY MENU 📚", dictOptions, 6, NULL);
    clearScreen();
    if (choice == 5) choice = 0;
    else choice += 1;

    switch (choice) {
    case 1: {
      int stayInSearch = 1;
      while (stayInSearch) {
        clearScreen();
        setColor(11);
        printf("═════════════════════════════════════════════\n");
        setColor(14);
        printf("              📚 DICTIONARY 📚              \n");
        setColor(11);
        printf("═════════════════════════════════════════════\n");
        setColor(7);
        printf("\n");
        displayDictionaryBoxed(ht);
        printf("\n");
        pauseScreen();
        clearScreen();

        printf("\n");
        setColor(14);
        printf("════════════════════════════════════════════\n");
        printf("  Type a prefix to search (Enter = go back): ");
        setColor(7);
        char prefix[50];
        if (!fgets(prefix, sizeof(prefix), stdin)) break;
        prefix[strcspn(prefix, "\r\n")] = '\0';

        if (strlen(prefix) == 0) {
          stayInSearch = 0;
          break;
        }

        Word *suggestions[20];
        int count = suggestWords(ht, prefix, suggestions);
        clearScreen();
        
        setColor(11);
        printf("════════════════════════════════════════════\n");
        setColor(14);
        printf("  🔍 Search: \"%s\"  [Found: %d result(s)]\n", prefix, count);
        setColor(11);
        printf("════════════════════════════════════════════\n");
        setColor(7);
        printf("\n");

        if (count == 0) {
          setColor(12);
          printf("  ✗ No words found with prefix \"%s\".\n", prefix);
          setColor(7);
          pauseScreen();
        } else if (count == 1) {
          setColor(10);
          printf("  ✓ Auto-showing the only match found.\n\n");
          setColor(7);
          showWordCardBoxed(suggestions[0]);
          pauseScreen();
        } else {
          // xây dựng và sd nút điều hướng
          char *searchOpts[21];
          char labels[20][80];
          for (int i = 0; i < count; i++) {
            strcpy(labels[i], suggestions[i]->word);
            for (int j = getVisualWidth(suggestions[i]->word); j < 20; j++) strcat(labels[i], " ");
            strcat(labels[i], " (");
            strcat(labels[i], suggestions[i]->type);
            strcat(labels[i], ")");
            searchOpts[i] = labels[i];
          }
          searchOpts[count] = "Back";

          char searchTitle[80];
          snprintf(searchTitle, sizeof(searchTitle),
                   "Found %d result(s) for \"%s\"", count, prefix);

          int chosen = selectMenu(searchTitle, searchOpts, count + 1, NULL);
          if (chosen < count) {
            showWordCardBoxed(suggestions[chosen]);
            pauseScreen();
          }
        }
      }
      break;
    }
    case 2: {
      Word *randomWord = getRandomWord(ht);
      if (randomWord != NULL) {
        showWordDetail(randomWord);
      } else {
        setColor(12);
        printf("No words available.\n");
        setColor(7);
      }
      pauseScreen();
      break;
    }
    case 3:
      addWord(ht);
      saveDictionary(ht);
      pauseScreen();
      break;
    case 4:
      editWord(ht);
      saveDictionary(ht);
      pauseScreen();
      break;
    case 5:
      deleteWord(ht);
      saveDictionary(ht);
      pauseScreen();
      break;
    }
  } while (choice != 0);
}

void gameMenu(HashTable *ht) {
  int gameChoice;
  do {
    char *gameOptions[] = {
        "🇬🇧 English -> Vietnamese",
        "🇻🇳 Vietnamese -> English",
        "🔤 Missing Letter",
        "Back"
    };
    gameChoice = selectMenu("🎮 GAME CENTER 🎮", gameOptions, 4, NULL);
    clearScreen();
    if (gameChoice == 3) gameChoice = 0;
    else gameChoice += 1;
    switch (gameChoice) {
    case 1:
      setColor(14);
      printf("╔════════════════════════════════════════════╗\n");
      printf("║  🇬🇧 ENGLISH -> VIETNAMESE GAME 🇬🇧          ║\n");
      printf("╚════════════════════════════════════════════╝\n");
      setColor(7);
      printf("\n");
      englishToVietnameseGame(ht);
      break;
    case 2:
      setColor(14);
      printf("╔════════════════════════════════════════════╗\n");
      printf("║  🇻🇳 VIETNAMESE -> ENGLISH GAME 🇻🇳          ║\n");
      printf("╚════════════════════════════════════════════╝\n");
      setColor(7);
      printf("\n");
      vietnameseToEnglishGame(ht);
      break;
    case 3:
      setColor(14);
      printf("╔════════════════════════════════════════════╗\n");
      printf("║       🔤 MISSING LETTER GAME 🔤            ║\n");
      printf("╚════════════════════════════════════════════╝\n");
      setColor(7);
      printf("\n");
      playMissingLetterGame(ht);
      break;
    case 0:
      return;
    default:
      printError("Invalid choice!");
      pauseScreen();
    }
  } while (gameChoice != 0);
  checkAndCompleteMission();
  saveUserData();
}
//in thông tin nv hằng ngày
//hiển thị bảng tóm tắt nv ở trên
void printMainMenuHeader() {
  int tw = dailyMission.targetWords;
  int tf = dailyMission.targetFlashcards;
  int tg = dailyMission.targetGames;

  setColor(13);
  printf("╔═══════════════════════════════════════════╗\n");
  printf("║        📋 DAILY MISSION STATUS 📋         ║\n");
  printf("╚═══════════════════════════════════════════╝\n");
  setColor(7);
  
  // Words Learned - Green
  setColor(10);
  printf("📚 Words Learned  : %2d / %2d  ", dailyMission.wordsLearnedToday, tw);
  drawBar(dailyMission.wordsLearnedToday, tw);
  printf("%s\n", dailyMission.wordsLearnedToday >= tw ? " ✓ DONE" : "");
  setColor(7);

  // Flashcards - Cyan
  setColor(11);
  printf("🎴 Flashcards     : %2d / %2d  ", dailyMission.flashcardsReviewed, tf);
  drawBar(dailyMission.flashcardsReviewed, tf);
  printf("%s\n", dailyMission.flashcardsReviewed >= tf ? " ✓ DONE" : "");
  setColor(7);

  // Games Played - Yellow
  setColor(14);
  printf("🎮 Games Played   : %2d / %2d  ", dailyMission.gamesPlayed, tg);
  drawBar(dailyMission.gamesPlayed, tg);
  printf("%s\n", dailyMission.gamesPlayed >= tg ? " ✓ DONE" : "");
  setColor(7);

  // Study Streak - Magenta
  setColor(13);
  printf("🔥 Study Streak   : %d days\n", studyStreak.streakDays);
  setColor(7);
  
  printf("═════════════════════════════════════════════\n");

  printHeader("ENGLISH DICTIONARY");
  showMiniPlayerCard();
  printf("\n");
}

int main() {
  showIntroScreen();
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
  clearScreen();
  srand((unsigned int)time(NULL));

  int option;
  while (1) {
    char *introOptions[] = {
        "🔓 Login",
        "📝 Register",
        "🚪 Exit"
    };
    int rawChoice = selectMenu("🌍 ENGLISH DICTIONARY & VOCAB GAME🌍", introOptions, 3, NULL);
    if (rawChoice == 0) option = 1;
    else if (rawChoice == 1) option = 2;
    else option = 0;

    if (option == 1) {
      if (login()) {
        break; // Proceed to main menu
      } else {
        setColor(12);
        printf("\nLogin failed. Incorrect username or password.\n");
        setColor(7);
        pauseScreen();
      }
    } else if (option == 2) {
      if (registerUser()) {
        setColor(10);
        printf("\nRegistration successful! Please login now.\n");
        setColor(7);
        pauseScreen();
      } else {
        setColor(12);
        printf("\nRegistration failed.\n");
        setColor(7);
        pauseScreen();
      }
    } else if (option == 0) {
      printf("\nExiting program... Goodbye!\n");
      exit(0);
    } else {
      setColor(12);
      printf("\nInvalid option. Please try again.\n");
      setColor(7);
      pauseScreen();
    }
  }

  SetConsoleOutputCP(CP_UTF8);

  resetDailyMissionIfNewDay();
  saveUserData();

  HashTable *ht = createHashTable();
  loadDictionary(ht);
  loadProgress(ht);

  
  globalHt = ht;
  SetConsoleCtrlHandler(consoleHandler, TRUE);

  int running = 1;
  int choice;
  while (running) {
    char *mainOptions[] = {
        "📚 Dictionary Menu",
        "🎴 Flashcard Mode",
        "🎮 Game Center",
        "📊 Show Stats",
        "🚪 Exit"
    };
    choice = selectMenu("", mainOptions, 5, printMainMenuHeader);
    choice += 1;
    clearScreen();

    switch (choice) {
    case 1:
      dictionaryMenu(ht);
      break;
    case 2:
      flashcardMode(ht);
      checkAndCompleteMission();
      saveUserData();
      break;
    case 3:
      gameMenu(ht);
      break;
    case 4:
      showStats(ht);
      break;
    case 5: {
      
      printf("Are you sure you want to exit? (y/n): ");
      char c;
      scanf(" %c", &c);
      while(getchar() != '\n');
      if (c == 'y' || c == 'Y') {
        saveProgress(ht);
        saveUserData();
        saveDictionary(ht);
        setColor(10);
        printf("\nData saved successfully. Goodbye!\n");
        setColor(7);
        pauseScreen();
        running = 0;
      }
      break;
    }
    default:
      printf("Invalid choice.\n");
      pauseScreen();
      break;
    }
  }
  freeHashTable(ht);
  globalHt = NULL;
  return 0;
}
