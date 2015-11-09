/*
  Copyright © 2015 Jan Synáček <jan.synacek@gmail.com>

  scheme-systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  scheme-systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with scheme-systemd; If not, see <http://www.gnu.org/licenses/>.
*/

#include <libguile.h>
#include <systemd/sd-daemon.h>
#include "common.h"

SCM_DEFINE(listen_fds, "sd-listen-fds", 1, 0, 0,
	   (SCM s_unset_environment),
	   "Check for file descriptors passed by the service manager as part\n"
	   "of the socket-based activation logic.\n"
	   "Return list of file descriptors with available data in them.")
{
	SCM s_fds = SCM_EOL;
	int r, i;

	r = sd_listen_fds(scm_is_true(s_unset_environment));
	if (r < 0)
		error_system("Failed to get listening fds", -r);

	for (i = 0; i < r; i++)
		s_fds = scm_cons(scm_from_int(SD_LISTEN_FDS_START+i), s_fds);

	return s_fds;
}

SCM_DEFINE(listed_fds_with_names, "sd-listen-fds-with-names", 1, 0, 0,
	   (SCM s_unset_environment),
	   "Check for file descriptors passed by the service manager as part\n"
	   "of the socket-based activation logic. Additionally, identification\n"
	   "names are provided with the available file descriptors."
	   "Return list of (file descriptor . name) pairs.")
{
	SCM s_fds = SCM_EOL;
	char **names;
	int r, i;

	r = sd_listen_fds_with_names(scm_is_true(s_unset_environment), &names);
	if (r < 0)
		error_system("Failed to get listening fds", -r);

	for (i = 0; i < r; i++) {
		s_fds = scm_cons(scm_cons(scm_from_int(SD_LISTEN_FDS_START+i),
					  scm_from_locale_string(names[i])),
				 s_fds);
		free(names[i]);
	}
	free(names);

	return s_fds;
}

SCM_DEFINE(notify, "sd-notify", 2, 0, 0,
	   (SCM s_unset_environment, SCM s_state),
	   "TBI.")
{
	return SCM_UNSPECIFIED;
}

SCM_DEFINE(pid_notify, "sd-pid-notify", 3, 0, 0,
	   (SCM s_pid, SCM s_unset_environment, SCM s_state),
	   "TBI.")
{
	return SCM_UNSPECIFIED;
}

SCM_DEFINE(pid_notify_with_fds, "sd-pid-notify-with-fds", 2, 0, 0,
	   (SCM s_unset_environment, SCM s_state),
	   "TBI.")
{
	return SCM_UNSPECIFIED;
}

SCM_DEFINE(booted_p, "sd-booted?", 0, 0, 0,
	   (),
	   "Return #t if the systemd was booted with systemd, otherwise return #f.")
{
	return sd_booted() ? SCM_BOOL_T : SCM_BOOL_F;
}

void init_daemon(void)
{
#include "daemon.x"
}
