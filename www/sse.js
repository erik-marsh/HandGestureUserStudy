"use strict";

const eventSourceEndpoint = "/eventPusher";
const eventSource = new EventSource(eventSourceEndpoint);

eventSource.addEventListener("open", e => {
    console.log("[SSE] Connection opened.");
});

eventSource.addEventListener("error", e => {
    console.log("[SSE] Connection errored.");
});

// captures events of type message AND events WITHOUT a type
// but nothing else
eventSource.addEventListener("message", e => {
    console.log("[SSE] Received unknown message from " + eventSourceEndpoint + ". Details...");
    console.log(e);
});

eventSource.addEventListener("keep-alive", e => {
    console.log(`[SSE] [EventID=${e.lastEventId}] Received keep-alive message.`);
});

eventSource.addEventListener("proceed", e => {
    console.log(`[SSE] [EventID=${e.lastEventId}] Received proceed message.`);
    setTimeout(() => { location.reload(); }, 500);
});