"use strict";

// TODO: need to PREVENT input when the user clicks the wrong field

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

// load the number of fields, used for tracking progress
const totalFields = Array.from(userStudyFields).length;

// shared containers
let clickEvents = [];

// variable state
// should not be accessed directly, only by the proxy declared below
let __state = {
    currentField: 0
};

const __stateHandler = {
    set(obj, prop, value) {
        console.log("[DEBUG] Property " + prop + " of " + obj + " updated from " + obj[prop] + " to " + value);

        if (prop === "currentField") {
            const completionTime = Date.now();

            // send field completion event
            console.log("[Study Control] Field completed.");
            sendEventsToServer({
                timestampMillis: completionTime,
                fieldIndex: obj[prop],
            }, "field");

            // if we have now done all fields
            if (value === totalFields) {
                console.log("[Study Control] Task completed.");
                sendEventsToServer(clickEvents, "click");
                clickEvents = [];
                sendEventsToServer({
                    timestampMillis: completionTime,
                    taskIndex: -1,  // TODO: idk how i want to retrieve this value tbh
                }, "task");
            }

            // then enable the next field
            const field = userStudyFields[value];
            console.log(field);
            if (Array.from(userStudyTextFields).includes(field)) {
                const inputSubfield = field.getElementsByClassName("input")[0];
                inputSubfield.removeAttribute("disabled");
                inputSubfield.setAttribute("data-input-state", "progress");
            }
            else if (Array.from(userStudyButtons).includes(field)) {
                const buttonSubfield = field.getElementsByTagName("button")[0];
                buttonSubfield.removeAttribute("disabled");
            }
        }

        return Reflect.set(...arguments);
    }
};

let state = new Proxy(__state, __stateHandler);


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

    if (fieldIndex === 0) {
        inputTextarea.removeAttribute("disabled");
        inputTextarea.setAttribute("data-input-state", "progress");
    }

    const makeKeystrokeEvent = (timestampMillis, keyupEvent, wasCorrect) => ({
        timestampMillis: timestampMillis,
        wasCorrect: wasCorrect,
        key: keyupEvent.key
    });

    let gatheredInputs = [];

    const listener = e => {
        const timestampMillis = Date.now();
        const equalSoFar = expectedString.startsWith(inputTextarea.value);
        const equality = inputTextarea.value === expectedString;

        gatheredInputs.push(makeKeystrokeEvent(timestampMillis, e, equalSoFar));

        if (equalSoFar) {
            inputTextarea.setAttribute("data-input-state", "progress");
        } else {
            inputTextarea.setAttribute("data-input-state", "error");
        }

        if (equality) {
            console.log("[Keystrokes] Field has been completed, moving on...");
            inputTextarea.setAttribute("disabled", "");
            inputTextarea.setAttribute("data-input-state", "completed");
            field.removeEventListener("keyup", listener);

            sendEventsToServer(gatheredInputs, "keystroke");
            state.currentField++;
        }
    };
    field.addEventListener("keyup", listener);
});


///////////////////////////////////////////////////////////////////////////////
// Click listeners
///////////////////////////////////////////////////////////////////////////////

// TODO: click on buttons miss when the button is disabled a priori to their activation
// otherwise everything seems to be in order

// missed clicks will not be caught by the other handler and will bubble up to here
document.addEventListener("click", e => {
    const timestampMillis = Date.now();
    clickEvents.push({
        timestampMillis: timestampMillis,
        location: "Background",
        wasCorrect: false
    });

    console.log("[Clicks] Click missed.");
});

Array.from(userStudyFields).forEach(field => {
    const fieldIndex = Number.parseInt(field.getAttribute("data-field-index"));

    field.addEventListener("click", e => {
        const timestampMillis = Date.now();
        const wasCorrect = fieldIndex === state.currentField;

        let clickLocation = "";
        if (field.classList.contains("user-study-field-text"))
            clickLocation = "TextField";
        else if (field.classList.contains("user-study-field-button"))
            clickLocation = "Button";
        else
            clickLocation = "OutOfBounds";

        const newClick = {
            timestampMillis: timestampMillis,
            location: clickLocation,
            wasCorrect: wasCorrect
        };
        clickEvents.push(newClick);
        console.log("[Clicks] Click successful on fieldIndex=" + fieldIndex + " [" + (wasCorrect ? "correct field" : "incorrect field") + "]");

        e.stopPropagation();
    });
});

// listeners for buttons
// clicking the buttons should be considered "moving on"
// TODO: need to change button colors as well
Array.from(userStudyButtons).forEach(field => {
    const fieldIndex = Number.parseInt(field.getAttribute("data-field-index"));

    field.addEventListener("click", e => {
        const wasCorrect = fieldIndex === state.currentField;
        if (wasCorrect) {
            console.log("[Clicks] Clicked the correct button.");
            state.currentField++;
        }
    });
});
