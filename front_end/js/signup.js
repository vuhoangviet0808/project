document.addEventListener("DOMContentLoaded", () => {
    const switchBtn = document.querySelector(".authen-form__switach-btn");

    switchBtn.addEventListener("click", () => {
        window.location.href = "login.html";
    });

    // document.getElementById("signup-form").addEventListener("submit", (e) => {
    //     e.preventDefault();

    //     const username = document.getElementById("username").value;
    //     const password = document.getElementById("password").value;
    //     const confirmPassword = document.getElementById("confirm-password").value;

    //     if (password !== confirmPassword) {
    //         alert("Mật khẩu không khớp!");
    //         return;
    //     }

    //     fetch("http://localhost:8080", {
    //         method: "POST",
    //         headers: { "Content-Type": "application/json" },
    //         body: JSON.stringify({ command: "signup", username, password }),
    //     })
    //         .then((response) => response.text())
    //         .then((data) => {
    //             alert(data);
    //             if (data.trim() === "Registration successful") {
    //                 window.location.href = "login.html";
    //             }
    //         });
    // });
});
