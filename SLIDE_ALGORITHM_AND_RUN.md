# English Dictionary & Vocabulary Game

## 1. Giới thiệu đề tài

Ứng dụng được viết bằng ngôn ngữ C, chạy trên console Windows. Chương trình hỗ trợ người dùng học từ vựng tiếng Anh thông qua từ điển, flashcard, trò chơi ôn tập và hệ thống thống kê tiến độ.

Chức năng chính:

- Đăng nhập, đăng ký tài khoản người dùng.
- Quản lý từ điển: thêm, sửa, xóa, tìm kiếm, hiển thị từ.
- Học bằng flashcard theo nhiều chế độ.
- Chơi game học từ: dịch Anh - Việt, Việt - Anh, Missing Letter.
- Lưu tiến độ học tập, EXP, level, streak và nhiệm vụ hằng ngày.

## 2. Cấu trúc dữ liệu chính

### Word

Mỗi từ vựng được lưu bằng struct `Word`, gồm:

- `word`: từ tiếng Anh.
- `meaning`: nghĩa tiếng Việt, có thể có nhiều nghĩa ngăn cách bằng dấu `;`.
- `pronunciation`: phiên âm.
- `type`: loại từ, ví dụ `noun`, `verb`, `adjective`, `adverb`.
- `learned`: trạng thái đã học hay chưa.
- `wrongCount`: số lần trả lời sai.
- `next`: con trỏ liên kết khi xảy ra đụng độ trong bảng băm.

### Hash Table

Từ điển được lưu trong `HashTable` với kích thước `HASH_SIZE = 10007`.

- Mỗi từ được băm thành một chỉ số bucket bằng hàm `hashFunction`.
- Nếu nhiều từ có cùng chỉ số, chương trình dùng danh sách liên kết để xử lý đụng độ.
- Tìm kiếm trung bình nhanh, gần `O(1)`.

### BST

Bên cạnh Hash Table, chương trình dùng thêm cây nhị phân tìm kiếm `BSTNode`.

- Lưu các từ theo thứ tự bảng chữ cái.
- Hỗ trợ hiển thị từ điển A-Z.
- Hỗ trợ gợi ý từ theo tiền tố.

## 3. Thuật toán bảng băm

Hàm `hashFunction` dùng ý tưởng djb2:

1. Khởi tạo `hash = 5381`.
2. Duyệt từng ký tự của từ.
3. Chuyển ký tự về chữ thường để tìm kiếm không phân biệt hoa thường.
4. Cập nhật hash theo công thức: `hash = hash * 33 + c`.
5. Lấy phần dư với `HASH_SIZE` để ra vị trí bucket.

Ứng dụng:

- `insertWord`: chèn từ vào bucket tương ứng.
- `searchWord`: băm từ cần tìm, vào đúng bucket và so sánh trong danh sách liên kết.
- `deleteWord`: tìm node trong bucket, ngắt liên kết và giải phóng bộ nhớ.

## 4. Thuật toán BST và gợi ý từ

Khi thêm từ mới, chương trình đồng thời:

- Chèn từ vào Hash Table để tra cứu nhanh.
- Chèn từ vào BST để sắp xếp và gợi ý.

Hiển thị từ điển:

1. Duyệt BST theo thứ tự `left -> node -> right`.
2. Mỗi node trong BST chỉ lưu từ.
3. Gọi `searchWord` để lấy đầy đủ nghĩa, phiên âm và loại từ.
4. In danh sách từ theo thứ tự A-Z.

Gợi ý theo tiền tố:

1. Người dùng nhập prefix.
2. Chương trình duyệt BST và so sánh tiền tố bằng `_strnicmp`.
3. Nếu từ bắt đầu bằng prefix, lưu vào mảng kết quả.
4. Trả về tối đa 20 gợi ý.

## 5. Thuật toán học thích nghi

Chương trình ưu tiên cho người dùng ôn lại những từ hay sai.

Điều kiện từ yếu:

- `wrongCount >= 3`.
- `learned == 0`.

Hàm `getAdaptiveWord` hoạt động như sau:

1. Sinh số ngẫu nhiên từ 0 đến 99.
2. Nếu số này nhỏ hơn 70, ưu tiên chọn từ yếu.
3. Nếu không có từ yếu, chọn một từ ngẫu nhiên trong từ điển.

Ý nghĩa:

- Từ sai nhiều sẽ xuất hiện lại thường xuyên hơn.
- Từ đã học tốt sẽ ít bị lặp lại hơn.
- Tạo cơ chế học gần giống spaced repetition cơ bản.

## 6. Thuật toán flashcard

Flashcard có các chế độ:

- Adaptive: chọn từ bằng `getAdaptiveWord`.
- Nouns Only: chọn danh từ.
- Verbs Only: chọn động từ.
- Adjectives Only: chọn tính từ.
- Adverbs Only: chọn trạng từ.
- Weak Words Only: chỉ chọn từ yếu.

Quy trình học:

1. Hiển thị mặt trước là từ tiếng Anh.
2. Người dùng nhấn Enter để xem mặt sau.
3. Mặt sau hiện nghĩa, loại từ và phiên âm.
4. Người dùng tự đánh giá:
   - Again: tăng `wrongCount`, đánh dấu chưa thuộc.
   - Good: giảm nhẹ `wrongCount`, cộng EXP.
   - Easy: đánh dấu đã học, reset `wrongCount`, cộng EXP cao hơn.

## 7. Thuật toán trò chơi

### English -> Vietnamese

1. Chọn một từ người dùng đã từng học.
2. Yêu cầu nhập nghĩa tiếng Việt.
3. Tách các nghĩa trong `meaning` bằng dấu `;`.
4. So sánh đáp án không phân biệt hoa thường.
5. Đúng thì cộng EXP, sai thì hiện đáp án đúng.

### Vietnamese -> English

1. Chọn một từ đã học.
2. Hiện nghĩa tiếng Việt.
3. Người dùng nhập từ tiếng Anh.
4. So sánh với `word` bằng `_stricmp`.
5. Đúng thì cộng EXP, sai thì hiện đáp án đúng.

### Missing Letter

1. Người dùng chọn độ khó:
   - Standard: ẩn 1 chữ cái.
   - Challenge: ẩn 2 chữ cái.
   - Expert: ẩn 3 chữ cái.
2. Chương trình chọn từ có độ dài phù hợp.
3. Ẩn ngẫu nhiên một số ký tự bằng dấu `_`.
4. Người dùng nhập từ đầy đủ hoặc các chữ cái bị thiếu.
5. Chương trình ghép lại và kiểm tra bằng `searchWord`.
6. Nếu từ hợp lệ thì cộng EXP, nếu sai thì tăng `wrongCount`.

## 8. Lưu trữ dữ liệu

Dữ liệu được lưu trong thư mục `data`.

File `data/dictionary.txt`:

```text
word|meaning|pronunciation|type
```

File `data/users.txt`:

```text
username|password
```

Mỗi người dùng có thư mục riêng:

```text
data/Users/<username>/userdata.txt
data/Users/<username>/progress.txt
```

`userdata.txt` lưu:

- Nhiệm vụ hằng ngày.
- Ngày học gần nhất và streak.
- Level và EXP.

`progress.txt` lưu:

- Từ đã học hay chưa.
- Số lần sai của từng từ.

## 9. Luồng chạy chương trình

1. Mở chương trình.
2. Hiển thị màn hình đăng nhập/đăng ký.
3. Nếu đăng nhập thành công:
   - Nạp dữ liệu người dùng.
   - Tạo nhiệm vụ ngày nếu cần.
   - Nạp từ điển vào Hash Table và BST.
   - Nạp tiến độ học tập.
4. Hiển thị menu chính:
   - Dictionary Menu.
   - Flashcard Mode.
   - Game Center.
   - Show Stats.
   - Exit.
5. Khi thoát:
   - Lưu từ điển.
   - Lưu tiến độ.
   - Lưu thông tin người dùng.
   - Giải phóng bộ nhớ.

## 10. Cách biên dịch và chạy

Yêu cầu:

- Windows.
- GCC, ví dụ MinGW/MSYS2.

Cách 1: chạy file có sẵn:

```bat
run.bat
```

File `run.bat` sẽ:

1. Thêm đường dẫn GCC vào `PATH`.
2. Biên dịch tất cả file trong `src`.
3. Tạo file `build/app.exe`.
4. Chạy chương trình.

Cách 2: gõ lệnh thủ công:

```bat
gcc src/*.c -o build/app.exe
build\app.exe
```

Nếu muốn chạy file đã build sẵn:

```bat
build\app.exe
```

Hoặc chạy file ở thư mục gốc:

```bat
run.exe
```

## 11. Tài khoản và dữ liệu mẫu

Tài khoản admin mặc định:

```text
username: admin
password: admin
```

Người dùng mới có thể đăng ký trực tiếp trong chương trình. Mật khẩu cần:

- Ít nhất 6 ký tự.
- Có chữ hoa.
- Có chữ thường.
- Có chữ số.

## 12. Gợi ý chia slide

Slide 1: Tên đề tài và mục tiêu.

Slide 2: Chức năng chính của ứng dụng.

Slide 3: Cấu trúc dữ liệu `Word`, Hash Table và BST.

Slide 4: Thuật toán Hash Table và tìm kiếm.

Slide 5: Thuật toán BST, sắp xếp A-Z và gợi ý prefix.

Slide 6: Flashcard và thuật toán học thích nghi.

Slide 7: Các game học từ và cách chấm điểm.

Slide 8: Lưu trữ dữ liệu và cách chạy chương trình.
