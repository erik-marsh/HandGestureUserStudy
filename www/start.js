const userIdField = document.getElementById("form-user-id");
const submitButton = document.getElementById("form-submit");

submitButton.addEventListener("click", e => {
    const input = userIdField.value;
    if (input === "") return;

    const userId = Number(input);
    if (Number.isNaN(userId)) return;

    fetch("/start", {
        method: "POST",
        body: `{"userId": ${userId}}`
    });
});