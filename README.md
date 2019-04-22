# dumptool

## how to build
```
g++ -std=c++11 main.cpp -o dumptool
```

## how to run
```
./dumptool 'string,char[3],int8_t' dummy.bin
```

## doc
* `,`区切りでformatを記述する
* formatは`:`区切りでnameとtypeを指定する(nameは省略可能)
  * typeは数値を設定することができる
    * `char[4]`
* formatに指定可能なtypeの一覧
* char,int,float,double
* int8_t,uint8_t,int16_t,uint16_t,int32_t,uint32_t,int64_t,uint64_t
* string
* offset: 指定したoffsetに移動する
* skip: 表示を行わずにskipする

## TODO
* write test
* write doc
* 表示出力形式の指定
* サイズ指定に，読み込んだ変数を利用できるように
  * 数値を演算可能とする

## NOTE
### dummy bin file
```
$ echo 'abc\0x00def' > dummy.bin
$ hexdump -C dummy.bin
00000000  61 62 63 00 64 65 66 0a                           |abc.def.|
00000008
```

### hexdump exmaple
```
echo 012345678 | { printf "%8s|%8s|%8s|%11s|%4s\n" "offset" "float32" "int32" "hex" "char"; printf '%*s\n' "43" '-' | tr ' ' '-'; hexdump -e '1/4 "%08_ax|%08f|"' -e '1/4 "%08X|"' -e '4/1 "%02X "' -e '1/4 "|"' -e '4/1 "%_p"' -e '"\n"'; }
```

## FYI
* [Google C\+\+ スタイルガイド\(日本語全訳\) 列挙型の名前]( https://ttsuki.github.io/styleguide/cppguide.ja.html#Enumerator_Names )
* [Can a C\+\+ enum class have methods? \- Stack Overflow]( https://stackoverflow.com/questions/21295935/can-a-c-enum-class-have-methods )
