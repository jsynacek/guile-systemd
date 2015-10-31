# scheme-systemd
Experimental Scheme bindings for systemd.

## Requirements
- gcc
- guile-2.0.11+
- systemd-v225+

## Installation
None, yet...

## Usage
First, compile the bindings.
```bash
$ make
```

The compilation will create a dynamically linked library called **libguile-journal.so**.

Fire up guile.
```bash

$ guile -l journal.scm
```

In the guile repl, load the module and start using it.
```
scheme@(guile-user)> (use-modules (journal))
scheme@(guile-user)> (send! "Hello journal!" #:priority 6)
```

## Examples
Runnable examples with comments are presented in **example.scm**.

Run
```bash

$ guile -l journal.scm
```
and then use the interactive REPL to play around with them.
