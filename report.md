I tested all of my old functionality from assignment 3 to make sure that it still works.

I tested piping, making sure that if I went over the max amount (or less than the min amount) of commands 
separated by pipe that I would print out invalid number of pipes and return -1. 
So if I try ls | ls | ls | ls | ls | ls | ls | ls | ls | ls | ls | ls, the shell will tell me invalid number of pipes. 
Another command I tried was find . | grep txt, which listed all contents in a directory with “txt” in their name. 
Another command I tried was ls -a | head -n 5 || sleep 10 which only outputs the first 
5 lines of ls -a (if the shell doesn’t respond for 10 seconds because sleep 10 is happening, 
then your parallel command is not working properly). Luckily my piping worked fine with the parallel implementation of the 
last assignment. If I pipe 3 commands like find . | grep txt | less, my solution also works fine. I tried various commands 
such as these that I found on Stack Overflow. 

Since there was no way of seeing if the child’s priority or policy was set just by running my shell since the priority level 
and scheduler of the parent process remains the same, I originally used print statements to test if the child’s 
priority/policy was properly changed before it exited. If I try set_priority 2, I do not get an error. I do not get an error 
if I set_policy FIFO. The professor also showed me how to use top command and proc file system in linux to see properties of 
the child. 
