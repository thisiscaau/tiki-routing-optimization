# TIKI Routing Optimization (hsgs_20)
Cuối tuần vừa rồi, đội của mình đã tham gia vào cuộc thi SOICT Hackathon 2023 tại Đại Học Bách Khoa Hà Nội! Repository này được tạo ra nhằm lưu lại kỷ niệm cũng như để có thể cải tiến trong tương lai.

P/s: code trên được viết vào lúc 3h sáng khi đang xem The International nên là .... good luck.

### Team members
- Lê Minh Khánh ([@thisiscaau] (https://github.com/thisiscaau))
- Vũ Hoàng Minh ([@vhm1605] (https://github.com/vhm1605))
- Đỗ Tuấn Nam ([@NamDT-146] (https://github.com/NamDT-146))
- Mai Đức Mạnh ([@Snuc1925] (https://github.com/Snuc1925))

### Thuật toán vòng sơ khảo
Tiêu chí: tối đa hóa số đơn hàng có thể giao được.

Gọi $vi$ là vector lưu thứ tự xử lý của các đơn hàng. Mỗi phần tử của $vi$ là một cặp $(key,index)$ với $index$ là số hiệu của đơn hàng và $key$ thay đổi theo từng chiến thuật Greedy:
- $ep(index)$ được lấy làm $key$
- $lp(index)$ được lấy làm $key$
- $ed(index)$ được lấy làm $key$
- $ld(index)$ được lấy làm $key$

Vector $vi$ sau đó được sắp xếp theo $key$ hoặc shuffle ngẫu nhiên để xác định được thứ tự xử lý của các đơn hàng. Các bước của thuật toán sau đó diễn ra như sau:
- Xử lý lần lượt các đơn hàng theo thứ tự đã chọn
- Với mỗi đơn hàng $i$, duyệt qua tất cả các xe tải $j$
- Xét tất cả các hành trình có thể của xe tải $j$ khi thêm nhận/giao đơn hàng $i$ vào lộ trình. Các hành trình này được mô phỏng tính khả thi và chấm điểm qua hàm $simulate-path$
- Chọn xe tải $j$ với lộ trình $opt-path$ có điểm tối ưu nhất
- Thêm $i$ vào danh sách các đơn hàng của xe $j$ và đồng thời cập nhật lộ trình của xe $j$ thành $opt-path$
- Nếu không có xe tải nào thỏa mãn, bỏ qua đơn hàng $i$
- Chuyển sang xử lý đơn hàng $i+1$  

## Những thay đổi so với thuật toán đầu tiên

### Hàm "insertion-heuristic"
Khi giới hạn của bài toán lớn dần, việc xét tất cả các hành trình có thể của xe tải $j$ khi thêm nhận/giao đơn hàng $i$ vào lộ trình trở nên khó khăn. Team bọn mình đã quyết định thay chia danh sách các đơn hàng mà xe tải $j$ giao thành các block có kích thước 50 đơn hàng. Với mỗi block, hàm $insertion-heuristic$ từ thuật toán đầu tiên sẽ tìm ra thứ tự để giao tối ưu nhất.

Thuật toán này qua quá trình chạy test local chỉ giao được khoảng 50 đến 60% đơn hàng. Cho nên bọn mình quyết định dùng hàm này để tạo ra những lời giải ban đầu cho thuật toán di truyền bên dưới.

### Thuật toán di truyền

Mỗi lời giải cho bài toán được ví như một cá thể. Ở bên trong lại có các gen tượng trưng cho hành trình của mỗi xe.

![](https://hackmd.io/_uploads/HkaIxwizT.png)

Mô tả thuật toán:
- Tạo ra một tập các lời giải ban đầu $F$ bằng cách dùng hàm Insertion Heuristic ở trên

![](https://hackmd.io/_uploads/SkR4ZDsGp.png)

- Chọn ra ngẫu nhiên hai cá thể $parent1$ và $parent2$ từ tập $F$

- Thực hiện phép lai giữa hai cá thể $parent1$ và $parent2$ để cho ra hai con tương ứng là $child1$ và $child2$
- Thêm $child1$ và $child2$ vào tập $F$. Sắp xếp lại tập $F$ theo hàm chấm điểm $evaluate-solution$
- Xóa 2 cá thể có điểm thấp nhất trong tập $F$
- Lặp lại quá trình này cho đến khi gần chạm mức giới hạn thời gian

#### Phép lai

Phép lai giữa $parent1$ và $parent2$ được thực hiện bằng cách chọn 1 đoạn gen ngẫu nhiên ở trong $parent2$ và chuyển sang $parent1$.

![](https://hackmd.io/_uploads/H1BuVDszp.png)


Khi đó sẽ có thể diễn ra trường hợp các request vận chuyển hàng bị lặp. Do đó chúng ta phải xóa những phần lặp này đi và chỉ giữ lại những request nằm trong phần gen mới được chuyển sang.


Trong quá trình kết hợp hai lời giải như thế này cũng có thể xảy ra trường hợp một số request không còn được gán với xe nào (do xung đột về số hiệu xe chẳng hạn). Khi đó chúng ta sẽ dùng hàm $insertion-heuristic$ để thêm các request này vào lại các xe và sử dụng thêm xe nếu cần thiết.

$Child2$ được tạo ra bằng cách thay đổi vai trò của $parent1$ và $parent2$ cho nhau.

#### Đột biến

Bên cạnh phép lai bình thường, bọn mình cũng có implement một phép đột biến.

Mô tả thuật toán:
- Xóa ngẫu nhiên một gen từ cá thể
- Phân bố các request của xe tải bị xóa vào các xe khác bằng insertion-heuristic

Sau khi có được $child1$ và $child2$, thuật toán sẽ tạo ra bản đột biến của $child1$ và $child2$ với xác suất là 50%.
