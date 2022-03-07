## Intro

This is my take on writing a wordle solver using mini-max search. I was
somewhat familiar with how chess engines work and I decided to apply a similar
algorithm for wordle, which may sound like it is a completely kind of game.

A writeup about the approach can be found at: https://medium.com/p/31601a5dbb42

If you want to try the solver, it is hosted at: https://bekyblog.tech/wordle-solver/

It is caled botle, because it is a BOT that solves wordLE.

## Some technical details

The engine was written in C++, the GUI was written in reacjs. There is a cache
of precomputed solutions for the first word of each configuration. From the
second word and on, the best word is computed locally using the C++ engine
compiled to wasm.

You can also run the engine from the command line, however, it is certainly
easier to use the GUI.
