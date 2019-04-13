# dumptool

## how to build
```
g++ -std=c++11 main.cpp -o dumptool
```

## how to run
```
./dumptool 'string,char[3],int8_t' dummy.bin
```

## NOTE
### dummy bin file
```
$ echo 'abc\0x00def' > dummy.bin
$ hexdump -C dummy.bin
00000000  61 62 63 00 64 65 66 0a                           |abc.def.|
00000008
```

## FYI
* [Google C\+\+ スタイルガイド\(日本語全訳\) 列挙型の名前]( https://ttsuki.github.io/styleguide/cppguide.ja.html#Enumerator_Names )
* [Can a C\+\+ enum class have methods? \- Stack Overflow]( https://stackoverflow.com/questions/21295935/can-a-c-enum-class-have-methods )
