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
#include "common.h"

/* XXX: This is an awful hack that is here because the systemd API hides
   the sd_journal struct with a typedef. I should really try to figure
   out another way to do this. The following definitions are not exactly
   what is in the library, but the size and internal data layout should
   be the same. */
#include <stdbool.h>
struct Location {
        int type;
        bool seqnum_set;
        bool realtime_set;
        bool monotonic_set;
        bool xor_hash_set;
        uint64_t seqnum;
        sd_id128_t seqnum_id;
        uint64_t realtime;
        uint64_t monotonic;
        sd_id128_t boot_id;
        uint64_t xor_hash;
};
struct sd_journal {
        char *path;
        char *prefix;
        void *files;
        void *mmap;
        struct Location current_location;
        void *current_file;
        uint64_t current_field;
        void *level0, *level1, *level2;
        pid_t original_pid;
        int inotify_fd;
        unsigned current_invalidate_counter, last_invalidate_counter;
        uint64_t last_process_usec;
        char *unique_field;
        void *unique_file;
        uint64_t unique_offset;
        int flags;
        bool on_network;
        bool no_new_files;
        bool unique_file_lost;
        size_t data_threshold;
        void *directories_by_path;
        void *directories_by_wd;
        void *errors;
};


static scm_t_bits journal_tag;

SCM_DEFINE(journal_send, "journal-sendv", 1, 0, 0,
	   (SCM s_fields),
	   "Send an entry consisting of FIELDS to the journal.")
{
	struct iovec *iov;
	int i, n, r;

	n = scm_to_int(scm_length(s_fields));
	iov = alloca(n * sizeof(struct iovec));

	for (i = 0; i < n; i++) {
		char *msg;

		msg = scm_to_locale_string(scm_list_ref(s_fields, scm_from_int(i)));
		iov[i].iov_base = msg;
		iov[i].iov_len = strlen(msg);
	}

	r = sd_journal_sendv(iov, n);
	if (r < 0)
		error_system("Failed to send data to journal", -r);

	return SCM_UNSPECIFIED;
}

SCM_SYMBOL(sym_local_only, "local-only");
SCM_SYMBOL(sym_runtime_only, "runtime-only");
SCM_SYMBOL(sym_system, "system");
SCM_SYMBOL(sym_current_user, "current-user");

SCM_DEFINE(journal_open, "journal-open", 1, 0, 0,
	   (SCM s_flags),
	   "Open journal using FLAGS and return a journal smob."
	   "Possible flags are symbols: local-only, runtime-only, system, current-user.\n"
	   "They map to the journal's internal flags SD_JOURNAL_LOCAL_ONLY,\n"
	   "SD_JOURNAL_RUNTIME_ONLY, SD_JOURNAL_SYSTEM and SD_JOURNAL_CURRENT_USER respectively.")
{
	SCM smob;
	sd_journal *j;
	int flags = 0;
	int i, n, r;

	n = scm_to_int(scm_length(s_flags));
	for (i = 0; i < n; i++) {
		SCM item;

		item = scm_list_ref(s_flags, scm_from_int(i));
		if (scm_is_eq(item, sym_local_only))
			flags |= SD_JOURNAL_LOCAL_ONLY;
		else if (scm_is_eq(item, sym_runtime_only))
			flags |= SD_JOURNAL_RUNTIME_ONLY;
		else if (scm_is_eq(item, sym_system))
			flags |= SD_JOURNAL_SYSTEM;
		else if (scm_is_eq(item, sym_current_user))
			flags |= SD_JOURNAL_CURRENT_USER;
		else
			scm_error_scm(scm_from_latin1_symbol("misc-error"),
				      scm_from_latin1_string(__func__),
				      scm_from_locale_string("Unknown flag: ~A"), scm_list_1(item),
				      SCM_EOL);
	}

	r = sd_journal_open(&j, flags);
	if (r < 0)
		error_system("Failed to open journal", -r);

	SCM_NEWSMOB(smob, journal_tag, j);

	return smob;
}

SCM_DEFINE(journal_add_match, "journal-add-match", 2, 0, 0,
	   (SCM smob, SCM s_match),
	   "Add MATCH to the journal.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	char *data;
	int r;

	data = scm_to_locale_string(s_match);
	r = sd_journal_add_match(j, data, strlen(data));
	if (r < 0)
		error_system("Failed to add match", -r);

	return SCM_UNSPECIFIED;
}

SCM_DEFINE(journal_add_conjunction, "journal-add-conjunction", 1, 0, 0,
	   (SCM smob),
	   "Add conjunction to the journal.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	int r;

	r = sd_journal_add_conjunction(j);
	if (r < 0)
		error_system("Failed to add conjunction", -r);

	return SCM_UNSPECIFIED;
}

SCM_DEFINE(journal_add_disjunction, "journal-add-disjunction", 1, 0, 0,
	   (SCM smob),
	   "Add disjunction to the journal.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	int r;

	r = sd_journal_add_disjunction(j);
	if (r < 0)
		error_system("Failed to add disjunction", -r);

	return SCM_UNSPECIFIED;
}

SCM_DEFINE(journal_flush_matches, "journal-flush-matches", 1, 0, 0,
	   (SCM smob),
	   "Flush all matches from the journal.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);

	sd_journal_flush_matches(j);
	return SCM_UNSPECIFIED;
}

SCM_DEFINE(journal_seek_head, "journal-seek-head", 1, 0, 0,
	   (SCM smob),
	   "Seek to the head of the journal.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	int r;

	r = sd_journal_seek_head(j);
	if (r < 0)
		error_system("Failed to seek to the journal head", -r);

	return SCM_UNSPECIFIED;
}

SCM_DEFINE(journal_seek_tail, "journal-seek-tail", 1, 0, 0,
	   (SCM smob),
	   "Seek to the tail of the journal.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	int r;

	r = sd_journal_seek_tail(j);
	if (r < 0)
		error_system("Failed to seek to the journal tail", -r);

	return SCM_UNSPECIFIED;
}

SCM_DEFINE(journal_seek_monotonic_usec, "journal-seek-monotonic-usec", 3, 0, 0,
	   (SCM smob, SCM s_boot_id, SCM s_usec),
	   "Seek to the entry with the specified BOOT_ID and monotonic USEC timestamp.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	sd_id128_t boot_id;
	int r;

	r = sd_id128_from_string(scm_to_locale_string(s_boot_id), &boot_id);
	if (r < 0)
		error_system("Failed to create boot id from string", -r);


	r = sd_journal_seek_monotonic_usec(j, boot_id, scm_to_uint64(s_usec));
	if (r < 0)
		error_system("Failed to seek to monotonic timestamp", -r);

	return SCM_UNSPECIFIED;
}

SCM_DEFINE(journal_seek_realtime_usec, "journal-seek-realtime-usec", 2, 0, 0,
	   (SCM smob, SCM s_usec),
	   "Seek to the entry with the specified realtime (wallclock) USEC timestamp.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	int r;

	r = sd_journal_seek_realtime_usec(j, scm_to_uint64(s_usec));
	if (r < 0)
		error_system("Failed to seek to realtime timestamp", -r);

	return SCM_UNSPECIFIED;
}

SCM_DEFINE(journal_next, "journal-next", 1, 0, 0,
	   (SCM smob),
	   "Advance to next entry of the journal.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);

	return scm_from_int(sd_journal_next(j));
}

SCM_DEFINE(journal_previous, "journal-previous", 1, 0, 0,
	   (SCM smob),
	   "Advance to previous entry of the journal.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);

	return scm_from_int(sd_journal_previous(j));
}

SCM_DEFINE(journal_get_data, "journal-get-data", 2, 0, 0,
	   (SCM smob, SCM s_field),
	   "Retrieve FIELD data of the current entry from the journal.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	char *data;
	size_t length;
	int r;

	r = sd_journal_get_data(j, scm_to_locale_string(s_field), (const void **)&data, &length);
	if (r < 0)
		error_system("Failed to retrieve data from journal", -r);

	return scm_from_locale_string((char *)data);
}

SCM_DEFINE(journal_enumerate_data, "journal-enumerate-data", 1, 0, 0,
	   (SCM smob),
	   "Enumerate data of the current entry of the journal.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	char *data;
	size_t length;
	int r;

	r = sd_journal_enumerate_data(j, (const void **)&data, &length);
	if (r < 0)
		error_system("Failed to enumerate data from journal", -r);
	else if (r == 0)
		return SCM_BOOL_F;

	return scm_from_locale_string((char *)data);
}

SCM_DEFINE(journal_restart_data, "journal-restart-data", 1, 0, 0,
	   (SCM smob),
	   "Reset the data enumeration index to the beginning of the entry.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);

	sd_journal_restart_data(j);
	return SCM_UNSPECIFIED;
}

SCM_DEFINE(journal_query_unique, "journal-query-unique", 2, 0, 0,
	   (SCM smob, SCM s_field),
	   "Query the journal for all unique values the specified FIELD can take.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	int r;

	r = sd_journal_query_unique(j, scm_to_locale_string(s_field));
	if (r < 0)
		error_system("Failed to query journal for unique data", -r);

	return SCM_UNSPECIFIED;
}

SCM_DEFINE(journal_enumerate_unique, "journal-enumerate-unique", 1, 0, 0,
	   (SCM smob),
	   "Iterate through all data fields which match the previously selected\n"
	   "field name as set with `journal-query-unique'.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	char *data;
	size_t length;
	char *strdata;
	int r;

	r = sd_journal_enumerate_unique(j, (const void **)&data, &length);
	if (r < 0)
		error_system("Failed to enumerate unique data from journal", -r);
	else if (r == 0)
		return SCM_BOOL_F;

	strdata = alloca(length + 1);
	memcpy(strdata, data, length);
	strdata[length] = '\0';

	return scm_from_locale_string(strchr(strdata, '=') + 1);
}

SCM_DEFINE(journal_restart_unique, "journal-restart-unique", 1, 0, 0,
	   (SCM smob),
	   "Reset the data enumeration index to the beginning of the list of unique data.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);

	sd_journal_restart_unique(j);
	return SCM_UNSPECIFIED;
}

SCM_DEFINE(journal_get_realtime_usec, "journal-get-realtime-usec", 1, 0, 0,
	   (SCM smob),
	   "Get the realtime (wallclock) timestamp of the current journal entry.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	uint64_t usec;
	int r;

	r = sd_journal_get_realtime_usec(j, &usec);
	if (r < 0)
		error_system("Failed to get realtime usec", -r);

	return scm_from_uint64(usec);
}

SCM_DEFINE(journal_get_monotonic_usec, "journal-get-monotonic-usec", 1, 1, 0,
	   (SCM smob),
	   "Get the monotonic timestamp of the current journal entry.\n"
	   "Returns (USEC . BOOT_ID) pair, where USEC is the monotonic timestamp\n"
	   "in microseconds and BOOT_ID is the boot ID of that timestamp.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	sd_id128_t boot_id;
	char boot_id_str[33];
	uint64_t usec;
	int r;

	r = sd_journal_get_monotonic_usec(j, &usec, &boot_id);
	if (r < 0)
		error_system("Failed to get monotonic usec", -r);

	return scm_cons(scm_from_uint64(usec),
			scm_from_locale_string(sd_id128_to_string(boot_id, boot_id_str)));
}

SCM_DEFINE(journal_get_usage, "journal-get-usage", 1, 0, 0,
	   (SCM smob),
	   "Return journal usage in bytes.")
{
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	uint64_t bytes;
	int r;

	r = sd_journal_get_usage(j, &bytes);
	if (r < 0)
		error_system("Failed to get journal usage", -r);

	return scm_from_uint64(bytes);
}

/** Convenience. */

SCM_DEFINE(journal_boot_id, "journal-boot-id", 0, 0, 0,
	   (),
	   "Return boot ID of the current boot.")
{
	sd_id128_t boot_id;
	char boot_id_str[39+1] = {0, };
	int r;

	r = sd_id128_get_boot(&boot_id);
	if (r < 0)
		error_system("Failed to read boot id", -r);

	sd_id128_to_string(boot_id, boot_id_str);

	return scm_from_latin1_string(boot_id_str);
}

SCM_DEFINE(journal_enumerate_entry, "journal-enumerate-entry", 1, 0, 0,
	   (SCM smob),
	   "Return the current entry as an associative list.")
{
	SCM alist = SCM_EOL;
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	const void *data;
	size_t length;

	SD_JOURNAL_FOREACH_DATA(j, data, length) {
		char strdata[length+1], *key, *value;

		memcpy(strdata, data, length);
		strdata[length] = '\0';

		value = strchr(strdata, '=') + 1;
		key = strdata;
		strdata[value-key-1] = '\0';

		alist = scm_acons(scm_from_latin1_string(key),
				  scm_from_latin1_string(value),
				  alist);
	}

	return alist;
}

SCM_DEFINE(journal_enumerate_unique_entry, "journal-enumerate-unique-entries", 1, 0, 0,
	   (SCM smob),
	   "Return the unique entries as a list.")
{
	SCM list = SCM_EOL;
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	const void *data;
	size_t length;

	SD_JOURNAL_FOREACH_UNIQUE(j, data, length) {
		char strdata[length+1];

		memcpy(strdata, data, length);
		strdata[length] = '\0';

		list = scm_cons(scm_from_locale_string(strchr(strdata, '=') + 1), list);
	}

	return list;
}

SCM_DEFINE(journal_slurp_data, "journal-slurp-data", 1, 0, 0,
	   (SCM smob),
	   "Return all entries of the journal.")
{
	SCM array;
	scm_t_array_handle handle;
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);
	int i, n;

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

	array = scm_make_array(SCM_UNSPECIFIED, scm_list_1(scm_from_int(n)));
	scm_array_get_handle(array, &handle);
	i = 0;

	SD_JOURNAL_FOREACH(j) {
		scm_array_handle_set(&handle, i, journal_enumerate_entry(smob));
		i++;
	}

	scm_array_handle_release(&handle);
	return array;
}

static size_t close_journal(SCM smob)
{
	sd_journal_close((sd_journal *)SCM_SMOB_DATA(smob));
	return 0;
}

static int print_journal(SCM smob, SCM port, scm_print_state *pstate)
{
	SCM s_flags = SCM_EOL;
	sd_journal *j = (sd_journal *)SCM_SMOB_DATA(smob);

	if (j->flags & SD_JOURNAL_CURRENT_USER)
		s_flags = scm_cons(scm_symbol_to_string(sym_current_user), s_flags);
	if (j->flags & SD_JOURNAL_SYSTEM)
		s_flags = scm_cons(scm_symbol_to_string(sym_system), s_flags);
	if (j->flags & SD_JOURNAL_RUNTIME_ONLY)
		s_flags = scm_cons(scm_symbol_to_string(sym_runtime_only), s_flags);
	if (j->flags & SD_JOURNAL_LOCAL_ONLY)
		s_flags = scm_cons(scm_symbol_to_string(sym_local_only), s_flags);

	scm_puts("#<journal flags:", port);
	if (scm_is_true(scm_null_p(s_flags)))
		scm_puts("none", port);
	else
		scm_display(scm_string_join(s_flags,
					    scm_from_latin1_string(","),
					    scm_from_latin1_symbol("infix")),
			    port);
	scm_puts(">", port);
	return 1;
}

void init_journal(void)
{
	journal_tag = scm_make_smob_type("journal", sizeof(struct sd_journal));
	scm_set_smob_free(journal_tag, close_journal);
	scm_set_smob_print(journal_tag, print_journal);
#include "journal.x"
}
