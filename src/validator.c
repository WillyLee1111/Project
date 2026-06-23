#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/validator.h"

//Duyệt danh sách liên kết từ đầu bucket để so sánh chuỗi bằng strcmp
//ngăn vc thêm từ đã có
int isDuplicateWord(Word *head, char *word) {
    Word *temp = head;
    while (temp != NULL) {
        if (strcmp(temp->word, word) == 0) {
            return 1; // Duplicate found
        }
        temp = temp->next;
    }
    return 0; // No duplicate
}
// kiểm tra độ dà chuỗi tên đăng nhập
//xác thực độ dài
int isValidUsername(char *username) {
    if (strlen(username) < 3 || strlen(username) > 50) {
        return 0; //yêu cầu đủ độ dài
    }
    // Chỉ kiểm tra độ dài, cho phép ký tự Unicode (tiếng Việt)
    return 1; // Valid username
}
//kiểm tra mkhau
//bắt ng dùng đki mật khẩu đủ mạnh để đảm bảo an toàn
int isStrongPassword(char *password) {
    if (strlen(password) < 6) {
        return 0; //tối thiếu 6
    }
    int hasUpper = 0, hasLower = 0, hasDigit = 0;
    for (int i = 0; password[i] != '\0'; i++) {
        if (isupper(password[i])) hasUpper = 1;
        else if (islower(password[i])) hasLower = 1;
        else if (isdigit(password[i])) hasDigit = 1;
    }
    return hasUpper && hasLower && hasDigit; 
}