# cmpt300_A2
#
# This README provides information about the "s-talk" assignment for the CMPT 300 course at Simon Fraser University, aiming to create a simple chat facility for communication between two users over a network using UDP and pthreads. The assignment teaches UNIX UDP IPC, concurrent programming with pthreads, client/server model development, and solving critical section problems. Users must agree on machine names and port numbers, then start s-talk with `s-talk [my port number] [remote machine name] [remote port number]`. The program involves four threads: keyboard input, UDP output, UDP input, and screen output, all sharing a message list controlled with mutexes and condition variables. The provided makefile compiles the program (`make`) and cleans up (`make clean`). To start s-talk, each user runs the command with their respective values; the session ends when a user inputs '!'. Dependencies include Linux CSIL lab computers and the `gcc` compiler. 

#Resources include man pages and online tutorials on socket programming and pthreads. 
#Socket programming: http://beej.us/guide/bgnet/
#Pthreads documentation: https://hpc-tutorials.llnl.gov/posix/
