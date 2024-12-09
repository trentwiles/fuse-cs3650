# Project 2: File System

This is the starter code for [Project 2](https://course.ccs.neu.edu/cs3650sp23/p2.html). The following contents are provided:

- [Makefile](Makefile)   - Targets are explained in the assignment text
- [README.md](README.md) - This README
- [helpers](helpers)     - Helper code implementing access to bitmaps and blocks
- [hints](hints)         - Incomplete bits and pieces that you might want to use as inspiration
- [nufs.c](nufs.c)       - The main file of the file system driver
- [test.pl](test.pl)     - Tests to exercise the file system

## Running the tests

You might need install an additional package to run the provided tests:

```
$ sudo apt-get install libtest-simple-perl
```

Then using `make test` will run the provided tests.



## Codespaces

Helpful note for running on codespaces: update packages, and then install the required packages, as stated on the project website:

```
sudo su
apt update
apt-get install libfuse-dev libbsd-dev pkg-config
```
## Other Notes

Due to some gitignore issues on my homeserver, you may see the log and nufs files here. They should be deleted and ignored.