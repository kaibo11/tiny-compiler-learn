const fs = require('fs');
const path = require('path');

const wasmFilePath = path.resolve(__dirname, 'fibonacciSequence.wasm');
const wasmBuffer = fs.readFileSync(wasmFilePath);

const importObject = {
    console: {
        log(arg) {
            console.log(arg);
        },
    },
}


WebAssembly.instantiate(wasmBuffer, importObject).then((wasmModule) => {
    wasmModule.instance.exports.writeFibonacciSequence(9);
});
