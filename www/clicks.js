"use strict";

// TODO: at this point we need to introduce some sort of global state
// probably needs at least a current task index and the current field index
// (and because of that, all the scripts need to be merged)

let clickEvents = []

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
        e.stopPropagation();

        const timestampMillis = Date.now();
        const fieldIndex = Number.parseInt(field.getAttribute("data-field-index"));
        const currentField = 2;  // TODO: dummy value, in reality this is in a global object somewhere
        
        let clickLocation = "";
        if (field.classList.contains("text-input-field"))
            clickLocation = "TextField";
        else if (field.classList.contains("user-study-button"))
            clickLocation = "Button";
        else
            clickLocation = "OutOfBounds";
        
        clickEvents.push({
            "timestampMillis": timestampMillis,
            "location": clickLocation,
            "wasCorrect": fieldIndex === currentField
        });
        
        console.log("click HIT on fieldIndex=" + fieldIndex);
        if (fieldIndex === currentField)
            console.log("... which was correct");
        else
            console.log("... which was incorrect");
    });
});