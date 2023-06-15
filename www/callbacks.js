"use strict";

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
// TODO: investigate https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Proxy
// should not be accessed directly
let __state = {
    currentField: 0
};

const __stateHandler = {
    set(obj, prop, value) {
        console.log("Property " + prop + " of " + obj + " updated from " + obj[prop] + " to " + value);

        if (prop === "currentField")
        {
            const completionTime = Date.now();

            // send field completion event
            sendEventsToServer({
                timestampMillis: completionTime,
                fieldIndex: obj[prop],
                //totalFields: totalFields
            });

            // if we have now done all fields
            if (value === totalFields)
            {
                console.log("task is complete");
                sendEventsToServer(clickEvents);
                clickEvents = [];
                sendEventsToServer({
                    timestampMillis: completionTime,
                    taskIndex: "<UNKNOWN>",
                    //taskName: "<UNKNOWN>",
                    //totalTasks: "<UNKNOWN>"
                });
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
            state.currentField++;
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
            "timestampMillis": timestampMillis,
            "location": clickLocation,
            "wasCorrect": wasCorrect
        };
        clickEvents.push(newClick);
        
        console.log(newClick);
        console.log("click HIT on fieldIndex=" + fieldIndex);

        // TODO: send events on TASK completion
        
        e.stopPropagation();
    });
});