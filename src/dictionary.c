#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../include/dictionary.h"
#include "../include/validator.h"
#include "../include/utils.h"

#define WIDTH 50

//cấp bộ nhớ cho struct HashTable, khởi tạo các bucket về NULL và BST root về NULL
// khởi tạo để sẵn sàng lưu trữ
HashTable* createHashTable() {
    HashTable *ht = (HashTable*)malloc(sizeof(HashTable));
    for (int i = 0; i < HASH_SIZE; i++) {
        ht->buckets[i] = NULL;
    }
    ht->bstRoot = NULL;  // BST starts empty
    return ht;
}
// thực hiện thuật toán djb2( duyệt chữ cái, dịch bít nhân 33 + max ASCII sau đó chia 10007)
// để lưu từ vựng thành 1 index thành sô nguyên để lưu trữ, truy cập nhanh trong HST
unsigned int hashFunction(char* word) {
    unsigned int hash = 5381; 
    int c;
    while ((c = *word++)) {
        hash = ((hash << 5) + hash) + tolower(c);
    }
    return hash % HASH_SIZE;
}
//Cấp ram cho struct Word, dùng strncpy để copy dữ liệu vào struct, đảm bảo không tràn bộ nhớ, khởi tạo các trường khác về mặc định, gán stat ban đầu là 0
//khởi tạo từ với thông số mặc định
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

//Ss từ cần chèn với node hiện tại, nếu nhỏ hơn thì đi trái, lớn hơn thì đi phải, nếu bằng thì bỏ qua (không chèn trùng)
//thêm từ vào BST để hỗ trợ gợi ý từ theo prefix
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
//chạy vòng lặp for theo nhánh trái, cho đến khi kh rẽ trái được nữa.
// tìm nút nhỏ nhất trong 1 nhánh cây của BST, sp BST delete trong trường hợp 2 con
static BSTNode* bstMinNode(BSTNode *node) {
    while (node->left != NULL) node = node->left;
    return node;
}
//Đệ quy tìm kiếm từ cần xóa, 1 con or kh con thì xóa, 2 con thì thế ở nhánh phải
//Xóa từ
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
//Đệ quy giải phóng nhánh trái, phải sau đó giải phóng chính nút đó bằng lệnh free
//thu hôi vùng bộ nhớ của BST khi xóa toàn bộ cây
void freeBST(BSTNode *root) {
    if (root == NULL) return;
    freeBST(root->left);
    freeBST(root->right);
    free(root);
}

//ss tiền tố prefix với từ ở nút htai nếu từ đứng trước/sau tiền tố thì chỉ duyệt tiếp nhánh phải/nhánh trái; nếu khớp tiền tố thì lưu từ đó vào mảng kết quả suggestions (tối đa 20 từ) rồi tiếp tục duyệt cả hai nhánh.
//duyệt cây BST để thu thập với tiền tố ng dùng nhập, sp cho thuật toán gợi ý
static void bstCollect(BSTNode *node, const char *prefix, int prefixLen,
                        Word **suggestions, int *count, HashTable *ht) {
    if (node == NULL || *count >= 20) return;
    int cmp = _strnicmp(node->word, prefix, prefixLen);
    if (cmp < 0) {
        // Từ này đứng trước tiền tố theo thứ tự bảng chữ cái -> chỉ nhánh cây bên phải mới khớp
        bstCollect(node->right, prefix, prefixLen, suggestions, count, ht);
    } else if (cmp > 0) {
        // Từ này đứng sau tiền tố theo thứ tự bảng chữ cái -> chỉ nhánh cây bên trái mới có thể khớp
        bstCollect(node->left, prefix, prefixLen, suggestions, count, ht);
    } else {
        // Từ này bắt đầu bằng tiền tố -> thu thập nó và kiểm tra cả hai nhánh
        bstCollect(node->left,  prefix, prefixLen, suggestions, count, ht);
        Word *found = searchWord(ht, node->word);
        if (found != NULL && *count < 20) suggestions[(*count)++] = found;
        bstCollect(node->right, prefix, prefixLen, suggestions, count, ht);
    }
}

//tính index và chèn từ mới vào ds lk tại bucket, gọi bstInsert để cập nhật BST cho gợi ý prefix
// đưa từ mới vào đồng bộ ở cả bảng băm và cây BST để đảm bảo truy cập nhanh và gợi ý hiệu quả
void insertWord(HashTable* ht, Word *newWord) {
    unsigned int index = hashFunction(newWord->word);
    newWord->next = ht->buckets[index];
    ht->buckets[index] = newWord;
    // Keep BST in sync for O(log n) prefix suggestions
    ht->bstRoot = bstInsert(ht->bstRoot, newWord->word);
}

// tạo chuỗi meaning có thể chứa nhiều nghĩa, mỗi nghĩa cách nhau bằng dấu chấm phẩy ';'
// hiển thị chi tiết 1 từ, tách nghĩa thành từng dòng nếu có nhiều nghĩa
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

// duyệt BST theo thứ tự inorder ( left -> node -> right) với mỗi nút, gọi searchWord để lấy thông tin và in ra
// Duyệt và in thông tin các từ trong từ điển theo A-> Z
static void bstPrintInOrder(BSTNode *node, HashTable *ht, int *count) {
    if (node == NULL) return;
    bstPrintInOrder(node->left, ht, count);
    Word *w = searchWord(ht, node->word);
    if (w != NULL) {
    (*count)++;

    setColor(11);

    char line1[100], line2[100], line3[100], line4[100], line5[100];

    sprintf(line1, "Word: %s", w->word);
    sprintf(line2, "Type: %s", w->type);
    sprintf(line3, "Pronunciation: [%s]", w->pronunciation);
    sprintf(line4, "Meaning: %s", w->meaning);
    sprintf(line5, "Status: %s | Wrong: %d",
            w->learned ? "Learned" : "Not learned",
            w->wrongCount);

    printf(" -------------------------------------------------------------\n");
    printf("| %-59s |\n", line1);
    printf("| %-59s |\n", line2);
    printf("| %-59s |\n", line3);
    printf("| %-59s |\n", line4);
    printf("| %-59s |\n", line5);
    printf(" -------------------------------------------------------------\n");

    setColor(7);
}
    bstPrintInOrder(node->right, ht, count);
}
//đếm tổng số từ trog bảng băm, nếu có từ gọi đệ quy bstPrintInOrder từ gốc để in
//Hiện ds từ điển đã đc sắp xếp bảng chữ cái
void displayDictionary(HashTable* ht) {
    // đếm tổng từ
    int total = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *tmp = ht->buckets[i];
        while (tmp) { total++; tmp = tmp->next; }
    }
    if (total == 0) { printf("Dictionary is empty.\n"); return; }

    // in ra BST theo bảng chữ cái
    int count = 0;
    bstPrintInOrder(ht->bstRoot, ht, &count);
    printf("\nTotal: %d word(s) [sorted A-Z via BST in-order]\n", count);
}
// tính index, nhảy tới bucket, duyệt ds lk để tìm từ khớp, trả về con trỏ đến struct Word nếu tìm thấy, ngược lại trả về NULL
// Tra cứu từ vựng bằng cách tính hash và duyệt bucket tương ứng
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

// chuyển tiền tố, tạo biến, gọi đệ quy bstCollect để thu thập từ gợi ý, trả về số lượng gợi ý tìm được
// trả về danh sách gợi ý tối đa 20 từ khớp với tiền tố đã nhập
int suggestWords(HashTable* ht, char *prefix, Word *suggestions[20]) {
    _strlwr(prefix);
    int count = 0;
    int prefixLen = (int)strlen(prefix);
    bstCollect(ht->bstRoot, prefix, prefixLen, suggestions, &count, ht);
    return count;
}
//duyệt toàn bộ mảng để giải phóng từ Word, trong dslk, gọi freeBST để giải phóng BST, xong gphong struct
// Giải phóng toàn bộ memory của từ điển khi thoát chương trình
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
//đếm tổng số từ đang có, sinh 1 số ngẫu nhiên, duyệt lại để lấy số đó
// bốc ngẫu nhiên 1 từ
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
//sinh ra số từ 0 tới 99, nếu <70 thì duyệt lại lấy ngẫu nhiên từ yếu, còn k thì ở 30 gọi getRandom
//Thuật toán học tập thích ứng 
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

// Tương tự như getAdaptiveWord nhưng chỉ chọn những từ có độ dài >= minLen.
// Được trò chơi Missing Letter sử dụng để đảm bảo đủ số lượng chữ cái hiển thị.
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
    if (count == 0) return getRandomWord(ht); 
    return candidates[rand() % count];
}
//tính tỉ lệ phần trăm, in kí tự ... để đại diện tiến độ
//vẽ biểu đồ thanh phần trăm 
static void drawAsciiBar(int learned, int total) {
    int width = 20;
    int filled = (total > 0) ? (learned * width / total) : 0;
    printf("[");
    for (int i = 0; i < width; i++) {
        if (i < filled) {
            setColor(10); // Light Green
            printf("\xE2\x96\xA0"); // ■
        } else {
            setColor(8); // Dark Gray
            printf("-");
        }
    }
    setColor(7); // Reset
    printf("]");
    int percent = (total > 0) ? (learned * 100 / total) : 0;
    printf(" %3d%%", percent);
}
//quét bản, thu thập số liệu. in ra báo cáo thống kê
//tổng hợp và hiển thị
void showStats(HashTable* ht) {
    int totalWords = 0, learnedWords = 0, weakWords = 0;
    // Bộ đếm loại từ
    int totalNoun = 0, learnedNoun = 0;
    int totalVerb = 0, learnedVerb = 0;
    int totalAdj = 0, learnedAdj = 0;
    int totalAdv = 0, learnedAdv = 0;
    int totalOther = 0, learnedOther = 0;

    // thu thập 5 từ khó
    Word *top5[5] = {NULL, NULL, NULL, NULL, NULL};

    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            totalWords++;
            if (temp->learned == 1) learnedWords++;
            if (temp->wrongCount >= 3 && temp->learned == 0) weakWords++;

            // đếm loại từ
            if (_stricmp(temp->type, "noun") == 0) {
                totalNoun++;
                if (temp->learned == 1) learnedNoun++;
            } else if (_stricmp(temp->type, "verb") == 0) {
                totalVerb++;
                if (temp->learned == 1) learnedVerb++;
            } else if (_stricmp(temp->type, "adjective") == 0) {
                totalAdj++;
                if (temp->learned == 1) learnedAdj++;
            } else if (_stricmp(temp->type, "adverb") == 0) {
                totalAdv++;
                if (temp->learned == 1) learnedAdv++;
            } else {
                totalOther++;
                if (temp->learned == 1) learnedOther++;
            }

            // Chèn vào top 5 nếu wrongCount đủ lớn (sắp xếp chèn)
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
    setColor(11);
    printf("╔════════════════════════════════════════════╗\n");
    setColor(14);
    printf("║         📊 PLAYER STATISTICS 📊           ║\n");
    setColor(11);
    printf("╚════════════════════════════════════════════╝\n");
    setColor(7);
    printf("\n");
    setColor(11); printf("🎖️  LEVEL      "); setColor(7); printf(": %d\n", playStats.level);
    setColor(11); printf("⭐ EXP        "); setColor(7); printf(": %d / %d (next level)\n",
        playStats.exp, (playStats.level) * 100);
    setColor(11); printf("🔥 STREAK     "); setColor(7); printf(": %d days\n", studyStreak.streakDays);
    printf("\n");

    // Dictionary progress
    setColor(14); printf("\n📚 DICTIONARY PROGRESS\n"); setColor(7);
    printf("─────────────────────────────────────────────\n");
    printf("Total Words   : %d\n", totalWords);
    printf("Learned       : %d / %d\n", learnedWords, totalWords);
    printf("\n");

    // By word type breakdown
    setColor(14); printf("\n📖 WORDS BY TYPE (Learned / Total)\n"); setColor(7);
    printf("─────────────────────────────────────────────\n");
    
    printf("  %-10s: %3d / %-3d ", "Noun", learnedNoun, totalNoun);
    drawAsciiBar(learnedNoun, totalNoun);
    printf("\n");
    
    printf("  %-10s: %3d / %-3d ", "Verb", learnedVerb, totalVerb);
    drawAsciiBar(learnedVerb, totalVerb);
    printf("\n");
    
    printf("  %-10s: %3d / %-3d ", "Adjective", learnedAdj, totalAdj);
    drawAsciiBar(learnedAdj, totalAdj);
    printf("\n");
    
    printf("  %-10s: %3d / %-3d ", "Adverb", learnedAdv, totalAdv);
    drawAsciiBar(learnedAdv, totalAdv);
    printf("\n");
    
    if (totalOther > 0) {
        printf("  %-10s: %3d / %-3d ", "Other", learnedOther, totalOther);
        drawAsciiBar(learnedOther, totalOther);
        printf("\n");
    }
    printf("\n");

    // Daily mission
    setColor(13); printf("\n📋 DAILY MISSION\n"); setColor(7);
    printf("─────────────────────────────────────────────\n");
    printf("Words Learned : %d / %d\n", dailyMission.wordsLearnedToday,
           dailyMission.targetWords > 0 ? dailyMission.targetWords : 10);
    printf("Flashcards    : %d / %d\n", dailyMission.flashcardsReviewed,
           dailyMission.targetFlashcards > 0 ? dailyMission.targetFlashcards : 5);
    printf("Games Played  : %d / %d\n", dailyMission.gamesPlayed,
           dailyMission.targetGames > 0 ? dailyMission.targetGames : 1);
    printf("\n");

    // Top-5 hardest words sorted by wrongCount
    setColor(12); printf("\n⚠️  TOP 5 HARDEST WORDS (sorted by wrong count)\n"); setColor(7);
    printf("─────────────────────────────────────────────\n");
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
    printf("─────────────────────────────────────────────\n");
    pauseScreen();
}
//mở file progress.txt ghi lại thuộc tính learned và wrongCount
//lưu progress
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
//mở file progess.txt của riêng user, đọc dulieu và dùng searchWord để cập nhật trạng thái học tập
//tải lên tiến độ học tập
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
//quét xem từ đó trùng với type nào
//lọc và lấy ngẫu nhiên theo loại
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
//lọc wrongCount >=3 đưa và learned ==0 đưa vào mảng, lấy ngẫu nhiên trả về
//chọn ngẫu nhiên 1 từ trong danh sách các từ ng dùng hay sai
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
//thu thập từ trạng thái learned ==1 hay wrongCount >0 đưa vào mảng và lấy ngẫu nhiên
//chọn 1 từ ngẫu nhiên ng dùng đã từng ttac để ôn
Word* getStudiedWord(HashTable* ht) {
    Word* studiedWords[1000];
    int count = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *temp = ht->buckets[i];
        while (temp != NULL) {
            
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
//ng dùng nhập từ mới, nhập nhiều nghĩa thông qua ;, rồi đưa vào BST
//cung cấp giao diện ng dùng tự thêm từ
void addWord(HashTable* ht) {
    char word[50], meaning[500], pronunciation[50], type[20];
    char confirm = 'n';

   
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
//tìm từ, hiện thông số, ng dùng sửa và ghi đè Ram
//sửa thông tin
void editWord(HashTable* ht) {
    char target[50];
    printf("Enter the word to edit: ");
    scanf("%49s", target);
    Word* word = searchWord(ht, target);
    if (word == NULL) {
        printf("Word not found.\n");
        return;
    }
    // cho thấy thông tin hiện tại
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
//tìm từ từ dslk, ngắt liên kết, xóa nút đó trên cây BST + gphong bộ nhớ
//xóa hoàn toàn khỏi từ điển
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
//mở file dictionary.txt duyệt toàn bộ từ điển theo struct phân tách |
//lưu vv các thay đổi từ điển
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
//tính toán cấp độ dựa trên exp
//cập nhật cấp bậc của tài khoản
void updateLevel (){
    playStats.level = playStats.exp / 100 + 1;
}
