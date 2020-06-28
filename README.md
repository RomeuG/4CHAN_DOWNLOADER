4CHAN_DOWNLOADER
================

Get 4chan catalogues/threads in a readable text format or download media with this tool!

Example usage where we get the information of the thread 76542350 from /g/:
`./4CHAN_DOWNLOADER -b g -t 76542350`

Part of the output will be:
``` text
Anonymous  pepe-d.png (1298x720) 06/27/20(Sat)20:32:15 76542350
Media: http://i.4cdn.org/g/1593304335611.png
Learn Nim: https://nim-lang.org/learn.html
Learn Fortran: https://www.fortrantutorial.com/
Learn F#: https://docs.microsoft.com/en-us/dotnet/fsharp/
Learn Pascal: http://www.pascal-programming.info/index.php
Learn Java: https://java-programming.mooc.fi/
Learn C++: https://www.learncpp.com/
Learn APL: http://www.microapl.com/apl/tutorial_contents.html
Learn R: https://www.tutorialspoint.com/r/index.htm
Learn Perl: https://perldoc.perl.org/
Learn Dylan: https://opendylan.org/books/dpg/

Previous: >>76531609

Anonymous  06/27/20(Sat)20:34:39 76542375
>>76542350
Lisp is the most powerful programming language.

Anonymous  06/27/20(Sat)20:35:07 76542380
I hate programming but I hate everything else more

Anonymous  06/27/20(Sat)20:35:47 76542385
In 2020, C# is the best all-around programming language.
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
