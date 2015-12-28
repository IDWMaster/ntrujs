var os = require('os');
var oldpath = process.env.PWD;
process.chdir(module.filename.replace('main.js',''));
var addon = require('./lib/addon-'+os.platform()+'-'+os.arch());
process.chdir(oldpath);


module.exports = {
    createKey: function () {
        return addon.genKey();
    },
    encrypt: function (data, publicKey) {
        return addon.encrypt(data, publicKey);
    },
    decrypt: function (data, privateKey) {
        return addon.decrypt(data, privateKey);
    }
};
