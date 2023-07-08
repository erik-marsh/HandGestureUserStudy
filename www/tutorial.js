const proceedButton = document.getElementById("proceed");
const loadingField = document.getElementById("loading");

proceedButton.addEventListener("click", async e => {
    const res = await fetch("/acknowledgeTutorial", {
        method: "POST",
        body: "{}"
    });

    if (res.ok) {
        loadingField.removeAttribute("style");
        return;
    }

    console.log("An unknown error occured.");
});