document.addEventListener("DOMContentLoaded", () => {
    const chatItems = document.querySelectorAll(".chat-item");
    const chatHeader = document.querySelector(".chat-header h3");
    const chatBody = document.querySelector(".chat-body");
    const textarea = document.querySelector("textarea");
    const sendButton = document.querySelector(".send-btn");

    // Lưu trữ tin nhắn cho từng người
    const chatData = {
        "Vũ Hoàng Việt": [
            { type: "received", text: "Hi" },
            { type: "sent", text: "Hello" },
        ],
        "Hà Tuấn Hoàng": [
            { type: "received", text: "How are you?" },
        ],
    };

    // Chức năng chuyển khung chat
    chatItems.forEach((item) => {
        item.addEventListener("click", () => {
            // Xóa trạng thái `active` khỏi tất cả các chat-item
            chatItems.forEach((el) => el.classList.remove("active"));
            item.classList.add("active");

            // Lấy tên người dùng được chọn
            const userName = item.querySelector(".chat-info h4").textContent;
            chatHeader.textContent = userName;

            // Hiển thị tin nhắn trong `chatBody`
            chatBody.innerHTML = ""; // Xóa các tin nhắn cũ
            const messages = chatData[userName] || [];
            messages.forEach((msg) => {
                const messageDiv = document.createElement("div");
                messageDiv.classList.add("message", msg.type);
                messageDiv.innerHTML = `<p>${msg.text}</p>`;
                chatBody.appendChild(messageDiv);
            });

            // Cuộn xuống cuối khung chat
            chatBody.scrollTop = chatBody.scrollHeight;
        });
    });

    // Chức năng gửi tin nhắn
    sendButton.addEventListener("click", () => {
        const message = textarea.value.trim();
        if (message) {
            // Lấy tên người dùng hiện tại từ header
            const userName = chatHeader.textContent;

            // Thêm tin nhắn vào `chatData`
            if (!chatData[userName]) chatData[userName] = [];
            chatData[userName].push({ type: "sent", text: message });

            // Hiển thị tin nhắn trên `chatBody`
            const newMessage = document.createElement("div");
            newMessage.classList.add("message", "sent");
            newMessage.innerHTML = `<p>${message}</p>`;
            chatBody.appendChild(newMessage);

            // Xóa nội dung ô nhập và cuộn xuống cuối
            textarea.value = "";
            chatBody.scrollTop = chatBody.scrollHeight;

            // Giả lập nhận phản hồi (tùy chọn)
            setTimeout(() => {
                const replyMessage = document.createElement("div");
                replyMessage.classList.add("message", "received");
                replyMessage.innerHTML = `<p>Received: ${message}</p>`;
                chatBody.appendChild(replyMessage);
                chatBody.scrollTop = chatBody.scrollHeight;

                // Lưu tin nhắn phản hồi vào `chatData`
                chatData[userName].push({ type: "received", text: `Received: ${message}` });
            }, 1000);
        }
    });

    // Cho phép nhấn Enter để gửi tin nhắn
    textarea.addEventListener("keypress", (e) => {
        if (e.key === "Enter" && !e.shiftKey) {
            e.preventDefault();
            sendButton.click();
        }
    });
});
