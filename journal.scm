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

(define-module (journal)
  #:export (boot-id
            get-usage
            send!
            read!))
(load-extension "./libguile-journal" "init_journal")


(define (boot-id)
  "Return boot ID of the current boot."
  (journal-boot-id))

(define (get-usage)
  "Return journal usage in bytes."
  (journal-usage))

(define* (send! message #:key (priority 6) (fields '()))
  "Send MESSAGE to the journal.

Use PRIORITY to set log level as defined by syslog. Possible values
are 0 (emergency) to 7 (debug). Default is 6. See man syslog(2) for
additional information.

Additional user-defined FIELDS can be specified in a form of an
associative list. By default, this list is empty. See man
systemd.journal-fields(7) for possible user journal fields values.

Example:
(send! \"Testing message\"
       #:priority 1
       #:fields '((\"CODE_FUNC\" . \"my_testing_func\")
                  (\"CODE_LINE\" . 10)))
"
  (journal-send
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

(define (read! fields)
  "Read entries from the journal.

FIELDS is a list of matches and symbols 'and or 'or. A match is specified as a string in
format \"FIELD=VALUE\". Multiple matches can be combined to more complex expressions using
the 'and / 'or symbols. The FIELDS are processed left to right and added to the filter. If
'or is encountered, all previously entered matches are combined using the logical OR with
all matches added afterwards. Similarly, 'and combines all previously
added matches in a logical AND.  For more information, see man
sd_journal_add_match(3).

The resulting entries are returned in an array of associative lists, each representing one
entry in the form of (FIELD . VALUE) pairs.

The following example returns all entries of the current boot, plus all entries generated
by the polkit service:

(read! (list (string-append \"_BOOT_ID=\" (boot-id))
             'or
             \"_SYSTEMD_UNIT=polkit.service\"))
"
  (journal-read fields))
