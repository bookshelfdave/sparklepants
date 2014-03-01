#define BUILDING_NODE_EXTENSION
#include <node/node.h>
#include <iostream>
#include "Connection.h"
#include <riak.h>

using namespace v8;

Persistent<Function> Connection::constructor;

Connection::Connection(char *hostname, int32_t port) : hostname_(hostname), port_(port) {
  std::cout << "Riak connection to " << hostname << ":" << port << std::endl;
  riak_error err = riak_config_new_default(&cfg);
  std::cout << "Error code = " << err << std::endl;
  std::cout << "Hostname =[" << hostname_ << "]" << std::endl;
  // UM, passing in hostname_ to riak_connection_new returns ERIAK_DNS_RESOLUTION
  err = riak_connection_new(cfg, &cxn, "localhost", "10017", NULL);
  std::cout << "Error code = " << err << std::endl;
  std::cout << "Connected to Riak" << std::endl;
}

Connection::~Connection() {
}

void Connection::Init(Handle<Object> exports) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Connection"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  //tpl->PrototypeTemplate()->Set(String::NewSymbol("plusOne"),
  //    FunctionTemplate::New(PlusOne)->GetFunction());
  constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("Connection"), constructor);
}

Handle<Value> Connection::New(const Arguments& args) {
  HandleScope scope;
  if (args.IsConstructCall()) {
    // Invoked as constructor: `new Connection(...)`

    char* hostname = *(v8::String::AsciiValue::AsciiValue(args[0]->ToString()));
    int32_t portnum = args[1]->IsUndefined() ? 0 : args[1]->Int32Value();
    Connection* obj = new Connection(hostname, portnum);
    obj->Wrap(args.This());
    return args.This();
  } else {
    // Invoked as plain function `MyObject(...)`, turn into construct call.
    const int argc = 1;
    Local<Value> argv[argc] = { args[0] };
    return scope.Close(constructor->NewInstance(argc, argv));
  }
}

