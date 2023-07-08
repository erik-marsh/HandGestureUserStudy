setTimeout(() => {
    fetch("/quit", {
        method: "POST",
        body: "{}"
    });
}, 3000);
