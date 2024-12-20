document.addEventListener("DOMContentLoaded", () => {
    const chatHeader = document.querySelector(".chat-header h3");
    const chatBody = document.querySelector(".chat-body");
    const textarea = document.querySelector("textarea");
    const sendButton = document.querySelector(".send-btn");
    const chatList = document.querySelector(".chat-list");

    // Lưu trữ tin nhắn và phòng chat
    const chatData = {};
    let currentRoom = null;

    // Kết nối WebSocket
    const socket = new WebSocket("ws://localhost:8080");

    socket.onopen = () => {
        console.log("WebSocket connected.");
        const username = sessionStorage.getItem("username");
        if (username) {
            usernameDisplay.textContent = username; // Hiển thị tên người dùng
            socket.send(`list_rooms ${sessionStorage.getItem("user_id")}`); // hiển thị danh sách phòng chat
        }
    };

    socket.onmessage = (event) => {
        const response = event.data;

        // Hiển thị danh sách phòng chat
        chatList.innerHTML = ""; // Xóa danh sách cũ
        const rooms = response.split("\n").filter(line => line.trim() !== "");
        // console.log(rooms);

        rooms.forEach((room) => {
            const chatItem = document.createElement("div");
            chatItem.className = "chat-item";

            chatItem.innerHTML = `
                <img src="https://via.placeholder.com/50" alt="Room Avatar">
                <div class="chat-info">
                    <h4>${room}</h4>
                    <span class="status online"></span>
                </div>
            `;

            chatList.appendChild(chatItem);

            // Xử lý sự kiện chọn phòng
            chatItem.addEventListener("click", () => {
                loadRoomMessages(room);
            });
        });

        // Tự động chọn phòng đầu tiên nếu có
        // if (rooms.length > 0) {
        //     loadRoomMessages(rooms[0]);
        // }
    };

    socket.onerror = (error) => console.error("WebSocket error:", error);

    // Hàm tải tin nhắn của phòng chat
    function loadRoomMessages(roomName) {
        currentRoom = roomName;
        chatHeader.textContent = roomName;

        // Xóa khung chat cũ
        chatBody.innerHTML = "";

        // Yêu cầu tin nhắn từ server
        socket.send(`get_messages ${roomName}`);
    }

    // Nhận tin nhắn từ server
    socket.onmessage = (event) => {
        const data = event.data;

        if (data.startsWith("MESSAGE")) {
            const [_, sender, message] = data.split(" ", 3);

            // Hiển thị tin nhắn trong khung chat
            const messageDiv = document.createElement("div");
            messageDiv.classList.add("message", sender === "you" ? "sent" : "received");
            messageDiv.innerHTML = `<p>${message}</p>`;
            chatBody.appendChild(messageDiv);

            // Cuộn xuống cuối
            chatBody.scrollTop = chatBody.scrollHeight;
        }
    };

    // Gửi tin nhắn
    sendButton.addEventListener("click", () => {
        const message = textarea.value.trim();
        if (message && currentRoom) {
            socket.send(`room_message ${currentRoom} ${message}`);

            // Hiển thị tin nhắn người gửi
            const newMessage = document.createElement("div");
            newMessage.classList.add("message", "sent");
            newMessage.innerHTML = `<p>${message}</p>`;
            chatBody.appendChild(newMessage);

            // Xóa textarea và cuộn xuống
            textarea.value = "";
            chatBody.scrollTop = chatBody.scrollHeight;
        }
    });

    // Gửi tin nhắn khi nhấn Enter
    textarea.addEventListener("keypress", (e) => {
        if (e.key === "Enter" && !e.shiftKey) {
            e.preventDefault();
            sendButton.click();
        }
    });
});
