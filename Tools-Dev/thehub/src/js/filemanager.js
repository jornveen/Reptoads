const { ipcRenderer, remote } = require('electron');
const fs = require('fs');

class FileManager {
    constructor({ editor, monaco }) {
        this.editor = editor;
        this.monaco = monaco;

        // When we receive a 'open-file' message, open the file
        ipcRenderer.on('open-file', (e, url) => this.openFile(url));

    }

    openFile(url) {
        // fs.readFile doesn't know what `file://` means
        const parsedUrl = (url.slice(0, 7) === 'file://') ? url.slice(7) : url;

        this.file = parsedUrl;

        fs.readFile(parsedUrl, 'utf-8', (err, data) => {
            this.editor.setModel(this.monaco.editor.createModel(data, 'javascript'));
        });
    }

    saveFile() {
        if (!this.file) return;

        const model = this.editor.getModel();
        let data = model.getValue();

        fs.writeFile(this.file, data, 'utf-8', (err) => {
            if(err) console.log(err);
        });
    }
}

module.exports = FileManager;