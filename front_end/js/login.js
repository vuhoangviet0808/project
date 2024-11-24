document.addEventListener("DOMContentLoaded", () => {
    const switchBtn = document.querySelector(".authen-form__switach-btn");

    switchBtn.addEventListener("click", () => {
        window.location.href = "signup.html";
    });

    document.getElementById("login-form").addEventListener("submit", (e) => {
        e.preventDefault();

        const username = document.getElementById("username").value;
        const password = document.getElementById("password").value;

        fetch("http://localhost:8080", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ command: "login", username, password }),
        })
            .then((response) => response.text())
            .then((data) => {
                if (data.trim() === "Login successful") {
                    alert("Đăng nhập thành công!");
                    window.location.href = "chat.html";
                } else {
                    alert(data);
                }
            });
    });
});
