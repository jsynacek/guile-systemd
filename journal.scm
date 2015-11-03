;;; Copyright © 2015 Jan Synáček <jan.synacek@gmail.com>
;;;
;;; scheme-systemd is free software; you can redistribute it and/or modify it
;;; under the terms of the GNU Lesser General Public License as published by
;;; the Free Software Foundation; either version 2.1 of the License, or
;;; (at your option) any later version.
;;;
;;; scheme-systemd is distributed in the hope that it will be useful, but
;;; WITHOUT ANY WARRANTY; without even the implied warranty of
;;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
;;; Lesser General Public License for more details.
;;;
;;; You should have received a copy of the GNU Lesser General Public License
;;; along with scheme-systemd; If not, see <http://www.gnu.org/licenses/>.

(define-module (systemd journal)
  #:export (journal-sendv
            journal-open
            journal-add-match
            journal-add-conjunction
            journal-add-disjunction
            journal-flush-matches
            journal-seek-head
            journal-seek-tail
            journal-seek-monotonic-usec
            journal-seek-realtime-usec
            journal-next
            journal-previous
            journal-get-data
            journal-enumerate-data
            journal-restart-data
            journal-query-unique
            journal-enumerate-unique
            journal-restart-unique
            journal-get-realtime-usec
            journal-get-monotonic-usec
            journal-get-usage
            ;; Convenience.
            journal-boot-id
            journal-enumerate-entry
            journal-enumerate-unique-entries
            journal-slurp-data
            ;; Local.
            current-boot-match
            journal-send
            ))
(load-extension "libguile-journal" "init_journal")


(define (current-boot-match)
  "Return match string that matches the current boot."
  (string-append "_BOOT_ID=" (journal-boot-id)))

(define* (journal-send message #:key (priority 6) (fields '()))
  "Send MESSAGE to the journal.

Use PRIORITY to set log level as defined by syslog. Possible values
are 0 (emergency) to 7 (debug). Default is 6. See man syslog(2) for
additional information.

Additional user-defined FIELDS can be specified in a form of an
associative list. By default, this list is empty. See man
systemd.journal-fields(7) for possible user journal fields values.

Example:
(journal-send \"Testing message\"
              #:priority 1
              #:fields '((\"CODE_FUNC\" . \"my_testing_func\")
                         (\"CODE_LINE\" . 10)))
"
  (journal-sendv
   (apply append
          (list (string-append "MESSAGE=" message)
                (string-append "PRIORITY=" (number->string priority)))
          (map (lambda (cell)
                 (let ((value (cdr cell)))
                   (list (string-append (car cell) "=" (if (number? value)
                                                           (number->string value)
                                                           ;; ...else assume it's a string.
                                                           value)))))
               fields))))
