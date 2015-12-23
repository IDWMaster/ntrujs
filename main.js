var addon = require('./build/Release/addon');

module.exports = {
createKey:function() {
return addon.genKey();
},
encrypt:function(data,publicKey) {
return addon.encrypt(data,publicKey);
},
decrypt:function(data,privateKey) {
return addon.decrypt(data,privateKey);
}
};
