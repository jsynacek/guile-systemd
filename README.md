# scheme-systemd
Experimental Scheme bindings for systemd.

## Requirements
- gcc
- guile-2.0.11+
- systemd-v225+

## Installation
Compile the bindings.
```bash
$ make
```

Install the bindings.
```bash
# make install
```

## Usage
Fire up guile.
```bash

$ guile
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

$ guile -l example.scm
```
and then use the interactive REPL to play around with them.
