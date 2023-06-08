"use strict";

// TODO: at this point we need to introduce some sort of global state
// probably needs at least a current task index and the current field index
// (and because of that, all the scripts need to be merged)

let clickEvents = []

// TODO: investigate https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Proxy
let currentField = 0;
let numFields = 7;

const userStudyFields = document.getElementsByClassName("user-study-field");

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

        if (currentField == numFields)
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