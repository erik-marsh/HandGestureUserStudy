"use strict";

const textFields = document.getElementsByClassName("user-study-field-text");
Array.from(textFields).forEach(field => {
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

            fetch("/events", {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify(dataToSend)
            })
        }
    };
    field.addEventListener("keyup", listener);
});
