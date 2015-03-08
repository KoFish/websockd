# Disclaimer

This project is by no means "finished" or really usable at all. In it's current
form it can only spawn workers, accept socket connections and handle reading and
writing to each connection.

# Intentions

The intentions of this project is to create a small reliable server that can
accept client connections. For each connection it should then upgrade it into a
WebSocketa and start a new child process. The websocket input and output will
then be directly connected to the new process `stdin` and `stdout`.

# Credits

This project is heavily inspired by websocketd.com[1] by Joe Walnes but shares,
of obvious reasons, no code with that project.

[1](https://github.com/joewalnes/websocketd)

# Alternative solutions

+ [websocketd](http://websocketd.com/) - The project that inspired this.
+ [xinetd](http://linux.die.net/man/8/xinetd) - This seems to be the most direct
  UNIX-way of doing something like this.
