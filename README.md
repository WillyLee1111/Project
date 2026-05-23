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
- Cấu trúc Bảng băm (Hash Table) xử lý từ điển: `createHashTable()`, `hashFunction()`, `insertWord()`, `searchWord()` trong `src/dictionary.c`
- Thuật toán gợi ý từ (Prefix Matching trên Bảng băm): `suggestWords()`
- Thuật toán chọn từ thích nghi (adaptive selection): `getAdaptiveWord()`
- Thuật toán chọn từ yếu (weak word selection): `getWeakWord()`
- Thuật toán chọn từ theo loại: `getWordByType()`
- Thuật toán trò chơi Missing Letter: `playMissingLetterGame()` (Tạo từ mới và đối chiếu bằng Hash Table)
- Thuật toán kiểm tra dịch Anh-Việt / Việt-Anh: `englishToVietnameseGame()` và `vietnameseToEnglishGame()`
- Thuật toán cập nhật streak và EXP: `updateStudyStreak()` và `updateLevel()`

### 1. Luồng chương trình chính
`src/main.c`
- Hiển thị màn hình chào mừng và yêu cầu người dùng chọn đăng nhập hoặc đăng ký.
- Nếu chọn đăng nhập: `login()` kiểm tra tài khoản `admin/admin` hoặc tìm trong `data/users.txt`.
- Nếu chọn đăng ký: `registerUser()` thêm tài khoản mới và tạo thư mục người dùng.
- Sau đăng nhập, tải từ điển vào Bảng băm (Hash Table) từ `data/dictionary.txt` và dữ liệu người dùng.
- Hiển thị menu chính với các lựa chọn:
  1. Dictionary Menu
  2. Flashcard Mode
  3. Game Center
  4. Show Stats
  5. Exit
- Khi thoát, lưu progress, user data và dictionary.

### 2. Quản lý từ điển
`src/dictionary.c`
- `createHashTable()`: Khởi tạo bảng băm với mảng buckets rỗng.
- `hashFunction(...)`: Hàm băm (hash) để ánh xạ chuỗi từ vựng thành index.
- `createWord(...)`: tạo node `Word` chứa thông tin chi tiết của từ.
- `insertWord(...)`: tính mã băm và chèn từ mới vào đầu bucket (Collision resolution bằng chaining).
- `displayDictionary(...)`: duyệt toàn bộ bảng băm và in các từ.
- `searchWord(...)`: tra cứu từ trong bảng băm bằng `hashFunction` với tốc độ O(1).
- `suggestWords(...)`: quét qua bảng băm để tìm tối đa 20 từ khớp với tiền tố.
- `getRandomWord(...)`: chọn ngẫu nhiên một từ từ bảng băm.
- `getAdaptiveWord(...)`: thuật toán chọn từ học:
  - 70% cơ hội chọn từ yếu (wrongCount >= 3 và learned == 0)
  - Nếu không có từ yếu hoặc không chọn, trả về từ ngẫu nhiên.
- `getWeakWord(...)`: tập hợp tất cả từ yếu (wrongCount >= 3 và learned == 0), sau đó chọn ngẫu nhiên.
- `getWordByType(...)`: lọc từ theo `type` (noun/verb/adjective/adverb) và chọn ngẫu nhiên.
- `freeHashTable(...)`: giải phóng toàn bộ vùng nhớ của Bảng băm.
- `saveDictionary(...)`: quét toàn bộ Bảng băm và ghi lại `data/dictionary.txt`.
- `addWord(...)`: 
  - nhập từ bằng `scanf` và kiểm tra trùng lặp bằng `searchWord()`.
  - sử dụng `fgets()` để nhận nghĩa, phát âm, và loại từ.
  - thêm từ vào bảng băm.
- `editWord(...)`: tìm từ trong bảng băm, nhập lại nghĩa, phát âm và loại từ, cập nhật thông tin.
- `deleteWord(...)`: tìm node theo từ trong bucket, bỏ node khỏi bucket và giải phóng bộ nhớ.
- `updateLevel()`: tính cấp độ dựa trên EXP: `level = exp / 100 + 1`.

### 3. Lưu và tải dữ liệu người dùng
`src/file.c`
- `loadDictionary(...)`: mở `data/dictionary.txt`, tách dòng theo `|`, tạo từ mới và chèn vào bảng băm.
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
  - `mode 1`: dùng `getAdaptiveWord(ht)` để chọn từ yếu ưu tiên 70%.
  - `mode 2-5`: dùng `getWordByType(ht, <type>)` để chọn ngẫu nhiên theo loại.
  - `mode 6`: dùng `getWeakWord(ht)` để chọn từ yếu.
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
  - chọn ngẫu nhiên 1 từ (ví dụ: `cut`).
  - ẩn một chữ cái bằng dấu `_` (ví dụ: `c_t`).
  - người chơi nhập chữ cái thiếu (ví dụ: `a`).
  - chương trình ghép chữ cái vào tạo thành từ mới (`cat`).
  - tra cứu từ mới này trong Bảng Băm bằng `searchWord(ht, "cat")`.
  - nếu hợp lệ (có trong từ điển): hiển thị thông tin từ `cat`, cộng EXP.
  - nếu không hợp lệ: báo lỗi chính xác `"Không có từ này trong tiếng Anh"`, tăng `wrongCount`.
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
- `isValidUsername(...)`: username phải dài 3-50 ký tự.
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

### Thuật toán
 ### 1. Thuật toán xác thực tài khoản (Authentication Algorithm)

* Sử dụng so sánh chuỗi để kiểm tra username/password
* Áp dụng xác thực dữ liệu đầu vào (Input Validation)

---

### 2. Thuật toán kiểm tra tính hợp lệ dữ liệu (Validation Algorithm)

* Kiểm tra username hợp lệ
* Kiểm tra độ mạnh mật khẩu
* Kiểm tra dữ liệu rỗng / ký tự đặc biệt

---

### 3. Cấu trúc dữ liệu Bảng Băm (Hash Table)

* Lưu trữ từ điển bằng cấu trúc Bảng băm (Hash Table) kết hợp mảng `buckets`.
* Xử lý đụng độ (Collision) bằng phương pháp Chaining (Danh sách liên kết phụ).
* Hỗ trợ thời gian tra cứu trung bình $O(1)$ cho các thao tác:
  * Insert (Thêm từ)
  * Delete (Xóa từ)
  * Search (Tìm kiếm từ)

---

### 4. Thuật toán tìm kiếm tuần tự

(Linear Search Algorithm)

* Dùng để:

  * tìm từ
  * chỉnh sửa từ
  * xóa từ
  * kiểm tra từ tồn tại

---

### 5. Thuật toán Prefix Matching

(Tiền tố chuỗi)

* Dùng cho chức năng gợi ý từ
* So khớp ký tự đầu chuỗi
* Tương tự cơ chế autocomplete

---

### 6. Thuật toán Random Selection

(Chọn ngẫu nhiên)

* Chọn từ ngẫu nhiên
* Chọn câu hỏi ngẫu nhiên
* Sử dụng rand()

---

### 7. Thuật toán Adaptive Learning

(Học thích nghi)

* Ưu tiên từ người dùng học yếu

* Dựa trên:

  * wrongCount
  * learned status

* Tương tự cơ chế:

  * Duolingo
  * Quizlet

---

### 8. Thuật toán Spaced Repetition cơ bản

(Lặp lại ngắt quãng)

* Từ sai nhiều xuất hiện lại thường xuyên hơn
* Từ đã thuộc xuất hiện ít hơn

---

### 9. Thuật toán Weak Word Selection

(Chọn từ yếu)

* Chọn từ có:

  * wrongCount cao
  * learned = 0

* Tăng khả năng ghi nhớ

---

### 10. Thuật toán phân loại dữ liệu

(Category Filtering Algorithm)

* Lọc từ theo:

  * noun
  * verb
  * adjective
  * adverb

---

### 11. Thuật toán Question-Answer Validation

* So sánh đáp án người dùng

* Hỗ trợ:

  * English -> Vietnamese
  * Vietnamese -> English

* Sử dụng:

  * strcmp()
  * _stricmp()

---

### 12. Thuật toán Missing Character Game

* Ẩn ký tự trong từ
* Người dùng đoán ký tự bị thiếu
* Dạng word puzzle algorithm

---

### 13. Thuật toán Experience & Level System

(EXP/Level Progression)

* EXP tăng theo hoạt động học
* Level tăng theo EXP threshold

---

### 14. Thuật toán Study Streak Tracking

* Theo dõi số ngày học liên tiếp

* So sánh:

  * ngày hiện tại
  * ngày học gần nhất

* Tương tự streak system của Duolingo

---

### 15. Thuật toán Daily Mission Tracking

* Theo dõi:

  * số từ học
  * số flashcard review
  * số game đã chơi

* Dùng bộ đếm nhiệm vụ

---

### 16. Thuật toán Thống kê học tập

(Learning Analytics)

* Tính:

  * Learning Rate
  * Learned Words
  * Weak Words
  * Accuracy

---

### 17. Thuật toán File Persistence

(Lưu trữ dữ liệu file)

* Đọc/Ghi dữ liệu bằng:

  * fopen()
  * fscanf()
  * fprintf()

* Lưu:

  * dictionary
  * progress
  * stats
  * user data

---

### 18. Thuật toán State Persistence

* Giữ trạng thái học tập sau khi thoát chương trình
* Khôi phục dữ liệu khi đăng nhập lại

---

### 19. Thuật toán Console UI Rendering

* Vẽ menu console
* Tô màu giao diện
* Hiển thị dashboard realtime

---

### 20. UTF-8 Console Encoding Handling

* Xử lý hiển thị Unicode console Windows
* Hỗ trợ box drawing + UTF-8
