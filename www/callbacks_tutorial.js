"use strict";

// This is a modified version of callbacks.js that strips out anything
// that affects the control flow of the user study.
// This file is intended to be used with the tutorial.

///////////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////////

const sendEventsToServer = (eventObject, eventType) => {
    console.log("[DEBUG] Sending events to server...");
    console.log(eventObject);
    fetch(`/events/${eventType}`, {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(eventObject)
    }).then(res => {
        console.log("[DEBUG] Done. Got response " + res.status);
    });
};

///////////////////////////////////////////////////////////////////////////////
// Global state
///////////////////////////////////////////////////////////////////////////////

// load the fields embedded within the page
const userStudyFields = document.getElementsByClassName("user-study-field");
const userStudyTextFields = document.getElementsByClassName("user-study-field-text");
const userStudyButtons = document.getElementsByClassName("user-study-field-button");

// Bootstrap 5 broke the CSS that lined up the input and expectd textareas,
// so we manually set the input field's size in JS now
const resizeListener = (userStudyTextField) => {
    const inputTextarea = userStudyTextField.getElementsByClassName("input")[0];
    const expectedTextarea = userStudyTextField.getElementsByClassName("expected")[0];
    inputTextarea.setAttribute("style", `position: absolute; z-index: 10; width: ${expectedTextarea.offsetWidth}px`);
};

const resizeHandler = () => {
    Array.from(userStudyTextFields).forEach(field => resizeListener(field));
};
window.onresize = resizeHandler;
resizeHandler();  // to handle initial page load

///////////////////////////////////////////////////////////////////////////////
// Keystroke listener
///////////////////////////////////////////////////////////////////////////////

Array.from(userStudyTextFields).forEach(field => {
    const fieldIndex = parseInt(field.getAttribute("data-field-index"));
    const inputTextarea = field.getElementsByClassName("input")[0];
    const expectedTextarea = field.getElementsByClassName("expected")[0];
    // the way the text field is set up in HTML may yield
    // leading and trailing whitespace, so we trim those out.
    const expectedString = expectedTextarea.value.trim();

    const button = field.getElementsByTagName("button")[0];
    button.addEventListener("click", e => {
        inputTextarea.value = "";
        inputTextarea.removeAttribute("disabled");
        inputTextarea.setAttribute("data-input-state", "progress");
        button.setAttribute("disabled", "");
    });

    // keeping this for initialization's sake
    if (fieldIndex === 0) {
        inputTextarea.removeAttribute("disabled");
        inputTextarea.setAttribute("data-input-state", "progress");
    }

    const listener = e => {
        const equalSoFar = expectedString.startsWith(inputTextarea.value);
        const equality = inputTextarea.value === expectedString;

        if (equalSoFar) {
            inputTextarea.setAttribute("data-input-state", "progress");
        } else {
            inputTextarea.setAttribute("data-input-state", "error");
        }

        if (equality) {
            inputTextarea.setAttribute("disabled", "");
            inputTextarea.setAttribute("data-input-state", "completed");
            button.removeAttribute("disabled");
        }
    };
    field.addEventListener("keyup", listener);
});

