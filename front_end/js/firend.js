document.addEventListener("DOMContentLoaded", () => {
    const addFriendInput = document.getElementById("add-friend-input");
    const addFriendBtn = document.getElementById("add-friend-btn");
    addFriendBtn.addEventListener("click", () => {
      const friendId = addFriendInput.value.trim(); 
  
      if (!friendId || isNaN(friendId)) {
        alert("Please enter a valid numeric User ID.");
        return;
      }
  
      const userId = sessionStorage.getItem("user_id");
  
      if (!userId) {
        alert("You must be logged in to send friend requests.");
        return;
      }
  
      // Gửi yêu cầu qua WebSocket
      socket.send(`addfr ${friendId}`);
  
      // Hiển thị trạng thái gửi yêu cầu (tạm thời)
      const statusElement = document.createElement("p");
      statusElement.textContent = "Sending friend request...";
      addFriendInput.parentElement.appendChild(statusElement);
  
      // Xóa trạng thái sau 3 giây (nếu không có phản hồi từ server)
      setTimeout(() => {
        statusElement.remove();
      }, 3000);
    });

    socket.onmessage = (event) => {
        const response = new Uint8Array(event.data)[0]; // Đọc mã byte đầu tiên
        console.log("Received response (hex):", response.toString(16)); // In mã hexa ra console
      
        const statusElement = document.createElement("p");
      
        if (response === 0x26) {
          // Thành công
          statusElement.textContent = "Friend request sent successfully!";
          statusElement.style.color = "green";
        } else if (response === 0x46) {
          // Thất bại
          statusElement.textContent = "Friend request failed. Please try again.";
          statusElement.style.color = "red";
        } else {
          // Phản hồi không mong muốn
          console.error("Unexpected response:", response);
          statusElement.textContent = "An unexpected error occurred.";
          statusElement.style.color = "orange";
        }
      
        // Hiển thị thông báo lên giao diện
        addFriendInput.parentElement.appendChild(statusElement);
      
        // Tự động xóa thông báo sau 5 giây
        setTimeout(() => {
          statusElement.remove();
        }, 5000);
      };
      
      
      
  });
  