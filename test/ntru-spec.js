var assert = require('assert');
var NTRU = require('../')

describe('ntru test', function () {
  it('should generate keypair', function () {
    const keyPair = NTRU.createKey()
    const plaintext = new Uint8Array([104, 101, 108, 108, 111, 0]) // "hello"
    const encrypted = NTRU.encrypt(plaintext, keyPair.public)
    const decrypted = NTRU.decrypt(encrypted, keyPair.private) // same as plaintext

    assert.deepEqual(plaintext, decrypted)
  })
})
