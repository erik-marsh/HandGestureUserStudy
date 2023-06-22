"use strict";

const eventSourceEndpoint = "/eventPusher";
const eventSource = new EventSource(eventSourceEndpoint);

eventSource.onmessage = e => {
    console.log("[SSE] Received unknown message from " + eventSourceEndpoint + ". Details...");
    console.log(e);
};

eventSource.addEventListener("keep-alive", e => {
    console.log("[SSE] Received keep-alive (heartbeat) message.");
});

eventSource.addEventListener("proceed", e => {
    console.log("[SSE] Received proceed message.");
    setTimeout(() => { location.reload(); }, 500);
});