4CHAN_DOWNLOADER
================

Get 4chan catalogues/threads in a readable text format or download media with this tool!

Example usage where we get the information of the thread 76542350 from /g/:
`./4CHAN_DOWNLOADER -b g -t 76542350`

Part of the output will be:
``` text
Anonymous  06/27/20(Sat)20:43:56 76542456
Media: http://i.4cdn.org/g/0
>>76542401
>>76542425
this except its my job

Anonymous  06/27/20(Sat)20:44:25 76542458
Media: http://i.4cdn.org/g/0
>>76542425
Surround yourself with parentheses

Anonymous  06/27/20(Sat)20:46:21 76542471
Media: http://i.4cdn.org/g/0
>>76542425
Use D instead.

Anonymous  06/27/20(Sat)20:47:09 76542481
Media: http://i.4cdn.org/g/0
while read -u3 file; do mpv "$file"; (( $? == 4 )) && echo "deleting $file" && rm "$file"; done 3< <(ls | shuf)
Uh, how do I exit this bash loop?

Anonymous  06/27/20(Sat)20:47:21 76542483
Media: http://i.4cdn.org/g/0
>>76542481
Ctrl-D
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
