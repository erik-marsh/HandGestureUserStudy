const proceedButton = document.getElementById("proceed");
const loadingField = document.getElementById("loading");

const proceedListener = async e => {
    const res = await fetch("/proceed", {
        method: "POST",
        body: "{}"
    });

    if (res.ok) {
        loadingField.removeAttribute("style");
        proceedButton.removeEventListener("click", proceedListener);
        return;
    }

    console.log("An unknown error occured.");
};

proceedButton.addEventListener("click", proceedListener);