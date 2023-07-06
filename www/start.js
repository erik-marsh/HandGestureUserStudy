const userIdField = document.getElementById("form-user-id");
const submitButton = document.getElementById("form-submit");
const errorDiv = document.getElementById("error-message-container");

const inputHandler = async e => {
    const input = userIdField.value;
    if (input === "") return;

    const userId = Number(input);
    if (Number.isNaN(userId)) return;

    const res = await fetch("/start", {
        method: "POST",
        body: `{"userId": ${userId}}`
    });

    if (res.ok) return;

    if (res.status === 403) {
        errorDiv.textContent = "The ID that you entered is already in use. Please enter a different ID.";
        return;
    }

    console.log("An unknown error occured.");
};

submitButton.addEventListener("click", inputHandler);
userIdField.addEventListener("keyup", e => {
    if (e.key == "Enter") {
        inputHandler(null);
    }
});
