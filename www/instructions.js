// name conflict with loadingField in callbacks.js
const loadingFieldInstructions = document.getElementById("loading");
const proceedButton = document.getElementById("proceed");

const proceedListener = async e => {
    const res = await fetch("/proceed", {
        method: "POST",
        body: "{}"
    });

    if (res.ok) {
        loadingFieldInstructions.removeAttribute("style");
        proceedButton.removeEventListener("click", proceedListener);
        return;
    }

    console.log("An unknown error occured.");
};

proceedButton.addEventListener("click", proceedListener);