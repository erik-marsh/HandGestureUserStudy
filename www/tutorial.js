const proceedButton = document.getElementById("proceed");
const loadingField = document.getElementById("loading");

proceedButton.addEventListener("click", async e => {
    const res = await fetch("/acknowledgeTutorial", {
        method: "POST",
        body: "{}"
    });

    if (res.ok) {
        loadingField.innerHTML =
        `<div class="col-2 justify-content-center">
            <div class="spinner-border"></div>
        </div>
        <div class="col">
            <p>If the study has not yet loaded, please refresh the page by pressing F5.</p>
        </div>`;
        return;
    }

    console.log("An unknown error occured.");
});