# English Dictionary App

## Tổng quan
Đây là đồ án quản lý từ điển tiếng Anh, hỗ trợ:
- Quản lý từ vựng (thêm, sửa, xóa, tìm kiếm, hiển thị)
- Lưu và tải dữ liệu người dùng, tiến độ học tập
- Flashcard với chế độ hiển thị thuật toán chọn từ thông minh
- Trò chơi học từ (Missing Letter, English->Vietnamese, Vietnamese->English)
- Thống kê tiến độ học, streak, điểm kinh nghiệm và bảng cấp độ

## Cấu trúc dự án
```
include/
  dictionary.h
  file.h
  flashcard.h
  game.h
  ui.h
  utils.h
  validator.h
src/
  main.c
  dictionary.c
  file.c
  flashcard.c
  game.c
  ui.c
  utils.c
  validator.c
build/
  app.exe
data/
  dictionary.txt
  users.txt
  userdata.txt
  users/<username>/progress.txt
```

## Hướng dẫn build và chạy
- Dùng Windows và GCC.
- Chạy `run.bat` để biên dịch tất cả file trong `src/` và tạo `build/app.exe`.
- Hoặc:
  ```
  gcc -Wall -Wextra src/*.c -o build/app.exe
  build\app.exe
  ```

## Dữ liệu và định dạng file
- `data/dictionary.txt`: mỗi dòng chứa từ vựng theo định dạng:
  `word|meaning|pronunciation|type`
- `data/users.txt`: mỗi dòng chứa đăng ký người dùng:
  `username|password`
- `data/users/<username>/userdata.txt`: lưu trạng thái người dùng:
  - `MISSION wordsLearnedToday flashcardsReviewed gamesPlayed`
  - `STREAK lastStudyDate streakDays`
  - `STATS level exp`
- `data/users/<username>/progress.txt`: lưu tiến độ từ vựng:
  `word learned wrongCount`

## Thuật toán và chức năng chính

### Tên thuật toán chính
- Thuật toán kiểm tra đăng nhập/đăng ký: `login()` và `registerUser()` trong `src/file.c`
- Thuật toán quản lý danh sách liên kết từ điển: `createWord()` / `insertWord()` / `deleteWord()` trong `src/dictionary.c`
- Thuật toán gợi ý từ theo tiền tố: `suggestWords()`
- Thuật toán chọn từ thích nghi (adaptive selection): `getAdaptiveWord()`
- Thuật toán chọn từ yếu (weak word selection): `getWeakWord()`
- Thuật toán chọn từ theo loại: `getWordByType()`
- Thuật toán trò chơi Missing Letter: `playMissingLetterGame()`
- Thuật toán kiểm tra dịch Anh-Việt / Việt-Anh: `englishToVietnameseGame()` và `vietnameseToEnglishGame()`
- Thuật toán cập nhật streak và EXP: `updateStudyStreak()` và `updateLevel()`

### 1. Luồng chương trình chính
`src/main.c`
- Hiển thị màn hình chào mừng và yêu cầu người dùng chọn đăng nhập hoặc đăng ký.
- Nếu chọn đăng nhập: `login()` kiểm tra tài khoản `admin/admin` hoặc tìm trong `data/users.txt`.
- Nếu chọn đăng ký: `registerUser()` thêm tài khoản mới và tạo thư mục người dùng.
- Sau đăng nhập, tải từ điển từ `data/dictionary.txt` và dữ liệu người dùng.
- Hiển thị menu chính với các lựa chọn:
  1. Dictionary Menu
  2. Flashcard Mode
  3. Game Center
  4. Show Stats
  5. Exit
- Khi thoát, lưu progress, user data và dictionary.

### 2. Quản lý từ điển
`src/dictionary.c`
- `createWord(...)`: tạo node `Word` trong danh sách liên kết đơn và sao chép an toàn chuỗi với `strncpy`.
- `insertWord(...)`: chèn từ mới vào cuối danh sách.
- `displayDictionary(...)`: duyệt danh sách và in toàn bộ từ.
- `searchWord(...)`: tìm từ chính xác theo tên.
- `suggestWords(...)`: gợi ý tối đa 20 từ bắt đầu bằng tiền tố được nhập.
- `getRandomWord(...)`: chọn ngẫu nhiên một từ từ danh sách bằng cách đếm số node và chọn chỉ số ngẫu nhiên.
- `getAdaptiveWord(...)`: thuật toán chọn từ học:
  - 70% cơ hội chọn từ yếu (wrongCount >= 3 và learned == 0)
  - Nếu không có từ yếu hoặc không chọn, trả về từ ngẫu nhiên.
- `getWeakWord(...)`: tập hợp tất cả từ yếu (wrongCount >= 3 và learned == 0), sau đó chọn ngẫu nhiên.
- `getWordByType(...)`: lọc từ theo `type` (noun/verb/adjective/adverb) và chọn ngẫu nhiên từ kết quả.
- `freeList(...)`: giải phóng toàn bộ danh sách liên kết.
- `saveDictionary(...)`: ghi lại `data/dictionary.txt` với tất cả từ hiện tại.
- `addWord(...)`: 
  - nhập từ bằng `scanf` và kiểm tra sao chép trùng lặp.
  - sử dụng `fgets()` để nhận nghĩa, phát âm, và loại từ.
  - thêm node mới vào danh sách.
- `editWord(...)`: tìm từ, nhập lại nghĩa, phát âm và loại từ, cập nhật node hiện tại.
- `deleteWord(...)`: tìm node theo từ, bỏ node khỏi danh sách liên kết và giải phóng bộ nhớ.
- `updateLevel()`: tính cấp độ dựa trên EXP: `level = exp / 100 + 1`.

### 3. Lưu và tải dữ liệu người dùng
`src/file.c`
- `loadDictionary(...)`: mở `data/dictionary.txt`, tách dòng theo `|`, tạo từ mới và chèn vào danh sách.
- `saveUserData()`:
  - ghi trạng thái mission, streak và stats vào `userdata.txt` của người dùng.
- `loadUserData()`:
  - đọc `userdata.txt` từng nhãn `MISSION`, `STREAK`, `STATS` và cập nhật biến toàn cục.
- `login()`:
  - nếu `admin/admin` thì cho phép login trực tiếp.
  - nếu không, đọc `data/users.txt`, tách `username|password`, so sánh và khởi tạo `currentUser`.
  - gọi `updateStudyStreak()` khi đăng nhập thành công.
- `registerUser()`:
  - kiểm tra username/password hợp lệ.
  - lưu vào `data/users.txt`.
  - tạo thư mục `data/users/<username>`.
- `updateStudyStreak()`:
  - lấy ngày hiện tại với `localtime()`.
  - định dạng `YYYY-MM-DD` bằng `strftime()`.
  - nếu khác ngày lưu trong `studyStreak.lastStudyDate`, tăng `streakDays` và cập nhật ngày mới.

### 4. Flashcard Mode
`src/flashcard.c`
- `flashcardMode(...)` hiển thị menu flashcard với 6 chế độ:
  1. All Word
  2. Noun Word
  3. Verb Word
  4. Adjective Word
  5. Adverb Word
  6. Weak Words
- Thuật toán chọn thẻ:
  - `mode 1`: dùng `getAdaptiveWord(head)` để chọn từ yếu ưu tiên 70%.
  - `mode 2-5`: dùng `getWordByType(head, <type>)` để chọn ngẫu nhiên theo loại.
  - `mode 6`: dùng `getWeakWord(head)` để chọn từ yếu.
- Hiển thị mặt trước thẻ (từ), đợi Enter, sau đó hiển thị nghĩa, loại và phát âm.
- Người dùng chọn review:
  - 1. Again -> tăng `wrongCount`
  - 2. Good -> chỉ cộng EXP
  - 3. Easy -> nếu chưa học, đánh dấu `learned = 1`, tăng `wordsLearnedToday`, cộng thêm EXP
- Cập nhật `dailyMission.flashcardsReviewed`, `playStats.exp`, và `updateLevel()`.
- Lặp lại hoặc thoát theo lựa chọn.

### 5. Game Center
`src/game.c`
- `playMissingLetterGame(...)`:
  - chọn từ từ `getAdaptiveWord(head)`.
  - ẩn một chữ cái ngẫu nhiên bằng dấu `_`.
  - người chơi nhập chữ cái thiếu.
  - nếu đúng: đánh dấu learned, cộng EXP.
  - nếu sai: tăng `wrongCount`, hiển thị đáp án.
- `englishToVietnameseGame(...)`:
  - chọn từ ngẫu nhiên.
  - yêu cầu người chơi dịch sang tiếng Việt.
  - nếu chuỗi trả lời chứa nghĩa đúng, coi là đúng.
  - cập nhật EXP.
- `vietnameseToEnglishGame(...)`:
  - chọn từ ngẫu nhiên.
  - yêu cầu dịch sang tiếng Anh.
  - so sánh không phân biệt hoa thường với `_stricmp`.
  - cập nhật EXP.

### 6. Giao diện và tiện ích
`src/utils.c`
- `clearScreen()`: gọi `system("cls")`.
- `pauseScreen()`: gọi `system("pause")`.
- `printHeader(...)`: in khung tiêu đề với ký tự Unicode.
- `setColor(...)`: thay đổi màu chữ console Windows.
- `showMiniPlayerCard()`:
  - tính phần trăm EXP so với cấp độ hiện tại.
  - vẽ thanh tiến trình bằng ký tự `■`.
- `showDictionaryMenu()` và `showGameMenu()` in menu đẹp.
- `printSuccess(...)` / `printError(...)` in thông báo màu.

### 7. Xác thực và kiểm tra dữ liệu
`src/validator.c`
- `isDuplicateWord(...)`: kiểm tra từ đã tồn tại trong danh sách.
- `isValidUsername(...)`: username phải dài 3-50 ký tự và chỉ chứa chữ/ số/ gạch dưới.
- `isStrongPassword(...)`: password phải >= 6 ký tự, có chữ hoa, chữ thường và chữ số.

## Các biến toàn cục chính
- `Mission dailyMission`: lưu số từ học trong ngày, flashcards đã xem, trò chơi đã chơi.
- `Streak studyStreak`: lưu ngày học gần nhất và số ngày liên tiếp.
- `PlayStats playStats`: lưu EXP và cấp độ.
- `User currentUser`: lưu username, password, id hiện tại.

## Đề xuất mở rộng
- Thêm xác thực khi `data/users.txt` hoặc thư mục người dùng không tồn tại.
- Thêm chức năng reset progress hoặc xoá tài khoản.
- Thêm lưu `last login` hoặc ghi thời gian học.
- Thêm tìm kiếm không phân biệt hoa thường.
- Thêm giao diện menu mềm mại hơn, nhập chuỗi đầy đủ cho từ, tránh dùng nhiều `scanf`.

---

## Ghi chú
README này đã bao gồm toàn bộ thuật toán và logic chính xuất hiện trong code hiện tại của dự án. Nếu muốn, bạn có thể mở rộng thêm phần mô tả chi tiết với mỗi hàm cụ thể. 