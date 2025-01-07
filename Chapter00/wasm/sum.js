const fs = require('fs');
const path = require('path');

// 读取 WebAssembly 文件
const wasmFilePath = path.resolve(__dirname, 'sum.wasm');
const wasmBuffer = fs.readFileSync(wasmFilePath);

WebAssembly.instantiate(wasmBuffer).then((wasmModule) => {
    const { sum } = wasmModule.instance.exports;

    const result = sum(5);
    console.log(`Result of sum(5): ${result}`)
});