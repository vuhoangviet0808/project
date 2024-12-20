let ws = null;
const url = "ws://localhost:8080";
const connectedPorts = [];

// Log for debugging
function log(message) {
    console.log(`[SharedWorker]: ${message}`);
}

self.addEventListener("connect", (event) => {
    const port = event.ports[0];
    connectedPorts.push(port);
    log("New client connected.");

    port.addEventListener("message", (event) => {
        const message = event.data;
        log(`Message from client: ${message}`);

        if (message === "start") {
            if (!ws) {
                log("Initializing WebSocket connection...");
                ws = new WebSocket(url);

                ws.onopen = () => {
                    log("WebSocket connection established.");
                    connectedPorts.forEach((p) => p.postMessage("WebSocket connected."));
                };

                ws.onmessage = (event) => {
                    log(`WebSocket message: ${event.data}`);
                    connectedPorts.forEach((p) => p.postMessage(event.data));
                };

                ws.onerror = (error) => {
                    log(`WebSocket error: ${error.message}`);
                    connectedPorts.forEach((p) =>
                        p.postMessage(`WebSocket error: ${error.message}`)
                    );
                };

                ws.onclose = () => {
                    log("WebSocket connection closed.");
                    connectedPorts.forEach((p) => p.postMessage("WebSocket connection closed."));
                    ws = null;
                };
            } else {
                log("Reusing existing WebSocket connection.");
                port.postMessage("Reusing WebSocket connection.");
            }
        } else if (message === "stop") {
            if (ws) {
                log("Closing WebSocket connection.");
                ws.close();
                ws = null;
                port.postMessage("WebSocket connection stopped.");
            } else {
                port.postMessage("No active WebSocket connection.");
            }
        } else {
            // Forward messages to WebSocket
            if (ws && ws.readyState === WebSocket.OPEN) {
                log(`Sending message to WebSocket: ${message}`);
                ws.send(message);
            } else {
                log("WebSocket is not connected.");
                port.postMessage("WebSocket is not connected.");
            }
        }
    });

    port.start();
    port.postMessage("SharedWorker ready.");
});
