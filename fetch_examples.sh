#!/bin/sh
mkdir -p examples
#     ^^<----- supress error message if directory already exists
curl -s "https://raw.githubusercontent.com/erikdubbelboer/brainfuck-jit/master/mandelbrot.bf" -o examples/mandelbrot.b
curl -s "https://www.linusakesson.net/programming/brainfuck/life.bf"                          -o examples/game_of_life.b
echo "Fetched some examples!"
