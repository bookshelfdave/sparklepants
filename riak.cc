#include <node.h>
#include <v8.h>
#include <stdio.h>
#include "Connection.h"

using namespace v8;

void InitAll(Handle<Object> exports) {
  Connection::Init(exports);
}

NODE_MODULE(riak, InitAll)
