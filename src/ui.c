#include <stdio.h>
#include <windows.h>

#include "../include/utils.h"

void showIntroScreen(){
    clearScreen();
    setColor(11);
    printf("===========================================================\n");
    setColor(14);
    printf("      Truong: Dai Hoc Bach Khoa - Dai hoc Da Nang          \n");
    printf("                 Khoa: Cong nghe thong tin                 \n");
    setColor(11);
    printf("-----------------------------------------------------------\n");
    setColor(10);
    printf("             PBL1: DO AN LAP TRINH TINH TOAN               \n");
    setColor(7);
    printf("   De tai: Hoc tu vung tieng Anh thong qua tro choi        \n");
    printf("   Giao vien huong dan: Truong Ngoc Chau                   \n");
    setColor(11);
    printf("   Sinh vien: Nguyen Hung Thinh - 25T_Nhat1               \n");
    printf("              Huynh Thi Anh Ngoc - 25T_Nhat1              \n");
    setColor(7);
    printf("===========================================================\n");
    printf("\n");
    setColor(14);
    printf("               Press ENTER to continue...");
    setColor(7);
    getchar();
}
