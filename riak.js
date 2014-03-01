var r = require('./build/Release/riak');

var c = new r.Connection("localhost", 10017);
console.log(c);
