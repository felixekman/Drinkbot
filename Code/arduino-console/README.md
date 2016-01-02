# arduino-console
A simple portable console module for Arduino.<br>
Originally ported from Ousia console. http://ousia.accrete.org/

## Features
* Easy for adding new commands
* Arguments passing through
* Simply extendable

## Running Demo
```
console starts...
Please press ENTER to activate it.

$
$ ?
available commands:
	help
	?
	debug
```
```
$ who
who: command not found
```
```
$ debug param1 param2
argv[1]: param1
argv[2]: param2
argc: 3
cmd_debug
```
## TODO
It would help a lot if anyone can pack it to be an Arduino Library.
