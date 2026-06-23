#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "../include/game.h"
#include "../include/dictionary.h"
#include "../include/utils.h"

// cài đặt độ khó
// Easy   (1 từ mất): words >= 2 letters  -> at least 1 visible
// Medium (2 từ mất): words >= 4 letters  -> at least 2 visible
// Hard   (3 từ mất): words >= 6 letters  -> at least 3 visible
static int getMinLen(int difficulty) {
    if (difficulty == 2) return 4;
    if (difficulty == 3) return 6;
    return 2;
}
// chọn độ khó, lấy ngẫu nhiên trong từ điển theo độ dài tương ứng, ẩn chữ bằng dấu _
void playMissingLetterGame(HashTable *ht){

    // Đếm số từ có sẵn cho mỗi độ khó
    int cnt2 = 0, cnt4 = 0, cnt6 = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        Word *tmp = ht->buckets[i];
        while (tmp != NULL) {
            int len = (int)strlen(tmp->word);
            if (len >= 2) cnt2++;
            if (len >= 4) cnt4++;
            if (len >= 6) cnt6++;
            tmp = tmp->next;
        }
    }

    int difficulty = 1;

    // Thiết kế nhãn menu độ khó kèm số lượng từ.
    char opt1[60], opt2[60], opt3[60];
    snprintf(opt1, sizeof(opt1), "🎯 Standard  - 1 letter hidden (+20 EXP)  [%d words]", cnt2);
    snprintf(opt2, sizeof(opt2), "⚡ Challenge - 2 letters hidden (+30 EXP)  [%d words]", cnt4);
    snprintf(opt3, sizeof(opt3), "💎 Expert    - 3 letters hidden (+40 EXP)  [%d words]", cnt6);
    char *diffOptions[] = { opt1, opt2, opt3, "Back" };

    int diffChoice = selectMenu("🔤 MISSING LETTER - SELECT DIFFICULTY", diffOptions, 4, NULL);
    if (diffChoice == 3) return; // Back
    difficulty = diffChoice + 1; // 1, 2, or 3

    int minLen   = getMinLen(difficulty);
    int numHidden = difficulty; // 1, 2, or 3

    const char *diffLabel = difficulty == 1 ? "STANDARD" : difficulty == 2 ? "CHALLENGE" : "EXPERT";

    dailyMission.gamesPlayed++;

    int sessionCorrect = 0, sessionTotal = 0;
    int choice = 1;

    while (choice != 2) {
        clearScreen();

        // Chọn một từ có độ dài phù hợp.
        Word* word = getAdaptiveWordMinLen(ht, minLen);
        if (word == NULL) {
            setColor(12);
            printf("Not enough words with length >= %d for %s mode.\n", minLen, diffLabel);
            printf("Please add more words (>= %d letters) or choose an easier difficulty.\n", minLen);
            setColor(7);
            pauseScreen();
            return;
        }

        int wordLen     = (int)strlen(word->word);
        int actualHidden = (numHidden <= wordLen) ? numHidden : wordLen;

        // chuỗi ẩn kí tự
        char hidden[50];
        strncpy(hidden, word->word, sizeof(hidden) - 1);
        hidden[sizeof(hidden) - 1] = '\0';

        int hiddenIndices[3] = {-1, -1, -1};
        for (int h = 0; h < actualHidden; h++) {
            int idx, tries = 0, dup;
            do {
                idx = rand() % wordLen;
                dup = 0;
                for (int k = 0; k < h; k++) {
                    if (hiddenIndices[k] == idx) { dup = 1; break; }
                }
                tries++;
            } while (dup && tries < 100);
            hiddenIndices[h] = idx;
            hidden[idx] = '_';
        }

        sessionTotal++;

        setColor(11);
        printf("╔══════════════════════════════════════════════════╗\n");
        setColor(14);
        printf("║ 🔤 MISSING LETTER | %s | Score: %d/%d ║\n", diffLabel, sessionCorrect, sessionTotal - 1);
        setColor(11);
        printf("╚══════════════════════════════════════════════════╝\n");
        setColor(7);
        printf("Type   : [%s]\n", word->type);
        printf("Word   : ");
        setColor(14);
        printf("%s", hidden);
        setColor(7);
        printf("  (%d letters total, %d hidden)\n\n", wordLen, actualHidden);

        printf("Enter the full word OR just the %d missing letter(s) ['H' = hint]: ", actualHidden);
        char answer[50];
        scanf("%49s", answer);
        getchar();

        
        _strlwr(answer);

        // Hint option
        if (strcmp(answer, "h") == 0) {
            setColor(14);
            printf("\n[HINT] Meaning: %s\n\n", word->meaning);
            setColor(7);
            printf("Enter the full word OR just the %d missing letter(s): ", actualHidden);
            scanf("%49s", answer);
            getchar();
            _strlwr(answer);
        }


        char formedWord[50];
        if ((int)strlen(answer) == wordLen) {
            strncpy(formedWord, answer, sizeof(formedWord) - 1);
            formedWord[sizeof(formedWord) - 1] = '\0';
        } else {
            strncpy(formedWord, hidden, sizeof(formedWord) - 1);
            formedWord[sizeof(formedWord) - 1] = '\0';
            int ansIdx = 0, ansLen = (int)strlen(answer);
            for (int i = 0; i < wordLen && ansIdx < ansLen; i++) {
                if (formedWord[i] == '_') formedWord[i] = answer[ansIdx++];
            }
        }

        
        int isValidPattern = 1;
        for (int i = 0; i < wordLen; i++) {
            if (hidden[i] != '_' && formedWord[i] != hidden[i]) {
                isValidPattern = 0;
                break;
            }
        }

        
        Word* foundWord = NULL;
        if (isValidPattern) {
            foundWord = searchWord(ht, formedWord);
        }

        if (foundWord != NULL) {
            sessionCorrect++;
            setColor(10);
            printf("\nCorrect! The word is: %s\n", foundWord->word);
            setColor(7);
            // Show all meanings split by ';'
            printf("Meaning(s):\n");
            char copy[500];
            strncpy(copy, foundWord->meaning, sizeof(copy) - 1);
            copy[sizeof(copy) - 1] = '\0';
            char *tok = strtok(copy, ";");
            int n = 1;
            while (tok) { printf("  %d. %s\n", n++, tok); tok = strtok(NULL, ";"); }
            printf("Pronunciation: %s\n", foundWord->pronunciation);

            foundWord->learned = 1;
            int expGain = 20 + (difficulty - 1) * 10; // 20 / 30 / 40
            playStats.exp += expGain;
            updateLevel();

            setColor(14);
            printf("EXP +%d | Total: %d | Level: %d\n", expGain, playStats.exp, playStats.level);
            setColor(7);
        } else {
            setColor(12);
            printf("\nKhông có từ này trong tiếng Anh.\n");
            setColor(7);

            
            char original[50];
            strncpy(original, hidden, sizeof(original) - 1);
            original[sizeof(original) - 1] = '\0';
            for (int h = 0; h < actualHidden; h++) {
                if (hiddenIndices[h] >= 0)
                    original[hiddenIndices[h]] = word->word[hiddenIndices[h]];
            }
            printf("The word we had in mind: ");
            setColor(14);
            printf("%s\n", original);
            setColor(7);

            word->wrongCount++;
            updateLevel();
        }

        // Điểm số trực tiếp + menu chơi lại
        printf("\nSession: %d/%d correct", sessionCorrect, sessionTotal);
        if (sessionTotal > 0)
            printf(" (%.0f%%)", (float)sessionCorrect / sessionTotal * 100.0f);
        printf("\n");
        pauseScreen();

        char *nextOpts[] = { "🔄 Play Again", "🚪 Exit Game" };
        int nextChoice = selectMenu("🔤 MISSING LETTER", nextOpts, 2, NULL);
        if (nextChoice == 1) choice = 2; // Exit
    }

    // Final score screen
    clearScreen();
    setColor(11);
    printf("╔════════════════════════════════════════╗\n");
    setColor(14);
    printf("║          🔤 GAME OVER 🔤              ║\n");
    setColor(11);
    printf("╚════════════════════════════════════════╝\n");
    setColor(7);
    printf("Difficulty  : %s\n", diffLabel);
    printf("Final Score : %d / %d\n", sessionCorrect, sessionTotal);
    if (sessionTotal > 0) {
        float acc = (float)sessionCorrect / sessionTotal * 100.0f;
        printf("Accuracy    : %.0f%%\n\n", acc);
        if (acc >= 80)      { setColor(10); printf("Rating: ⭐ Excellent! Well done!\n"); }
        else if (acc >= 50) { setColor(14); printf("Rating: 👍 Good job! Keep it up!\n"); }
        else                { setColor(12); printf("Rating: 💪 Keep practicing!\n"); }
        setColor(7);
    }
    pauseScreen();
}
// chọn ngẫu nhiên 1 từ đã học, hiện tanh gõ tviet
//dịch thuật 
void englishToVietnameseGame(HashTable *ht){
    clearScreen();
    if (dailyMission.flashcardsReviewed == 0) {
        printf("\n  Ban chua hoc Flashcard hom nay!\n");
        printf("  Hay vao Flashcard on tap tu vung truoc nhe.\n");
        pauseScreen();
        return;
    }
    dailyMission.gamesPlayed++;
    Word *word = getStudiedWord(ht);
    if(word == NULL){
        printf("No words available.\n");
        pauseScreen();
        return;
    }
    char answer[100];
    setColor(11);
    printf("╔════════════════════════════════════════════╗\n");
    setColor(14);
    printf("║    🇬🇧 ENGLISH -> VIETNAMESE 🇻🇳        ║\n");
    setColor(11);
    printf("╚════════════════════════════════════════════╝\n");
    setColor(7);
    printf("\nTranslate this word:\n");
    setColor(14);
    printf("  %s\n\n", word->word);
    setColor(7);
    printf("Your answer: ");
    fgets(answer, sizeof(answer), stdin);
    answer[strcspn(answer, "\n")] = '\0';
    
    int isCorrect = 0;
    if (strlen(answer) > 0) {
        char copy[500];
        strncpy(copy, word->meaning, sizeof(copy) - 1);
        copy[sizeof(copy) - 1] = '\0';
        char *token = strtok(copy, ";");
        while (token != NULL) {
            // Remove leading/trailing spaces if any
            while(*token == ' ') token++;
            int len = strlen(token);
            while(len > 0 && token[len-1] == ' ') { token[len-1] = '\0'; len--; }
            
            if (_stricmp(token, answer) == 0) {
                isCorrect = 1;
                break;
            }
            token = strtok(NULL, ";");
        }
    }

    if (isCorrect) {
        setColor(10);
        printf("\nCorrect!\n");
        setColor(7);
        playStats.exp += 20;
        updateLevel();
    } else {
        setColor(12);
        printf("\nWrong answer.\n");
        setColor(7);
        printf("Correct meaning: %s\n", word->meaning);
    }
    pauseScreen();
}
// chọn ngẫu nhiên 1 từ đã học, hiện tviet gõ tanh
//dịch thuật 
void vietnameseToEnglishGame(HashTable *ht){
    clearScreen();
    if (dailyMission.flashcardsReviewed == 0) {
        printf("\n  Ban chua hoc Flashcard hom nay!\n");
        printf("  Hay vao Flashcard on tap tu vung truoc nhe.\n");
        pauseScreen();
        return;
    }
    dailyMission.gamesPlayed++;
    Word *word = getStudiedWord(ht);
    if(word == NULL){
        printf("No words available.\n");
        pauseScreen();
        return;
    }
    char answer[100];
    setColor(11);
    printf("╔════════════════════════════════════════════╗\n");
    setColor(14);
    printf("║    🇻🇳 VIETNAMESE -> ENGLISH 🇬🇧        ║\n");
    setColor(11);
    printf("╚════════════════════════════════════════════╝\n");
    setColor(7);
    printf("\nTranslate this meaning:\n");
    setColor(14);
    printf("  %s\n\n", word->meaning);
    setColor(7);
    printf("Your answer: ");
    fgets(answer, sizeof(answer), stdin);
    answer[strcspn(answer, "\n")] = '\0';
    if(_stricmp(answer, word->word) == 0){
        setColor(10);
        printf("\nCorrect!\n");
        setColor(7);
        playStats.exp += 20;
        updateLevel();
    } else {
        setColor(12);
        printf("\nWrong answer.\n");
        setColor(7);
        printf("Correct word: %s\n", word->word);
    }
    pauseScreen();
}
