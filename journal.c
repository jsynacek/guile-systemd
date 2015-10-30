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

#include <alloca.h>
#include <libguile.h>
#include <systemd/sd-journal.h>


#define _error_with_args(key, subr, message, args, data) \
  scm_error_scm(scm_from_latin1_symbol(key),             \
  scm_from_locale_string(subr),                          \
  scm_from_locale_string(message),                       \
  args, data)

#define _error(message) _error_with_args("misc-error", \
					 __func__,     \
					 message,      \
					 SCM_EOL,      \
					 SCM_EOL)


SCM_DEFINE(journal_send, "journal-send", 1, 0, 0,
	   (SCM messages),
	   "Send a list of messages to the journal.")
{
	struct iovec *iov;
	int i, n, r;

	n = scm_to_int(scm_length(messages));
	iov = alloca(n * sizeof(struct iovec));

	for (i = 0; i < n; i++) {
		char *msg;

		msg = scm_to_locale_string(scm_list_ref(messages, scm_from_int(i)));
		iov[i].iov_base = msg;
		iov[i].iov_len = strlen(msg);
	}

	r = sd_journal_sendv(iov, n);
	if (r < 0)
		return _error("failed to send data to journal");

	return SCM_UNSPECIFIED;
}

SCM_DEFINE(journal_boot_id, "journal-boot-id", 0, 0, 0,
	   (),
	   "Return boot ID of the current boot.")
{
	sd_id128_t boot_id;
	char boot_id_str[39+1] = {0, };
	int r;

	r = sd_id128_get_boot(&boot_id);
	if (r < 0)
		return _error("failed to read boot id");

	sd_id128_to_string(boot_id, boot_id_str);

	return scm_from_latin1_string(boot_id_str);
}

SCM_DEFINE(journal_usage, "journal-usage", 0, 0, 0,
	   (),
	   "Return journal usage in bytes.")
{
	sd_journal *j;
	uint64_t bytes;
	int r;

	r = sd_journal_open(&j, SD_JOURNAL_LOCAL_ONLY);
	if (r < 0)
		_error("failed to open journal");

	r = sd_journal_get_usage(j, &bytes);
	if (r < 0)
		_error("failed to get journal usage");

	return scm_from_uint64(bytes);
}



SCM_DEFINE(journal_read, "journal-read", 1, 0, 0,
	   (SCM fields),
	   "Read entries from the journal and return the result in an array.")
{
	SCM array;
	scm_t_array_handle handle;
	sd_journal *j;
	const void *data;
	size_t len;
	int i, n, r;

	r = sd_journal_open(&j, SD_JOURNAL_LOCAL_ONLY);
	if (r < 0)
		_error("failed to open journal");

	/* First go over the fields and create the filter. */
	n = scm_to_int(scm_length(fields));
	for (i = 0; i < n; i++) {
		SCM item;

		item = scm_list_ref(fields, scm_from_int(i));

		if (scm_is_string(item)) {
			char *match;

			match = scm_to_locale_string(item);
			sd_journal_add_match(j, match, strlen(match));

		} else if (scm_is_symbol(item)) {
			if (scm_is_eq(item, scm_from_locale_symbol("and")))
				sd_journal_add_conjunction(j);
			else if (scm_is_eq(item, scm_from_locale_symbol("or")))
				sd_journal_add_disjunction(j);
			else {
				sd_journal_close(j);
				_error_with_args("misc-error",
						 __func__,
						 "unknown junction: ~A",
						 scm_list_1(item),
						 SCM_EOL);
			}
		}
	}

	/* Iterate over the journal and collect the entries with the filter applied.
	 * A scheme array is used here, because it's very fast an convenient to work
	 * with from C. Despite using two passes to first count the number of entries
	 * that will be returned and then creating the array, it's still *much* faster
	 * that appending the values to a list in a single pass.
	 */
	n = 0;
	SD_JOURNAL_FOREACH(j)
		n++;
	sd_journal_seek_head(j);

	array = scm_make_array(SCM_UNSPECIFIED, scm_list_1(scm_from_uint(n)));
	scm_array_get_handle(array, &handle);
	i = 0;

	SD_JOURNAL_FOREACH(j) {
		SCM alist = SCM_EOL;

		SD_JOURNAL_FOREACH_DATA(j, data, len) {
			char strdata[len+1], *key, *value;

			memcpy(strdata, data, len);
			strdata[len] = '\0';

			value = strchr(strdata, '=') + 1;
			key = strdata;
			strdata[value-key-1] = '\0';

			alist = scm_acons(scm_from_latin1_string(key),
					  scm_from_latin1_string(value),
					  alist);
		}

		scm_array_handle_set(&handle, i, alist);
		i++;
	}

	scm_array_handle_release(&handle);
	sd_journal_close(j);
	return array;
}

void init_journal(void)
{
#include "journal.x"
}
