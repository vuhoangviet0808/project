document.addEventListener("DOMContentLoaded", () => {
    const switchBtn = document.querySelector(".authen-form__switach-btn");

    switchBtn.addEventListener("click", () => {
        window.location.href = "signup.html";
    });

    document.getElementById("login-form").addEventListener("submit", (e) => {
        e.preventDefault();

        const username = document.getElementById("username").value;
        const password = document.getElementById("password").value;

        const ws = new WebSocket('ws://localhost:8080');
        ws.onopen = () => {
            document.getElementById('output').innerText = 'Connected to WebSocket server';
        };

        ws.onmessage = (event) => {
            const output = document.getElementById('output');
            output.innerText += '\nServer: ' + event.data;
        };

        ws.onerror = (error) => {
            console.error('WebSocket Error:', error);
        };

        ws.onclose = () => {
            document.getElementById('output').innerText += '\nConnection closed.';
        };
        function sendMessage() {
            const message = `login${username} ${password}`;
            ws.send(message);
        }

    });
});