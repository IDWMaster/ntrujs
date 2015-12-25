var os = require('os');
var addon = require('./lib/addon-'+os.platform()+'-'+os.arch());

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
