#ifndef PROTOCOL_H
#define CMD_REGISTER        0x01  // Đăng ký tài khoản
#define CMD_LOGIN           0x02  // Đăng nhập
#define CMD_CHAT            0x03  // Gửi tin nhắn riêng tư
#define CMD_CHAT_OFFLINE    0x04  // Gửi tin nhắn ngoại tuyến
#define CMD_RETRIEVE        0x05  // Lấy tin nhắn đã gửi/nhận
#define CMD_ADDFR           0x06  // Gửi yêu cầu kết bạn
#define CMD_ACCEPT          0x07  // Chấp nhận yêu cầu kết bạn
#define CMD_DECLINE         0x08  // Từ chối yêu cầu kết bạn
#define CMD_LISTFR          0x09  // Liệt kê danh sách bạn bè
#define CMD_CANCEL          0x0A  // Hủy yêu cầu kết bạn đã gửi
#define CMD_LISTREQ         0x0B  // Liệt kê danh sách yêu cầu kết bạn
#define CMD_REMOVE          0x0C  // Xóa bạn bè
#define CMD_CREATE_ROOM     0x0D  // Tạo phòng chat
#define CMD_JOIN_ROOM       0x0E  // Tham gia phòng chat
#define CMD_ROOM_MESSAGE    0x0F  // Gửi tin nhắn đến phòng chat
#define CMD_ADD_TO_ROOM     0x10  // Thêm người dùng vào phòng chat
#define CMD_LEAVE_ROOM      0x11  // Rời khỏi phòng chat
#define CMD_REMOVE_USER     0x12  // Xóa người dùng khỏi phòng chat
#define CMD_LIST_ROOMS      0x13  // Liệt kê các phòng chat đã tham gia
#define CMD_LOGOUT          0x14

#define STATUS_SUCCESS          0x30 // Thành công
#define STATUS_ERROR            0x31 // Lỗi yêu cầu
#define STATUS_NOT_FOUND        0x32 // Tài nguyên không tồn tại
#define STATUS_SERVER_ERROR     0x33 // Lỗi hệ thống

#define RESPONSE_REGISTER       0x21 // Đăng ký thành công
#define RESPONSE_LOGIN          0x22 // Đăng nhập thành công
#define RESPONSE_CHAT           0x23 // Gửi tin nhắn thành công
#define RESPONSE_CHAT_OFFLINE   0x24 // Gửi tin nhắn ngoại tuyến thành công
#define RESPONSE_RETRIEVE       0x25 // Lấy tin nhắn thành công
#define RESPONSE_ADDFR          0x26 // Gửi yêu cầu kết bạn thành công
#define FAIL_ADDFR              0x46
#define RESPONSE_ACCEPT         0x27 // Chấp nhận yêu cầu kết bạn thành công
#define RESPONSE_DECLINE        0x28 // Từ chối yêu cầu kết bạn thành công
#define RESPONSE_LISTFR         0x29 // Liệt kê danh sách bạn bè thành công
#define RESPONSE_CANCEL         0x2A // Hủy yêu cầu kết bạn thành công
#define RESPONSE_LISTREQ        0x2B // Liệt kê danh sách yêu cầu kết bạn thành công
#define RESPONSE_REMOVE         0x2C // Xóa bạn bè thành công
#define RESPONSE_CREATE_ROOM    0x2D // Tạo phòng chat thành công
#define RESPONSE_JOIN_ROOM      0x2E // Tham gia phòng chat thành công
#define RESPONSE_ROOM_MESSAGE   0x2F // Gửi tin nhắn đến phòng chat thành công
#define RESPONSE_ADD_TO_ROOM    0x30 // Thêm người dùng vào phòng chat thành công
#define RESPONSE_LEAVE_ROOM     0x31 // Rời khỏi phòng chat thành công
#define RESPONSE_REMOVE_USER    0x32 // Xóa người dùng khỏi phòng chat thành công
#define RESPONSE_LIST_ROOMS     0x33 // Liệt kê các phòng chat đã tham gia thành công
#define RESPONSE_LOGOUT         0x34 // Đăng xuất thành công

void add_response_header(char *final_response, int response_header, const char *response_content, size_t response_content_size);

#endif