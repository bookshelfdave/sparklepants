{
  "targets": [
    {
      "target_name": "riak",
      "sources": [ "riak.cc", "Connection.cc" ],
      "cflags_cc" : ["-I/usr/local/include",
                    "-I/usr/local/Cellar/protobuf-c/0.15/include",
                    "-D_THREAD_SAFE",
                    "-I/usr/local/Cellar/protobuf/2.5.0/include"
                    ],
      "libraries": [
          "-L/usr/local/lib",
          "-L/usr/local/Cellar/protobuf-c/0.15/lib",
          " -lriak_c_client-0.5",
          "-lprotobuf-c",
          "-L/usr/local/Cellar/protobuf/2.5.0/lib",
          "-lprotobuf",
          "-D_THREAD_SAFE"
      ]
    },
  ]
}
