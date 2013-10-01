USB RLY08
=========

Simple interface to USBRLY08 relay module written in C.


## Using the binary ##

Turn on relays 5 and 8
```
jabi@host:~/rly08$ ./rly08 on 5,8
```

Check state for relay #5
```
jabi@host:~/rly08$ ./rly08 get 5
1
jabi@host:~/rly08$ echo $?
1
```

Check relay firmware version
```
jabi@host:~/rly08$ ./rly08 version
Module ID: 8
Soft version: 1
```

Mass set states for every relay (on from 1-4, off from 5-8)
```
jabi@host:~/rly08$ ./rly08 set 11110000
```


Mass-get state for every relay
```
jabi@host:~/rly08$ ./rly08 get 0
11110000
```


