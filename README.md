4CHAN_DOWNLOADER
================

Get 4chan catalogues/threads in a readable text format or download media with this tool!

Example usage where we get the information of the thread 70648879 from /g/:
`./4CHAN_DOWNLOADER -b g -t 70648879`

Part of the output will be:
``` text
Anonymous 04/22/19(Mon)02:15:32 70648896
>>70648879
*SNAP*


Anonymous 04/22/19(Mon)02:20:02 70648927
>>70648879
How do you stop it from losing all your shit?


Anonymous 04/22/19(Mon)02:21:23 70648933
The Apple Macbook Pro with Retina Display doesn't have this problem.


Anonymous 04/22/19(Mon)03:14:05 70649238
>>70648927
Try to recover as much as you can. Make multiple attempts and see if you get more each time. I can't recommend any software but I can definitely say you can save a lot of your data if you act quickly.


Anonymous 04/22/19(Mon)03:15:52 70649252
Should i just buy a 1gb ssd before it corrupts all of my data


Anonymous 04/22/19(Mon)03:21:57 70649293
>>70649252
Only 1GB of valuable data huh.


Anonymous barnesthreatenslittlegirl.jpg (564x310) 04/22/19(Mon)03:35:52 70649387
Media: http://i.4cdn.org/g/1555918552295.jpg
>>70648927
a gun

```

The overall usage is:
``` text
Usage: ./4CHAN_DOWNLOADER [ARGS]
	-b: board
	-c: get catalogue
	-h: print this message
	-i: download media from thread
	-t: thread id
```
