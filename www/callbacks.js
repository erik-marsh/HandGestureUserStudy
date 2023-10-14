"use strict";

///////////////////////////////////////////////////////////////////////////////
// Page elements
///////////////////////////////////////////////////////////////////////////////

// load the fields embedded within the page
const userStudyFields = document.getElementsByClassName("user-study-field");
const userStudyTextFields = document.getElementsByClassName("user-study-field-text");
const userStudyButtons = document.getElementsByClassName("user-study-field-button");
const loadingField = document.getElementById("loading");

// load the number of fields, used for tracking progress
const totalFields = Array.from(userStudyFields).length;

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
// Page initialization
///////////////////////////////////////////////////////////////////////////////

// Bootstrap 5 broke the CSS that lined up the input and expectd textareas,
// so we manually set the input field's size in JS now
const resizeListener = (userStudyTextField) => {
    const inputTextarea = userStudyTextField.getElementsByClassName("input")[0];
    const expectedTextarea = userStudyTextField.getElementsByClassName("expected")[0];
    inputTextarea.setAttribute("style", `width: ${expectedTextarea.offsetWidth}px`);
};

const resizeHandler = () => {
    Array.from(userStudyTextFields).forEach(field => resizeListener(field));
};

window.onresize = resizeHandler;
resizeHandler();  // to handle initial page load


///////////////////////////////////////////////////////////////////////////////
// Global state
///////////////////////////////////////////////////////////////////////////////

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

                loadingField.removeAttribute("style");
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

const navigationKeys = ["ArrowDown", "ArrowUp", "ArrowLeft", "ArrowRight", "End", "Home", "PageDown", "PageUp"];

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

    let gatheredInputs = [];

    let assembledString = "";
    const sink = {
        keystrokeQueue: [],
        timestampQueue: [],
        inputCharQueue: [],
        notify() {
            // TODO: the user must be reminded that they are not to:
            //     Use arrow keys (this is prevented)
            //     Use the delete key (this is prevented)
            //     Use any other key combinations for editing,
            //         such as Shift+Arrow keys, Shift+Home, Shift+End, etc...
            //     (This is largely prevented by disallowing navigation keys)
            // remind me to never do keystroke-level input validation ever again

            // inputChar is null if the input operation did NOT insert text
            // hence if we filtered our control keys properly,
            // we don't need to worry about the queues becoming desynced
            const keystroke = this.keystrokeQueue.shift();
            const timestamp = this.timestampQueue.shift();
            const inputChar = this.inputCharQueue.shift();

            if (keystroke === "Backspace") {
                assembledString = assembledString.slice(0, assembledString.length - 1);
            } else {
                assembledString += inputChar;
            }

            console.log(`[Keystrokes]  keytroke=${keystroke}, len=${keystroke.length}`);
            console.log(`[Keystrokes] timestamp=${timestamp}`);
            console.log(`[Keystrokes] inputChar=${inputChar}`);
            console.log(`[Keystrokes] assembled=${assembledString}`);

            const equalSoFar = expectedString.startsWith(assembledString);
            const equality = expectedString === assembledString;

            if (keystroke != "Backspace") {
                gatheredInputs.push({
                    timestampMillis: timestamp,
                    wasCorrect: equalSoFar,
                    key: inputChar
                });
            }

            if (equalSoFar) {
                inputTextarea.setAttribute("data-input-state", "progress");
            } else {
                inputTextarea.setAttribute("data-input-state", "error");
            }

            if (equality) {
                console.log("[Keystrokes] Field has been completed, moving on...");
                console.log(`[Keystrokes]     Last key that was pressed: ${keystroke.key}`);
                console.log(`[Keystrokes]     Current equality value: ${equality}`);
                inputTextarea.setAttribute("disabled", "");
                inputTextarea.setAttribute("data-input-state", "completed");
                field.removeEventListener("keydown", keydownListener);
                field.removeEventListener("input", inputListener)

                sendEventsToServer(gatheredInputs, "keystroke");
                state.currentField++;
            }
        }
    }

    const keydownListener = e => {
        const timestampMillis = Date.now();
        // we want to explicitly disable keyboard navigation keys
        if (navigationKeys.includes(e.key)) {
            e.preventDefault();
            e.stopPropagation();
        }
        // some keydown events do not have a corresponding input event
        // this is usually control keys
        // many of these can be filtered as follows (as per https://www.w3.org/TR/uievents-key/#keys-unicode):
        if (e.key != "Backspace" && e.key.length >= 2) return;

        sink.keystrokeQueue.push(e.key);
        sink.timestampQueue.push(timestampMillis);
    };

    const inputListener = e => {
        console.log(e);
        sink.inputCharQueue.push(e.data);
        sink.notify();
    }

    field.addEventListener("keydown", keydownListener);
    field.addEventListener("input", inputListener);
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
