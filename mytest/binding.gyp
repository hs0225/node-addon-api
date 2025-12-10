{
  "targets": [
    {
      "target_name": "hello",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "hello.cc" ],
      "include_dirs": [
        "./"
      ],
      'cflags_cc': [
        '-g'
      ],
      'cflags': [
        '-g'
      ],
      'defines': [  ],
    }
  ]
}
