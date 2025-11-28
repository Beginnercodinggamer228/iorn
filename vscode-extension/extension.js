const vscode = require('vscode');
const { exec } = require('child_process');
const path = require('path');

function activate(context) {
    // Команда компиляции
    let compileCommand = vscode.commands.registerCommand('iorn.compile', function (uri) {
        const filePath = uri ? uri.fsPath : vscode.window.activeTextEditor?.document.fileName;
        if (!filePath || !filePath.endsWith('.iorn')) {
            vscode.window.showErrorMessage('Please select a .iorn file');
            return;
        }

        const terminal = vscode.window.createTerminal('IORN Compiler');
        terminal.sendText(`iorn "${filePath}"`);
        terminal.show();
    });

    // Команда запуска
    let runCommand = vscode.commands.registerCommand('iorn.run', function (uri) {
        const filePath = uri ? uri.fsPath : vscode.window.activeTextEditor?.document.fileName;
        if (!filePath || !filePath.endsWith('.iorn')) {
            vscode.window.showErrorMessage('Please select a .iorn file');
            return;
        }

        const terminal = vscode.window.createTerminal('IORN Runner');
        terminal.sendText(`iorn "${filePath}"`);
        terminal.show();
    });

    context.subscriptions.push(compileCommand, runCommand);
}

function deactivate() {}

module.exports = {
    activate,
    deactivate
};