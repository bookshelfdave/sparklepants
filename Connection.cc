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
  // TODO
}

void Connection::Init(Handle<Object> exports) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Connection"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("get"),
      FunctionTemplate::New(Get)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("put"),
      FunctionTemplate::New(Put)->GetFunction());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("delete"),
      FunctionTemplate::New(Delete)->GetFunction());
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

void debug(const char* msg) {
  std::cout << "DEBUG:" << msg << std::endl;
}


Local<Object> makeGetResponse(Local<String> bucket, Local<String> key, riak_get_response *resp) {
  riak_int32_t objcount = riak_get_get_n_content(resp);
  Local<Object> result_obj = Object::New();
  Local<Number> n_content = Number::New(riak_get_get_n_content(resp));
  Handle<Boolean> deleted  = Boolean::New(riak_get_get_deleted(resp));
  if(riak_get_get_has_vclock(resp)) {
    riak_binary *vcb = riak_get_get_vclock(resp);
    Local<String> vclock = v8::String::New((char*)riak_binary_data(vcb), riak_binary_len(vcb));
    result_obj->Set(String::NewSymbol("vclock"), vclock);
  }

  result_obj->Set(String::NewSymbol("riak_bucket"), bucket);
  result_obj->Set(String::NewSymbol("riak_key"), key);
  result_obj->Set(String::NewSymbol("n_content"), n_content);
  result_obj->Set(String::NewSymbol("deleted"), deleted);

  Local<Array> array = Array::New(objcount);
  int index = 0;
  for(index = 0; index < objcount; index++) {
     riak_object* v = riak_get_get_content(resp)[index];
     riak_binary *vv = riak_object_get_value(v);
     Local<String> vvv = v8::String::New((char*)riak_binary_data(vv), riak_binary_len(vv));
     array->Set(index, vvv);
  }
  result_obj->Set(String::NewSymbol("content"), array);
  return result_obj;
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
  if(err) {
    std::cout << "GET ERROR" << std::endl;
  }

  Local<Object> response_obj = makeGetResponse(args[0]->ToString(),
                                               args[1]->ToString(),
                                               get_response);

  riak_get_response_free(obj->cfg, &get_response);
  riak_get_options_free(obj->cfg, &get_options);
  free(in_bucket);
  free(in_key);
  riak_binary_free(obj->cfg, &bucket_bin);
  riak_binary_free(obj->cfg, &key_bin);

  return scope.Close(response_obj);
}


Handle<Value> Connection::Put(const Arguments& args) {
  HandleScope scope;

  Connection* obj = ObjectWrap::Unwrap<Connection>(args.This());

  if (args.Length() < 3) {
      ThrowException(Exception::TypeError(String::New("Wrong number of arguments")));
      return scope.Close(Undefined());
  }

  if (!args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
      ThrowException(Exception::TypeError(String::New("Wrong arguments")));
      return scope.Close(Undefined());
  }

  char* in_bucket = strdup(*(v8::String::AsciiValue::AsciiValue(args[0]->ToString())));
  char* in_key    = strdup(*(v8::String::AsciiValue::AsciiValue(args[1]->ToString())));
  char* in_value  = strdup(*(v8::String::AsciiValue::AsciiValue(args[2]->ToString())));


  riak_binary *bucket_bin   = riak_binary_copy_from_string(obj->cfg, in_bucket);
  riak_binary *key_bin      = riak_binary_copy_from_string(obj->cfg, in_key);
  riak_binary *value_bin    = riak_binary_copy_from_string(obj->cfg, in_value);
  riak_object *riak_obj = riak_object_new(obj->cfg);

  riak_object_set_bucket(obj->cfg, riak_obj, bucket_bin);
  riak_object_set_key(obj->cfg, riak_obj, key_bin);
  riak_object_set_value(obj->cfg, riak_obj, value_bin);

  riak_put_options *put_options = riak_put_options_new(obj->cfg);
  riak_put_options_set_return_body(put_options, RIAK_TRUE);

  riak_put_response *put_response = NULL;
  // TODO: riak_get() has a memory leak
  riak_error err = riak_put(obj->cxn, riak_obj, put_options, &put_response);
  if(err) {
    std::cout << "PUT ERROR" << std::endl;
  }

  riak_object* v = riak_put_get_content(put_response)[0];
  riak_binary *vv = riak_object_get_value(v);
  Local<String> vvv = v8::String::New((char*)riak_binary_data(vv), riak_binary_len(vv));


  riak_put_response_free(obj->cfg, &put_response);
  riak_put_options_free(obj->cfg, &put_options);
  free(in_bucket);
  free(in_key);
  free(in_value);
  riak_binary_free(obj->cfg, &bucket_bin);
  riak_binary_free(obj->cfg, &key_bin);
  riak_binary_free(obj->cfg, &value_bin);

  Local<Object> result_obj = Object::New();
  result_obj->Set(String::NewSymbol("riak_bucket"), args[0]->ToString());
  result_obj->Set(String::NewSymbol("riak_key"),    args[1]->ToString());
  //result_obj->Set(String::NewSymbol("riak_value"),    args[2]->ToString());
  result_obj->Set(String::NewSymbol("riak_value"), vvv);

  return scope.Close(result_obj);
}


Handle<Value> Connection::Delete(const Arguments& args) {
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

  riak_delete_options *delete_options = riak_delete_options_new(obj->cfg);


  // TODO: riak_get() has a memory leak
  riak_error err = riak_delete(obj->cxn, bucket_bin, key_bin, delete_options);
  if(err) {
    std::cout << "DELETE ERROR" << std::endl;
  }

  riak_delete_options_free(obj->cfg, &delete_options);
  free(in_bucket);
  free(in_key);
  riak_binary_free(obj->cfg, &bucket_bin);
  riak_binary_free(obj->cfg, &key_bin);

  Local<Object> result_obj = Object::New();
  return scope.Close(result_obj);
}

