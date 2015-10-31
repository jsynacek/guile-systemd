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
Show the current boot ID.
```scheme
(boot-id)
$1 = "381e59be988c4ca99b1947b83c4c6843"
```

Show how much space in MiB the journal files occupy on the local machine.
```scheme
(/ (get-usage) 1024.0 1024)
$2 = 808.44921875
```

Send a simple message.
```scheme
(send! "Testing info message")
(send! "Testing error message" #:priority 3)
```

Send message with additional fields.
```scheme
(send! "Some code error"
       #:priority 3
       #:fields '(("CODE_FUNC" . "fix_all_software_procedure")
                  ("CODE_LINE" . 1)))
```

Iterate over entries of this boot and print their fields.
```scheme
(define (display-entry entry)
  (display ":::ENTRY:::") (newline)
  (for-each (lambda (field)
              (display "  FIELD ==> ") (display field) (newline))
            entry))

(for-each (lambda (entry)
            (display-entry entry))
          (array->list (read! (list (string-append "_BOOT_ID=" (boot-id))))))

=>
:::ENTRY:::
  FIELD ==> (_SOURCE_REALTIME_TIMESTAMP . 1446241254117121)
  FIELD ==> (_PID . 3627)
  FIELD ==> (MESSAGE . Testing error message)
  FIELD ==> (CODE_LINE . 37)
  FIELD ==> (PRIORITY . 3)
  FIELD ==> (_SYSTEMD_UNIT . session-16.scope)
  FIELD ==> (_SYSTEMD_SESSION . 16)
...
```
