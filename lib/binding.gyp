{
  "targets": [
    {
      "target_name": "urlParser",
      "sources": [ "urlParser.cc" ],
      "conditions": [  
        [ 
          "OS==\"mac\"",
          {
            "xcode_settings": {  
              "OTHER_CPLUSPLUSFLAGS": [  
                "-std=c++11",
                "-stdlib=libc++"
              ],
              "OTHER_LDFLAGS": [  
                "-stdlib=libc++"
              ],
              "MACOSX_DEPLOYMENT_TARGET":"10.10.2"
            }
          }
        ]
      ]
    }
  ]
}