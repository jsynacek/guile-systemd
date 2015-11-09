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

(define-module (systemd daemon)
  #:export (SD_LISTEN_FDS_START
            sd-listen-fds
            sd-listen-fds-with-names
            sd-booted?))
(load-extension "libguile-daemon" "init_daemon")

(define SD_LISTEN_FDS_START 3)
