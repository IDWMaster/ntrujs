var os = require('os');
var oldpath = process.cwd();
process.chdir(module.filename.replace('main.js',''));
var addon = require('./lib/addon-'+os.platform()+'-'+os.arch());
process.chdir(oldpath);


module.exports = {
    createKey: function () {
        return addon.genKey();
    },
    encrypt: function (data, publicKey) {
        var abi = addon.encrypt(data,publicKey);
	return abi.buffer.slice(0,abi.length);
    },
    decrypt: function (data, privateKey) {
        var abi = addon.decrypt(data, privateKey);
        return abi.buffer.slice(0,abi.length);
    }
};
