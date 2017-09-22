
First get the root access for make command using 
			sudo bash

To Compile use command:
			make

To Insert module:
			insmod linepipe.ko buffer_size=N

To Run use command:
			./consumer /dev/linepipe
			./producer /dev/linepipe

To Remove module:
			rmmod linepipe

TASK B
We will see concurrency(synchronization), deadlock, race condition issues when we run the script by giving conditions. As it is read or written byte by byte so we will get error when multiple producer and consumer try to read or write they will not get correct output.
