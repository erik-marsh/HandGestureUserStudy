"use strict";

const textFields = document.getElementsByClassName('textInputField');
Array.from(textFields).forEach(field => {
    console.log(field);
    const fieldIndex = field.getAttribute("data-field-index");
    const inputTextarea = field.getElementsByClassName("input")[0];
    const expectedTextarea = field.getElementsByClassName("expected")[0];
    console.log(inputTextarea);
    console.log(expectedTextarea);
    field.addEventListener('keyup', e => {
        // the way the text field is set up in HTML may yield
        // leading and trailing whitespace, so we trim those out.
        const equality = inputTextarea.value === expectedTextarea.value.trim();
        console.log("Does string match? " + equality);
    });
});
