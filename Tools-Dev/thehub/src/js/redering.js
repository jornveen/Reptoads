const remote = require('electron').remote;
/// controlls:
module.exports.handleWindowControls = function()  {
    // When document has loaded, initialise
        let window = remote.getCurrentWindow();
        const minButton = document.getElementById('min-btn'),
        maxButton = document.getElementById('max-btn'),
        closeButton = document.getElementById('close-btn'),
        restoreButton = document.getElementById('restore-btn');
        minButton.addEventListener("click", event => {
            window = remote.getCurrentWindow();
            window.minimize();
        });
        maxButton.addEventListener("click", event => {
            window = remote.getCurrentWindow();
            window.maximize();
            toggleMaxRestoreButtons();
        });

        restoreButton.addEventListener("click", event => {
            window = remote.getCurrentWindow();
            window.unmaximize();
            toggleMaxRestoreButtons();
        });
        closeButton.addEventListener("click", event => {
            window = remote.getCurrentWindow();
            window.close();
        });
        toggleMaxRestoreButtons();
        window.on('maximize', toggleMaxRestoreButtons);
        window.on('unmaximize', toggleMaxRestoreButtons);
        function toggleMaxRestoreButtons() {
            window = remote.getCurrentWindow();
            if (window.isMaximized()) {
                maxButton.style.display = "none";
                restoreButton.style.display = "flex";
            } else {
                restoreButton.style.display = "none";
                maxButton.style.display = "flex";
            }
        }
};