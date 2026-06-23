#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "../include/utils.h"
#include "../include/dictionary.h"

int getVisualWidth(const char *str) {
    int width = 0;
    while (*str) {
        unsigned char c = (unsigned char)*str;
        if (c < 0x80) {
            width++;
            str++;
        } else if ((c & 0xE0) == 0xC0) {
            width += 1;
            str += 2;
        } else if ((c & 0xF0) == 0xE0) {
            width += 1;
            str += 3;
        } else if ((c & 0xF8) == 0xF0) {
            width += 2;
            str += 4;
        } else {
            width++;
            str++;
        }
    }
    return width;
}

void clearScreen() {
    system("cls"); // Use "cls" on Windows
}
//đưa con troe hiển thị về tọa độ (0,0);
//vẽ đè nội dung mới lên giao diện cũ mà k xóa màn hình
void resetCursor() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = {0, 0};
    SetConsoleCursorPosition(hOut, pos);
}

void pauseScreen() {
    system("pause"); // Use "pause" on Windows
}

void printHeader(char title[]) {
    printf("╔══════════════════════════════════════╗\n");
    printf("║ %s", title);
    for (int i = getVisualWidth(title); i < 36; i++) printf(" ");
    printf(" ║\n");
    printf("╚══════════════════════════════════════╝\n");
    setColor(7); 
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
//hiển thị tính toán phần trăm điểm kinh nghiệm
//vẽ thẻ thông tin người chơi thu nhỏ hiển thị ở đầu menu
void showMiniPlayerCard(){
    int maxExp = playStats.level * 100;
    if(maxExp <= 0) { maxExp = 100;}
    int percent = (playStats.exp * 100) / maxExp;
    int bars = percent / 10;
    setColor(11); // màu xanh dương
    printf("+--------------------+\n");
    printf("| LV : %-13d |\n",playStats.level);
    printf("| EXP: %-13d |\n",playStats.exp);
    printf("| ");
for(int i = 0; i < 14; i++){
    if(i < bars){
        printf("■");
    }
    else {
        printf(" ");
    }
}
    printf("%3d%% |\n",percent);
    printf("+--------------------+\n");
}


void showWordCard(Word *word){
    printf("========================================\n");
    printf("| Word          : %s", word->word);
    for (int i = getVisualWidth(word->word); i < 20; i++) printf(" ");
    printf(" |\n");
    printf("| Meaning       : %s", word->meaning);
    for (int i = getVisualWidth(word->meaning); i < 20; i++) printf(" ");
    printf(" |\n");
    printf("| Pronunciation : %s", word->pronunciation);
    for (int i = getVisualWidth(word->pronunciation); i < 20; i++) printf(" ");
    printf(" |\n");
    printf("| Type          : %s", word->type);
    for (int i = getVisualWidth(word->type); i < 20; i++) printf(" ");
    printf(" |\n");
    printf("========================================\n");
}

void printSuccess(char text[]){
    setColor(10);
    printf("[SUCCESS] %s\n", text);
    setColor(7);
}


void printError(char text[]){
    setColor(12);
    printf("[ERROR] %s\n", text);
    setColor(7);
}

// Hiển thị thẻ từ với khung/box có màu - đơn giản không dùng tính toán
void showWordCardBoxed(Word *word) {
    if (word == NULL) return;
    
    // Chọn màu dựa trên trạng thái từ
    int boxColor = 11;  // Mặc định cyan
    if (word->learned) {
        boxColor = 10;  // Xanh lá nếu đã học
    } else if (word->wrongCount >= 3) {
        boxColor = 12;  // Đỏ nếu sai nhiều
    } else if (word->isFavorite) {
        boxColor = 14;  // Vàng nếu yêu thích
    }
    
    setColor(boxColor);
    printf("┌──────────────────────────────────────────────────┐\n");
    printf("│                                                  │\n");
    
    printf("│ ");
    setColor(240); // White background
    printf("Word: %s", word->word);
    for (int i = getVisualWidth(word->word) + 6; i < 49; i++) printf(" ");
    setColor(boxColor);
    printf("│\n");
    
    printf("│                                                  │\n");
    printf("│ ");
    setColor(7);
    printf("Type: %s", word->type);
    for (int i = getVisualWidth(word->type) + 6; i < 49; i++) printf(" ");
    setColor(boxColor);
    printf("│\n");
    
    printf("│                                                  │\n");
    printf("│ ");
    setColor(10);  // Green for pronunciation
    printf("Pronunciation: [%s]", word->pronunciation);
    for (int i = getVisualWidth(word->pronunciation) + 17; i < 49; i++) printf(" ");
    setColor(boxColor);
    printf("│\n");
    
    printf("│                                                  │\n");
    printf("│ ");
    setColor(14);  // Yellow for meaning
    printf("Meaning: %s", word->meaning);
    for (int i = getVisualWidth(word->meaning) + 9; i < 49; i++) printf(" ");
    setColor(boxColor);
    printf("│\n");
    
    printf("│                                                  │\n");
    printf("└──────────────────────────────────────────────────┘\n");
    setColor(7);
}

// Hiển thị từ điển với các khung/box có màu
void displayDictionaryBoxed(HashTable *ht) {
    int total = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *tmp = ht->buckets[i];
        while (tmp) { total++; tmp = tmp->next; }
    }
    
    if (total == 0) { 
        setColor(12);
        printf("Dictionary is empty.\n");
        setColor(7);
        return; 
    }

    // Collect all words and sort by word
    Word **allWords = (Word**)malloc(total * sizeof(Word*));
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *tmp = ht->buckets[i];
        while (tmp) {
            allWords[count++] = tmp;
            tmp = tmp->next;
        }
    }
    
    // Simple bubble sort
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (_stricmp(allWords[j]->word, allWords[j+1]->word) > 0) {
                Word *temp = allWords[j];
                allWords[j] = allWords[j+1];
                allWords[j+1] = temp;
            }
        }
    }
    
    // Display all words in boxes
    for (int i = 0; i < count; i++) {
        showWordCardBoxed(allWords[i]);
        printf("\n");
    }
    
    setColor(11);
    printf("╔════════════════════════════════════════════════╗\n");
    setColor(7);
    char totalText[50];
    sprintf(totalText, "  Total: %d word(s) [sorted A-Z]", count);
    printf("║%s", totalText);
    for (int i = getVisualWidth(totalText); i < 48; i++) printf(" ");
    printf("║\n");
    setColor(11);
    printf("╚════════════════════════════════════════════════╝\n");
    setColor(7);
    free(allWords);
}
//chạy vòng lặp vẽ menu bằng con trỏ. nhận phím bằng getch, di chuyển bằng up/down
//tạo menu tương tác điều hướng
int selectMenu(char *title, char *options[], int numOptions, void (*printHeaderCallback)(void)) {
    int cursor = 0;
    int key;

    clearScreen(); // Xóa màn hình 1 lần duy nhất lúc mới vào menu
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE)); // Dọn buffer tránh Enter thừa từ fgets

    while (1) {
        resetCursor(); // Đưa con trỏ về (0,0) thay vì xóa toàn bộ màn hình
        if (printHeaderCallback != NULL) {
            printHeaderCallback();
        } else if (title != NULL && strlen(title) > 0) {
            printHeader(title);
            printf("\n");
        }

        for (int i = 0; i < numOptions; i++) {
            if (i == cursor) {
                // Determine color based on option text
                int color = 11; // Default cyan
                
                if (strstr(options[i], "Again") != NULL) {
                    color = 12; // Bright red for Again (flashcard)
                } else if (strstr(options[i], "Good") != NULL) {
                    color = 14; // Bright yellow for Good (flashcard)
                } else if (strstr(options[i], "Easy") != NULL) {
                    color = 10; // Bright green for Easy (flashcard)
                } else if (strstr(options[i], "Exit") != NULL || strstr(options[i], "Back") != NULL) {
                    color = 12; // Bright red (check Back first to override Flashcard)
                } else if (strstr(options[i], "Dictionary") != NULL) {
                    color = 9;  // Bright blue
                } else if (strstr(options[i], "Flashcard") != NULL) {
                    color = 10; // Bright green
                } else if (strstr(options[i], "Game") != NULL) {
                    color = 14; // Bright yellow
                } else if (strstr(options[i], "Stats") != NULL) {
                    color = 11; // Bright cyan
                } else if (strstr(options[i], "Login") != NULL || strstr(options[i], "Register") != NULL) {
                    color = 13; // Bright magenta
                } else if (strstr(options[i], "Standard") != NULL) {
                    color = 11; // Bright cyan for Standard (missing letter)
                } else if (strstr(options[i], "Challenge") != NULL) {
                    color = 14; // Bright yellow for Challenge (missing letter)
                } else if (strstr(options[i], "Expert") != NULL) {
                    color = 12; // Bright red for Expert (missing letter)
                } else if (strstr(options[i], "Play") != NULL || strstr(options[i], "English") != NULL || 
                          strstr(options[i], "Vietnamese") != NULL) {
                    color = 14; // Bright yellow for game options
                }
                
                setColor(color);
                printf(" %s ", ARROW);
                setColor(240); // White background, Black text
                printf(" %s \n", options[i]);
                setColor(7); // Reset
            } else {
                // Non-selected items also have subtle colors
                int color = 7; // Default white
                
                if (strstr(options[i], "Again") != NULL) {
                    color = 4;  // Dark red for Again (flashcard)
                } else if (strstr(options[i], "Good") != NULL) {
                    color = 6;  // Dark yellow for Good (flashcard)
                } else if (strstr(options[i], "Easy") != NULL) {
                    color = 2;  // Dark green for Easy (flashcard)
                } else if (strstr(options[i], "Exit") != NULL || strstr(options[i], "Back") != NULL) {
                    color = 4;  // Dark red (check Back first to override Flashcard)
                } else if (strstr(options[i], "Dictionary") != NULL) {
                    color = 1;  // Dark blue
                } else if (strstr(options[i], "Flashcard") != NULL) {
                    color = 2;  // Dark green
                } else if (strstr(options[i], "Game") != NULL) {
                    color = 6;  // Dark yellow
                } else if (strstr(options[i], "Stats") != NULL) {
                    color = 3;  // Dark cyan
                } else if (strstr(options[i], "Login") != NULL || strstr(options[i], "Register") != NULL) {
                    color = 5;  // Dark magenta
                } else if (strstr(options[i], "Standard") != NULL) {
                    color = 3;  // Dark cyan for Standard (missing letter)
                } else if (strstr(options[i], "Challenge") != NULL) {
                    color = 6;  // Dark yellow for Challenge (missing letter)
                } else if (strstr(options[i], "Expert") != NULL) {
                    color = 4;  // Dark red for Expert (missing letter)
                } else if (strstr(options[i], "Play") != NULL || strstr(options[i], "English") != NULL || 
                          strstr(options[i], "Vietnamese") != NULL) {
                    color = 6;  // Dark yellow for game options
                }
                
                setColor(color);
                printf("   ");
                printf(" %s \n", options[i]);
                setColor(7);
            }
        }

        key = _getch();
        if (key == 224) { // Arrow key prefix
            key = _getch();
            if (key == 72) { // Up arrow
                cursor--;
                if (cursor < 0) cursor = numOptions - 1;
            } else if (key == 80) { // Down arrow
                cursor++;
                if (cursor >= numOptions) cursor = 0;
            }
        } else if (key == 13) { // Enter key
            return cursor;
        }
    }
}
