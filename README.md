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
scheme@(guile-user)> (use-modules (systemd journal))
scheme@(guile-user)> (journal-send "Hello journal!" #:priority 6)
```

## Examples
Runnable examples with comments are presented in **example.scm**.

Run
```bash

$ guile -l example.scm
```
and then use the interactive REPL to play around with them.


## Socket activation
The following example shows how to test a simple socket activated service.

Create **/etc/systemd/system/test-socket-activation.socket** with the following
contents.
```
[Socket]
ListenDatagram=/run/test.sk
```

Next, create **/etc/systemd/system/test-socket-activation.service**.
```
[Service]
ExecStart=/var/tmp/test-socket-activation.scm
```

The **/var/tmp/test-socket-activation.scm** script's contents:
```scheme
#!/usr/bin/guile -s
!#

(use-modules (ice-9 rw))
(use-modules (systemd daemon))

(for-each (lambda (fd)
            (let* ((buf (make-string 4096)))
              (read-string!/partial buf (fdes->inport fd))
              (format #t "fd[~a]: ~a" fd buf) (newline)))
          (sd-listen-fds #f))
```

Make sure to make the script executable.
```
$ chmod +x /var/tmp/test-socket-activation.scm
```

Reload the systemd daemon and start the socket unit.
```
# systemctl daemon-reload && systemctl start test-socket-activation.socket
```

You can now send a message to the **/run/test.sk** socket. After you have
successfully done that, use journalctl to verify the output. You should see
something like this:

```
Nov 09 14:08:25 rawhide test-socket-activation.scm[585]: fd[3]: guile is awesome
```

If you would like to see the socket's identification along with the file descriptor number,
use the following for-each loop to replace the one in **/var/tmp/test-socket-activation.scm**.


```scheme
(for-each (lambda (fd-pair)
            (let* ((fd (car fd-pair))
                   (fd-name (cdr fd-pair))
                   (buf (make-string 4096)))
              (read-string!/partial buf (fdes->inport fd))
              (format #t "fd[~a:~a]: ~a" fd-name fd buf) (newline)))
          (sd-listen-fds-with-names #f))
```

The resulting output in the journal then looks similarly to
```
Nov 09 14:13:30 rawhide test-socket-activation.scm[658]: fd[test-socket-activation.socket:3]: guile is awesome
```
