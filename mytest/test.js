const addon = require('./build/Release/hello.node');
const clearBytes = new Uint8Array(30);
console.log(addon.encrypt('test encrypto', clearBytes));

try {
  console.log(addon.encrypt());
} catch (e) {
  console.error(e.message);
}

try {
  console.log(addon.encrypt(12, clearBytes));
} catch (e) {
  console.error(e.message);
}

try {
  console.log(addon.encrypt('test', []));
} catch (e) {
  console.error(e.message);
}

console.log('reached');
console.log(addon.encrypt());
console.log('not reached');
