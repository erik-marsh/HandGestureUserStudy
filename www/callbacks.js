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

    const makeKeystrokeEvent2 = (timestampMillis, inputEvent, wasCorrect) => ({
        timestampMillis: timestampMillis,
        wasCorrect: wasCorrect,
        key: inputEvent.data
    });

    let gatheredInputs = [];

    // const listener = e => {
    //     console.log(`[Keystrokes] key: ${e.key}`);
    //     // the problem is right here:
    //     // if you press the next key fast enough, 
    //     // the document updates and inputTextarea.value updates faster than this callback can start
    //     const currInputText = inputTextarea.value.repeat(1);
    //     const timestampMillis = Date.now();
    //     const equalSoFar = expectedString.startsWith(currInputText);
    //     const equality = currInputText === expectedString;

    //     gatheredInputs.push(makeKeystrokeEvent(timestampMillis, e, equalSoFar));

    //     if (equalSoFar) {
    //         inputTextarea.setAttribute("data-input-state", "progress");
    //     } else {
    //         inputTextarea.setAttribute("data-input-state", "error");
    //     }

    //     if (equality) {
    //         console.log("[Keystrokes] Field has been completed, moving on...");
    //         console.log(`[Keystrokes]     Last key that was pressed: ${e.key}`);
    //         console.log(`[Keystrokes]     Current equality value: ${equality}`);
    //         inputTextarea.setAttribute("disabled", "");
    //         inputTextarea.setAttribute("data-input-state", "completed");
    //         field.removeEventListener("keyup", listener);

    //         sendEventsToServer(gatheredInputs, "keystroke");
    //         state.currentField++;
    //     }
    // };
    // field.addEventListener("keyup", listener);

    let assembledString = "";
    //let lastEqualSoFar = true;
    // TODO: the visuals still don't work...
    // maybe tying that to keystroke events will be a better idea?
    const listener2 = e => {
        if (e.data == null) {
            // check for equality anyway: this helps with the visual updates
            const test = assembledString;
            const equalSoFar = expectedString.startsWith(test);

            if (equalSoFar) {
                inputTextarea.setAttribute("data-input-state", "progress");
            } else {
                inputTextarea.setAttribute("data-input-state", "error");
            }

            return;  // important: sending a null to the server crashes the program
        }

        const timestampMillis = Date.now();
        console.log(`[Keystrokes] key: [${typeof (e.data)}]${e.data}`);

        // assemble string
        const test = assembledString + e.data;
        const equalSoFar = expectedString.startsWith(test);
        const equality = test === expectedString;

        if (equalSoFar) {
            assembledString = test;
        }
        // if (lastEqualSoFar) {
        //     assembledString += e.data;
        // } else {
        //     assembledString = assembledString.slice(0, assembledString.length() - 2);
        //     assembledString += e.data;
        // }
        console.log(`[Keystrokes]: assembled: ${assembledString}`);

        //const equalSoFar = expectedString.startsWith(assembledString);
        //lastEqualSoFar = equalSoFar;

        gatheredInputs.push(makeKeystrokeEvent2(timestampMillis, e, equalSoFar));
        console.log(`[Keystrokes] Gathered Inputs: ${gatheredInputs}`);

        if (equalSoFar) {
            inputTextarea.setAttribute("data-input-state", "progress");
        } else {
            inputTextarea.setAttribute("data-input-state", "error");
        }

        if (equality) {
            console.log("[Keystrokes] Field has been completed, moving on...");
            console.log(`[Keystrokes]     Last key that was pressed: ${e.data}`);
            console.log(`[Keystrokes]     Current equality value: ${equality}`);

            inputTextarea.setAttribute("disabled", "");
            inputTextarea.setAttribute("data-input-state", "completed");
            field.removeEventListener("keyup", listener2);

            sendEventsToServer(gatheredInputs, "keystroke");
            state.currentField++;
        }
    };
    field.addEventListener("input", listener2);
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
