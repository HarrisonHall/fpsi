# Development

## Bug fixing
Many bugs in this program have been race conditions solved by adding appropriate
locks on data structures.

### GDB
`make debug` and `gdb fpsi` is a good place to start.

### Valgrind
`valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=valgrind-out.txt ./fpsi`
should show if there are any memory issues after a quick run.
