4CHAN_DOWNLOADER
================

Get 4chan catalogues/threads in a readable text format or download media with this tool!

Example usage where we get the information of the thread 76542350 from /g/:
`./4CHAN_DOWNLOADER -b g -t 76542350`

Part of the output will be:
```text
Link: https://boards.4channel.org/g/thread/84494522#p84494522
Anonymous url.jpg (966x399) 11/27/21(Sat)12:07:13 no.84494522
Media: http://i.4cdn.org/g/1638032833733.jpg
I installed Linux, what now?

Link: https://boards.4channel.org/g/thread/84494522#p84494630
Anonymous no.84494630
>>84494522
throw your computer out of the window

Link: https://boards.4channel.org/g/thread/84494522#p84494676
Anonymous no.84494676
change wallpaper, post neofetch/desktop, then reinstall windows
```

The overall usage is:
```text
4chan-downloader 1.0
Romeu Vieira <romeu.bizz@gmail.com>
4Chan Downloader

USAGE:
    FOURCHAN_DOWNLOADER [FLAGS] [OPTIONS]

FLAGS:
    -j, --json       Output as JSON
    -h, --help       Prints help information
    -V, --version    Prints version information

OPTIONS:
    -b, --board <BOARD>      4chan board
    -i, --images <DIR>       Get images
    -t, --thread <THREAD>    Thread id
```

