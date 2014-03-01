var r = require('./build/Release/riak');

var c = new r.Connection("localhost", 10017);
var i = 0;
for(i = 0; i < 100; i++) {
  console.log(c.get("foo","bar"));
}
console.log("Done!");
