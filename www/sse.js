"use strict";

const eventSourceEndpoint = "/eventPusher";
const eventSource = new EventSource(eventSourceEndpoint);

eventSource.addEventListener("open", e => {
    console.log("[SSE] Connection opened.");
    console.log(e);
});

eventSource.addEventListener("error", e => {
    console.log("[SSE] Connection errored.");
    console.log(e);
});

// captures events of type message AND events WITHOUT a type
// but nothing else
eventSource.addEventListener("message", e => {
    console.log("[SSE] Received unknown message from " + eventSourceEndpoint + ". Details...");
    console.log(e);
});

eventSource.addEventListener("qux", e => {
    console.log(e);
})

eventSource.addEventListener("proceed", e => {
    console.log("[SSE] Received proceed message.");
    console.log(e);
    setTimeout(() => { location.reload(); }, 500);
});