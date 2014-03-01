#ifndef CONNECTION_H
#define CONNECTION_H

#include <node/node.h>
#include <riak.h>

class Connection : public node::ObjectWrap {
 public:
  static void Init(v8::Handle<v8::Object> exports);

 private:
  explicit Connection(char* hostname, int32_t port);
  ~Connection();

  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  //static v8::Handle<v8::Value> PlusOne(const v8::Arguments& args);
  static v8::Persistent<v8::Function> constructor;

  riak_config *cfg;
  riak_connection *cxn;
  char* hostname_;
  int32_t port_;
};

#endif
