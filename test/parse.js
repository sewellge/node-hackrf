const assert = require('assert')
const parse = require('../bin.js').parse

assert.strictEqual(parse('10Mhz'), 10 * 1e6)
assert.strictEqual(parse('100khz'), 100 * 1e3)
assert.strictEqual(parse(500), 500)

console.log('parse tests passed')
