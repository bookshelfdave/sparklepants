var r = require('./build/Release/riak');

var c = new r.Connection("localhost", 10017);

console.log("Test put:");
var resp = c.put("FooBucket", "FooKey", "FooValue");
console.log(resp);


console.log("Test gets:");
console.log(c.get("FooBucket","FooKey"));


console.log("Test delete:");
resp = c.delete("FooBucket", "FooKey");
console.log(resp);


console.log("Test gets:");
console.log(c.get("FooBucket","FooKey"));
console.log("Done!");
