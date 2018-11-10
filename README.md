**NOTE**: The CASC ReadMe file is in `DOC/Readme`.

## Short Installation Instructions for the impatient

This assumes that you have GNU tar, sh and gawk in your search path!

Simple installation of the first-order version (will install
executables):

```sh
tar -xzf E.tgz
cd E
./configure --bindir=/path/to/EXECDIR
make
make install
/path/to/EXECDIR/eprover -h | more
```

Simplest installation (in-place):

```sh
tar -xzf E.tgz
cd E
./configure
make
cd PROVER
./eprover -h | more
```

Read the rest of this file and the fine (if incomplete) manual if
anything fails. There should be a copy of the manual in
`DOC/eprover.pdf`.

To build the higher-order version, user

```sh
./configure --enable-ho
make
cd PROVER
eprover-ho -h
```


The recommended command for running E on the file problem.p is

```
eprover --auto --proof-object problem.p
```

If you want to try the usually stronger strategy scheduling mode, use

```
eprover --auto-schedule --tptp3-in --proof-object problem.p
```

Replace `eprover` by `eprover-ho` for the higher-order-enabled
version.
You can add a time limit of 300 seconds with the option
`--cpu-limit=300` and a memory limit of 2 GB with `--memory-limit=2048`
for all "automatic" modes. You can reduce output with `-s` (or
`--silent`). As of version 1.9.1, E will try to auto-detect the input
format and adjust the output format accordingly. You can still force
input and output formats via commandline options.



# The Equational Theorem Prover E


This is the README file for version 2.3 "Gielle" of the E equational
theorem prover. This version of E is free software, see the file
COPYING for details about the license and the fact that THERE IS NO
WARRANTY!

This version of E is pre-released on an experimental basis. It
supports a fragment of higher-order logic called lambda-free
higher-order logic (LFHOL).  Any issues you encounter with LFHOL
support should be reported by email to <petar.vukmirovic2@gmail.com>.


## What is E?

E is an equational theorem prover. That means it is a program that you
can stuff a mathematical specification (in many-sorted first-order
logic with equality) and a hypothesisconjecture into, and which will
then run forever, using up all of your machines resources. Very
occasionally it will find a proof for the conjecture and tell you so
;-).

E has been created and is currently maintained by Stephan Schulz,
<schulz@eprover.org>, now with the help of several contributors (see
`DOC/CONTRIBUTORS`). It is developed and distributed under the GNU
General Public License.

The E homepage can be found at <http://www.eprover.org>


## Installation:

E can be installed anywhere in the file system, either by a normal
user or by the system administrator. By default, the prover will still
be compiled as a version that supports first-order logic only.

To install the package, unpack the distribution (if you are reading
this, you probably already did):

```sh
gunzip -c E.tgz|tar -xvf -
```

or

```sh
(g)tar -xzf E.tgz
```
(if you have GNU tar)

This should create a directory named E. After unpacking, optionally
edit `E/Makefile.vars` to your liking. In particular, if building for
HPUX, comment out the suitable CFLAGS definition (for most systems,
the default definition should be ok). Then change to the E directory:

```sh
cd E
```

Determine if you want to run E from its own build directory or wether
you want to install the executables in some other directory
EXECDIR. In the first case, run

```sh
./configure
```

otherwise

```sh
./configure --bindir=EXECDIR
```

or, if you also want to install the man-pages into MANDIR,

```sh
./configure --bindir=EXECDIR --man-prefix=MANDIR
```

To enable higher-order-support, add the option `--enable-ho`, e,g,

```sh
./configure --enable-ho
```

Then type

```sh
make
```

to compile the libraries and all included programs under the E
directory. If you want to install E in a particular EXECDIR, type

```sh
make install
```

You must have write permission in the EXECDIR, so if you install E
outside your own home directory, you may need to become root or use
sudo.

Type

```sh
make documentation
```

to translate the LaTeX documentation (this requires LaTeX2e, pdflatex,
and the packages theorem, amssymb and epsfig, which are included in
most current LaTeX distributions). The manual should also be included
as a pre-compiled PDF.

For some operating systems, especially if you do not have the GNU gcc
compiler installed, you may need to edit Makefile.vars manually to
select tools and options. If you have any problems, look into
E/DOC/PORTING.

If you get into trouble,

```sh
make rebuild
```

will rebuild E completely to your current configuration.

After installation, go to E/PROVER and type

```sh
./eprover BOO001-1+rm_eq_rstfp.lop
```

to see the prover in action. Type

```sh
./eprover LUSK6.lop
```

for a harder example. `./eprover -h` will give you some information and
a list of options.

For impatient people who do not want to read anything:

```sh
eprover --auto --memory-limit=<80%_of_your_main_memory> <problem-file>
```

should give a reasonable performance on a large class of problems
(unless your main memory is really small).

The auto mode will perform a heuristic pruning of the axiom set which
may result in incompleteness for very large problems (many thousands
of axioms). If you need completeness, use

```sh
eprover --satauto --memory-limit=<80%_of_your_main_memory> <problem-file>
```

In general, different proof problems are easy for different
strategies. If you run

```sh
eprover --auto-schedule --memory-limit=<80%_of_your_main_memory> <problem-file>
```

or

```sh
eprover --satauto-schedule --memory-limit=<80%_of_your_main_memory> <problem-file>
```

the prover will try a series of strategies on the problem. It assumes
a 300 second run time - if you impose a different one externally, it
is important to let E know via the `--cpu-limit=XXX` option so that it
can adjust the schedule.

One of the features of E is the ability to produce semi-readable
proofs. To use this, type

```sh
eprover --proof-object <other-stuff>
```

By default, E will now automatically detect the input format (LOP,
TPTP-2 or TPTP-3), and will select the matching output format (PCL2
for LOP and TPTP-2 inputs, TPTP-3 for TPTP-3 inputs).

You can check the proof objects in PCL format for correctness using
the tool checkproof in the same directory. "checkproof -h" should give
you all necessary information. Note that checkproof cannot yet deal
with the full first order part, and will skip anything not
clausal. Also, support for independent provers in checkproof can be
subject to bit-rot, as other systems and interfaces change.



## Bug reports and questions

We welcome bug reports and even reasonable questions. If the prover
behaves in an unexpected way, please include the following
information:

- What did you observe?
- What did you expect?
- The output of `eprover --version`
- The full commandline that lead to the unexpected behaviour
- The input file(s) that lead to the unexpected behaviour

Most bug reports should be send to <schulz@eprover.org>. Bugs with the
LFHO-version should be send to <petar.vukmirovic2@gmail.com>. Please
remember that this is an unpaid volunteer service ;-).