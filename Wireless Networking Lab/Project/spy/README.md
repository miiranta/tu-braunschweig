# Spy Node

This code can be loaded into an IBR-Node to listen all messages exchanged using
the MSP protocol. Besides that, it also provides a Python script to filter the
messages and exhibit the position of each car in the virtual grid they use to
locate themselves.

## How to use
For the main.c file, it is like every other IBR-Node application. You can boot
it into the board and execute it.

### run.sh

For ease of use, we created a Bash script that will locate the connected boards
in the PC and load and execute the code into them.

There are three ways to use it:

1. ./run.sh, if there is only one board connected into the PC
2. ./run.sh {index}, if there are more than one board connected and you do not
need to know exactly which board it will use. It will find the board in your PC
and separate them by this index.
3. ./run.sh --serial {serial}, if you'd like to specify the board by its serial
number.

Note: do not forget to give the adequate permissions to it with 
`chmod +x run.sh`.

### fiter.py

The virtual grid visualization script, written in Python, can be used by using 
pipes in the command line. Simply redirect the output of the main.c code to it
like this:

`./run.sh | python3 filter.py`

or, to redirect the error messages and do not polute the terminal:

`./run.sh 2> /dev/null | python3 filter.py`

