var r = require('./build/Release/riak');

var c = new r.Connection("localhost", 10017);
console.log(c.get("foo","bar"));
console.log(c.get("Foo","Bar"));
