#define BUILDING_NODE_EXTENSION
#include <node/node.h>
#include <iostream>
#include "Connection.h"
#include <riak.h>

using namespace v8;

Persistent<Function> Connection::constructor;

Connection::Connection(char *hostname, int32_t port) : hostname_(hostname), port_(port) {
  // TODO: lookup stupid string copying stuff for C++
  std::cout << "Riak connection to " << hostname << ":" << port << std::endl;
  riak_error err = riak_config_new_default(&cfg);
  err = riak_connection_new(cfg, &cxn, strdup(hostname), "10017", NULL);
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
  tpl->PrototypeTemplate()->Set(String::NewSymbol("get"),
      FunctionTemplate::New(Get)->GetFunction());
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

Handle<Value> Connection::Get(const Arguments& args) {
  HandleScope scope;

  Connection* obj = ObjectWrap::Unwrap<Connection>(args.This());

  if (args.Length() < 2) {
      ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
      return scope.Close(Undefined());
  }

  if (!args[0]->IsString() || !args[1]->IsString()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments")));
      return scope.Close(Undefined());
  }

  char* in_bucket = strdup(*(v8::String::AsciiValue::AsciiValue(args[0]->ToString())));
  char* in_key    = strdup(*(v8::String::AsciiValue::AsciiValue(args[1]->ToString())));


  riak_binary *bucket_bin   = riak_binary_copy_from_string(obj->cfg, in_bucket);
  riak_binary *key_bin      = riak_binary_copy_from_string(obj->cfg, in_key);

  riak_get_options *get_options = riak_get_options_new(obj->cfg);

  riak_get_response *get_response = NULL;
  // TODO: riak_get() has a memory leak
  riak_error err = riak_get(obj->cxn, bucket_bin, key_bin, get_options, &get_response);

  riak_object* v = riak_get_get_content(get_response)[0];
  riak_binary *vv = riak_object_get_value(v);
  Local<String> vvv = v8::String::New((char*)riak_binary_data(vv), riak_binary_len(vv));

  riak_binary *vc = riak_get_get_vclock(get_response);
  Local<String> vcc = v8::String::New((char*)riak_binary_data(vc), riak_binary_len(vc));

  riak_get_response_free(obj->cfg, &get_response);
  riak_get_options_free(obj->cfg, &get_options);
  free(in_bucket);
  free(in_key);
  riak_binary_free(obj->cfg, &bucket_bin);
  riak_binary_free(obj->cfg, &key_bin);

  Local<Object> result_obj = Object::New();
  result_obj->Set(String::NewSymbol("riak_bucket"), args[0]->ToString());
  result_obj->Set(String::NewSymbol("riak_key"),    args[1]->ToString());
  result_obj->Set(String::NewSymbol("riak_value"), vvv);

  return scope.Close(result_obj);
}

