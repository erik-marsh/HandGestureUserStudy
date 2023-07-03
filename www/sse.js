"use strict";

const eventSourceEndpoint = "/eventPusher";
const eventSource = new EventSource(eventSourceEndpoint);

eventSource.onmessage = e => {
    console.log("[SSE] Received unknown message from " + eventSourceEndpoint + ". Details...");
    console.log(e);
};

eventSource.addEventListener("proceed", e => {
    console.log("[SSE] Received proceed message.");
    console.log(e);
    setTimeout(() => { location.reload(); }, 500);
});