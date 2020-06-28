4CHAN_DOWNLOADER
================

Get 4chan catalogues/threads in a readable text format or download media with this tool!

Example usage where we get the information of the thread 76542350 from /g/:
`./4CHAN_DOWNLOADER -b g -t 76542350`

Part of the output will be:
``` text
Anonymous  test.png (1250x1250) 06/27/20(Sat)21:29:55 76542951
Media: http://i.4cdn.org/g/1593307795491.png
>>76542350
having fun making mazes in go

Anonymous  06/27/20(Sat)21:30:20 76542957
>>76542932
what did he do this time?

Anonymous  06/27/20(Sat)21:33:18 76542980
>>76542804
Because clojure took its place

Anonymous  06/27/20(Sat)21:34:32 76543000
>>76542980
Is Clojure a Lisp?

Anonymous  06/27/20(Sat)21:37:12 76543028
>>76542350
Has anyone on /g/ even used Dylan? Does it even have a modern compiler?

Anonymous  06/27/20(Sat)21:37:37 76543033
>>76542934
Build your own String tokenizer.

https://docs.oracle.com/javase/7/docs/api/java/util/StringTokenizer.ht
```

The overall usage is:
``` text
Usage: 4CHAN_DOWNLOADER [OPTION...] [FILENAME]...
Software to download 4chan threads.

  -b, --board=value          Board option.
  -c, --catalog=value        Get catalog.
  -f, --file=value           Get file.
  -i, --images=value         Get images.
  -j, --json                 Print json as output.
  -t, --thread=value         Get thread.
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to <romeu.bizz@gmail.com>.
```
