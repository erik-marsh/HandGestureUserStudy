"use strict";

///////////////////////////////////////////////////////////////////////////////
// Global state
///////////////////////////////////////////////////////////////////////////////

// load the fields embedded within the page
const userStudyFields = document.getElementsByClassName("user-study-field");
const userStudyTextFields = document.getElementsByClassName("user-study-field-text");
const userStudyButtons = document.getElementsByClassName("user-study-field-button");

// load the number of fields, used for tracking progress
const totalFields = Array.from(userStudyFields).length;

// variable state
// TODO: investigate https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Proxy
let currentField = 0;

// shared containers
let clickEvents = [];


///////////////////////////////////////////////////////////////////////////////
// Helper functions
///////////////////////////////////////////////////////////////////////////////

const sendEventsToServer = eventArray => {
    fetch("/events", {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(eventArray)
    });
};


///////////////////////////////////////////////////////////////////////////////
// Keystroke listener
///////////////////////////////////////////////////////////////////////////////

Array.from(userStudyTextFields).forEach(field => {
    const fieldIndex = field.getAttribute("data-field-index");
    const inputTextarea = field.getElementsByClassName("input")[0];
    const expectedTextarea = field.getElementsByClassName("expected")[0];
    // the way the text field is set up in HTML may yield
    // leading and trailing whitespace, so we trim those out.
    const expectedString = expectedTextarea.value.trim();

    const makeKeystrokeEvent = (timestamp, keyupEvent, wasCorrect) => ({
        "timestamp": timestamp,
        "wasCorrect": wasCorrect,
        "key": keyupEvent.key
    });

    let gatheredInputs = [];
    let completion = {
        "timestamp": null,
        "fieldIndex": fieldIndex
    };

    const listener = e => {
        const timestamp = Date.now();
        const equalSoFar = expectedString.startsWith(inputTextarea.value);
        const equality = inputTextarea.value === expectedString;

        gatheredInputs.push(makeKeystrokeEvent(timestamp, e, equalSoFar));

        inputTextarea.style.color = equalSoFar ? "black" : "red";

        if (equality) {
            completion.timestamp = timestamp;
            console.log("Field has been completed, moving on...");
            inputTextarea.setAttribute("disabled", "");
            inputTextarea.style.color = "green";
            field.removeEventListener("keyup", listener);
            // TODO: need to re-enable background

            const dataToSend = {
                "keystrokes": gatheredInputs,
                "completion": completion
            };

            sendEventsToServer(dataToSend);
        }
    };
    field.addEventListener("keyup", listener);
});

///////////////////////////////////////////////////////////////////////////////
// Click listeners
///////////////////////////////////////////////////////////////////////////////

// missed clicks will not be caught by the other handler and will bubble up to here
document.addEventListener("click", e => {
    const timestampMillis = Date.now();
    clickEvents.push({
        "timestampMillis": timestampMillis,
        "location": "Background",
        "wasCorrect": false
    });

    console.log("click MISSED");
});

Array.from(userStudyFields).forEach(field => {
    field.addEventListener("click", e => {
        const timestampMillis = Date.now();
        const fieldIndex = Number.parseInt(field.getAttribute("data-field-index"));
        const wasCorrect = fieldIndex === currentField;
        
        let clickLocation = "";
        if (field.classList.contains("user-study-field-text"))
            clickLocation = "TextField";
        else if (field.classList.contains("user-study-field-button"))
            clickLocation = "Button";
        else
            clickLocation = "OutOfBounds";
        
        const newClick = {
            "timestampMillis": timestampMillis,
            "location": clickLocation,
            "wasCorrect": wasCorrect
        };
        clickEvents.push(newClick);
        
        console.log(newClick);
        console.log("click HIT on fieldIndex=" + fieldIndex);

        if (wasCorrect)
        {
            currentField++;
        }

        if (currentField == totalFields)
        {
            // send timestamps
            // TODO: we need to respond to field completion, not field clicks
            fetch("/events", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify(clickEvents)
            });

            // send signal to move on
        }
        
        e.stopPropagation();
    });
});