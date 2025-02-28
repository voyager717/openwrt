// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <glib.h>
#include <string.h>
#include <glib/gstdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ksmbd_server.h>

#include <config_parser.h>
#include <ksmbdtools.h>
#include <management/user.h>
#include <management/share.h>

struct smbconf_global global_conf;
struct smbconf_parser parser;
struct smbconf_group *global_group;

unsigned long long memparse(const char *v)
{
	char *eptr;

	unsigned long long ret = strtoull(v, &eptr, 0);

	switch (*eptr) {
	case 'E':
	case 'e':
		ret <<= 10;
	case 'P':
	case 'p':
		ret <<= 10;
	case 'T':
	case 't':
		ret <<= 10;
	case 'G':
	case 'g':
		ret <<= 10;
	case 'M':
	case 'm':
		ret <<= 10;
	case 'K':
	case 'k':
		ret <<= 10;
	}

	return ret;
}

static void kv_release_cb(gpointer p)
{
	g_free(p);
}

static int is_ascii_space_tab(char c)
{
	return c == ' ' || c == '\t';
}

static int is_a_comment(char *line)
{
	return (*line == 0x00 || *line == ';' || *line == '\n' || *line == '#');
}

static int is_a_group(char *line)
{
	char *p = line;

	if (*p != '[')
		return 0;
	p++;
	while (*p && *p != ']')
		p = g_utf8_find_next_char(p, NULL);
	if (*p != ']')
		return 0;
	return 1;
}

static int add_new_group(char *line)
{
	char *begin = line;
	char *end = line;
	char *name = NULL;
	struct smbconf_group *group = NULL;
	struct smbconf_group *lookup;

	while (*end && *end != ']')
		end = g_utf8_find_next_char(end, NULL);

	name = g_ascii_strdown(begin + 1, end - begin - 1);
	if (!name)
		goto out_free;

	lookup = g_hash_table_lookup(parser.groups, name);
	if (lookup) {
		parser.current = lookup;
		pr_info("SMB conf: multiple group definitions `%s'\n",
				name);
		g_free(name);
		return 0;
	}

	group = g_malloc(sizeof(struct smbconf_group));
	if (!group)
		goto out_free;

	group->name = name;
	group->kv = g_hash_table_new_full(g_str_hash,
					  g_str_equal,
					  kv_release_cb,
					  kv_release_cb);
	if (!group->kv)
		goto out_free;

	parser.current = group;
	g_hash_table_insert(parser.groups, group->name, group);
	return 0;

out_free:
	g_free(name);
	if (group && group->kv)
		g_hash_table_destroy(group->kv);
	g_free(group);
	return -ENOMEM;
}

static int add_group_key_value(char *line)
{
	char *key, *value;

	key = strchr(line, '=');
	if (!key)
		return -EINVAL;

	value = key;
	*key = 0x00;
	key--;
	value++;

	while (is_ascii_space_tab(*key))
		key--;
	while (is_ascii_space_tab(*value))
		value++;

	if (is_a_comment(value))
		return 0;

	if (g_hash_table_lookup(parser.current->kv, key)) {
		pr_info("SMB conf: multuple key-value [%s] %s\n",
				parser.current->name, key);
		return 0;
	}

	key = g_strndup(line, key - line + 1);
	value = g_strdup(value);

	if (!key || !value) {
		g_free(key);
		g_free(value);
		return -ENOMEM;
	}

	g_hash_table_insert(parser.current->kv, key, value);
	return 0;
}

static int process_smbconf_entry(char *data)
{
	while (is_ascii_space_tab(*data))
		data++;

	if (is_a_comment(data))
		return 0;

	if (is_a_group(data))
		return add_new_group(data);

	return add_group_key_value(data);
}

static int __mmap_parse_file(const char *fname, int (*callback)(char *data))
{
	GMappedFile *file;
	GError *err = NULL;
	gchar *contents;
	int len;
	char *delim;
	int fd, ret = 0;

	fd = g_open(fname, O_RDONLY, 0);
	if (fd == -1) {
		ret = errno;
		pr_err("Can't open `%s': %s\n", fname, strerr(ret));
		return -ret;
	}

	file = g_mapped_file_new_from_fd(fd, FALSE, &err);
	if (err) {
		pr_err("%s: `%s'\n", err->message, fname);
		g_error_free(err);
		ret = -EINVAL;
		goto out;
	}

	contents = g_mapped_file_get_contents(file);
	if (!contents)
		goto out;

	len = g_mapped_file_get_length(file);
	while (len > 0) {
		delim = strchr(contents, '\n');
		if (!delim)
			delim = contents + len - 1;

		if (delim) {
			size_t sz = delim - contents;
			char *data;

			if (delim == contents) {
				contents = delim + 1;
				len--;
				continue;
			}

			if (!sz)
				break;

			data = g_malloc(sz + 1);
			if (!data) {
				ret = -ENOMEM;
				goto out;
			}

			strncpy(data, contents, sz);
			data[sz] = 0x00;

			ret = callback(data);
			if (ret) {
				g_free(data);
				goto out;
			}

			g_free(data);
			contents = delim + 1;
			len -= (sz + 1);
		}
	}

	ret = 0;
out:
	if (file)
		g_mapped_file_unref(file);

	if (fd) {
		g_close(fd, &err);
		if (err) {
			pr_err("%s: %s\n", err->message, fname);
			g_error_free(err);
		}
	}
	return ret;
}

static int init_smbconf_parser(void)
{
	if (parser.groups)
		return 0;

	parser.groups = g_hash_table_new(g_str_hash, g_str_equal);
	if (!parser.groups)
		return -ENOMEM;
	return 0;
}

static void release_smbconf_group(gpointer k, gpointer v, gpointer user_data)
{
	struct smbconf_group *g = v;

	g_hash_table_destroy(g->kv);
	g_free(g->name);
	g_free(g);
}

static void release_smbconf_parser(void)
{
	if (!parser.groups)
		return;

	g_hash_table_foreach(parser.groups, release_smbconf_group, NULL);
	g_hash_table_destroy(parser.groups);
	parser.groups = NULL;
}

char *cp_ltrim(char *v)
{
	if (!v)
		return NULL;

	while (*v && *v == ' ')
		v++;
	if (*v == 0x00)
		return NULL;
	return v;
}

int cp_key_cmp(char *k, char *v)
{
	if (!k || !v)
		return -1;
	return g_ascii_strncasecmp(k, v, strlen(v));
}

char *cp_get_group_kv_string(char *v)
{
	return g_strdup(v);
}

int cp_get_group_kv_bool(char *v)
{
	if (!g_ascii_strncasecmp(v, "yes", 3) ||
		!g_ascii_strncasecmp(v, "1", 1) ||
		!g_ascii_strncasecmp(v, "true", 4) ||
		!g_ascii_strncasecmp(v, "enable", 6))
		return 1;
	return 0;
}

int cp_get_group_kv_config_opt(char *v)
{
	if (!g_ascii_strncasecmp(v, "disabled", 8))
		return KSMBD_CONFIG_OPT_DISABLED;
	if (!g_ascii_strncasecmp(v, "enabled", 7))
		return KSMBD_CONFIG_OPT_ENABLED;
	if (!g_ascii_strncasecmp(v, "auto", 4))
		return KSMBD_CONFIG_OPT_AUTO;
	if (!g_ascii_strncasecmp(v, "mandatory", 9))
		return KSMBD_CONFIG_OPT_MANDATORY;
	return KSMBD_CONFIG_OPT_DISABLED;
}

unsigned long cp_get_group_kv_long_base(char *v, int base)
{
	return strtoul(v, NULL, base);
}

unsigned long cp_get_group_kv_long(char *v)
{
	return cp_get_group_kv_long_base(v, 10);
}

char **cp_get_group_kv_list(char *v)
{
	/*
	 * SMB conf lists are "tabs, spaces, commas" separated.
	 */
	return g_strsplit_set(v, "\t ,", -1);
}

void cp_group_kv_list_free(char **list)
{
	g_strfreev(list);
}

static int cp_add_global_guest_account(gpointer _v)
{
	struct ksmbd_user *user;

	if (usm_add_new_user(cp_get_group_kv_string(_v),
			     g_strdup("NULL"))) {
		pr_err("Unable to add guest account\n");
		return -ENOMEM;
	}

	user = usm_lookup_user(_v);
	if (!user) {
		pr_err("Fatal error: unable to find `%s' account.\n",
			(const char *) _v);
		return -EINVAL;
	}

	set_user_flag(user, KSMBD_USER_FLAG_GUEST_ACCOUNT);
	put_ksmbd_user(user);
	global_conf.guest_account = cp_get_group_kv_string(_v);
	return 0;
}

static gboolean global_group_kv(gpointer _k, gpointer _v, gpointer user_data)
{
	if (!cp_key_cmp(_k, "server string")) {
		global_conf.server_string = cp_get_group_kv_string(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "workgroup")) {
		global_conf.work_group = cp_get_group_kv_string(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "netbios name")) {
		global_conf.netbios_name = cp_get_group_kv_string(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "server min protocol")) {
		global_conf.server_min_protocol = cp_get_group_kv_string(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "server signing")) {
		global_conf.server_signing = cp_get_group_kv_config_opt(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "server max protocol")) {
		global_conf.server_max_protocol = cp_get_group_kv_string(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "guest account")) {
		cp_add_global_guest_account(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "max active sessions")) {
		global_conf.sessions_cap = cp_get_group_kv_long(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "tcp port")) {
		if (!global_conf.tcp_port)
			global_conf.tcp_port = cp_get_group_kv_long(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "ipc timeout")) {
		global_conf.ipc_timeout = cp_get_group_kv_long(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "max open files")) {
		global_conf.file_max = cp_get_group_kv_long(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "restrict anonymous")) {
		global_conf.restrict_anon = cp_get_group_kv_long(_v);
		if (global_conf.restrict_anon > KSMBD_RESTRICT_ANON_TYPE_2 ||
				global_conf.restrict_anon < 0) {
			global_conf.restrict_anon = 0;
			pr_err("Invalid restrict anonymous value\n");
		}

		return TRUE;
	}

	if (!cp_key_cmp(_k, "map to guest")) {
		global_conf.map_to_guest = KSMBD_CONF_MAP_TO_GUEST_NEVER;
		if (!cp_key_cmp(_v, "bad user"))
			global_conf.map_to_guest =
				KSMBD_CONF_MAP_TO_GUEST_BAD_USER;
		if (!cp_key_cmp(_v, "bad password"))
			global_conf.map_to_guest =
				KSMBD_CONF_MAP_TO_GUEST_BAD_PASSWORD;
		if (!cp_key_cmp(_v, "bad uid"))
			global_conf.map_to_guest =
				KSMBD_CONF_MAP_TO_GUEST_BAD_UID;
		return TRUE;
	}

	if (!cp_key_cmp(_k, "bind interfaces only")) {
		global_conf.bind_interfaces_only = cp_get_group_kv_bool(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "interfaces")) {
		global_conf.interfaces = cp_get_group_kv_list(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "deadtime")) {
		global_conf.deadtime = cp_get_group_kv_long(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "smb2 leases")) {
		if (cp_get_group_kv_bool(_v))
			global_conf.flags |= KSMBD_GLOBAL_FLAG_SMB2_LEASES;
		else
			global_conf.flags &= ~KSMBD_GLOBAL_FLAG_SMB2_LEASES;

		return TRUE;
	}

	if (!cp_key_cmp(_k, "root directory")) {
		global_conf.root_dir = cp_get_group_kv_string(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "smb2 max read")) {
		global_conf.smb2_max_read = memparse(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "smb2 max write")) {
		global_conf.smb2_max_write = memparse(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "smb2 max trans")) {
		global_conf.smb2_max_trans = memparse(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "smb3 encryption")) {
		if (cp_get_group_kv_bool(_v))
			global_conf.flags |= KSMBD_GLOBAL_FLAG_SMB3_ENCRYPTION;
		else
			global_conf.flags &= ~KSMBD_GLOBAL_FLAG_SMB3_ENCRYPTION;

		return TRUE;
	}

	if (!cp_key_cmp(_k, "share:fake_fscaps")) {
		global_conf.share_fake_fscaps = cp_get_group_kv_long(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "kerberos service name")) {
		global_conf.krb5_service_name = cp_get_group_kv_string(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "kerberos keytab file")) {
		global_conf.krb5_keytab_file = cp_get_group_kv_string(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "server multi channel support")) {
		if (cp_get_group_kv_bool(_v))
			global_conf.flags |= KSMBD_GLOBAL_FLAG_SMB3_MULTICHANNEL;
		else
			global_conf.flags &= ~KSMBD_GLOBAL_FLAG_SMB3_MULTICHANNEL;

		return TRUE;
	}

	if (!cp_key_cmp(_k, "smb2 max credits")) {
		global_conf.smb2_max_credits = memparse(_v);
		return TRUE;
	}

	if (!cp_key_cmp(_k, "smbd max io size")) {
		global_conf.smbd_max_io_size = memparse(_v);
		return TRUE;
	}

	/* At this point, this is an option that must be applied to all shares */
	return FALSE;
}

static void global_conf_default(void)
{
	/* The SPARSE_FILES file system capability flag is set by default */
	global_conf.share_fake_fscaps = 64;
}

static void global_conf_create(void)
{
	/*
	 * This will transfer server options to global_conf, and leave behind
	 * in the global parser group, the options that must be applied to every
	 * share
	 */
	g_hash_table_foreach_remove(global_group->kv, global_group_kv, NULL);
}

static void global_conf_fixup_missing(void)
{
	int ret;

	/*
	 * Set default global parameters which were not specified
	 * in smb.conf
	 */
	if (!global_conf.file_max)
		global_conf.file_max = KSMBD_CONF_FILE_MAX;
	if (!global_conf.server_string)
		global_conf.server_string =
			cp_get_group_kv_string(
					KSMBD_CONF_DEFAULT_SERVER_STRING);
	if (!global_conf.netbios_name)
		global_conf.netbios_name =
			cp_get_group_kv_string(KSMBD_CONF_DEFAULT_NETBIOS_NAME);
	if (!global_conf.work_group)
		global_conf.work_group =
			cp_get_group_kv_string(KSMBD_CONF_DEFAULT_WORK_GROUP);
	if (!global_conf.tcp_port)
		global_conf.tcp_port = KSMBD_CONF_DEFAULT_TPC_PORT;

	if (global_conf.sessions_cap <= 0)
		global_conf.sessions_cap = KSMBD_CONF_DEFAULT_SESS_CAP;

	if (global_conf.guest_account)
		return;

	ret = cp_add_global_guest_account(KSMBD_CONF_DEFAULT_GUEST_ACCOUNT);
	if (!ret)
		return;
	ret = cp_add_global_guest_account(KSMBD_CONF_FALLBACK_GUEST_ACCOUNT);
	if (ret)
		pr_err("Fatal error: Cannot set a global guest account %d\n",
			ret);
}

static void append_key_value(gpointer _k, gpointer _v, gpointer user_data)
{
	GHashTable *receiver = (GHashTable *) user_data;

	/* Don't override local share options */
	if (!g_hash_table_lookup(receiver, _k))
		g_hash_table_insert(receiver, g_strdup(_k), g_strdup(_v));
}

#define GROUPS_CALLBACK_STARTUP_INIT	0x1
#define GROUPS_CALLBACK_REINIT		0x2

static void groups_callback(gpointer _k, gpointer _v, gpointer user_data)
{
	struct smbconf_group *group = (struct smbconf_group *)_v;

	if (group != global_group) {
		if (global_group && g_ascii_strncasecmp(_k, "ipc$", 4))
			g_hash_table_foreach(global_group->kv,
					     append_key_value,
					     group->kv);

		shm_add_new_share(group);
	}
}

static int cp_add_ipc_share(void)
{
	char *comment = NULL, *guest = NULL;
	int ret = 0;

	if (g_hash_table_lookup(parser.groups, "ipc$"))
		return 0;

	comment = g_strdup("comment = IPC share");
	guest = g_strdup("guest ok = yes");
	ret = add_new_group("[IPC$]");
	ret |= add_group_key_value(comment);
	ret |= add_group_key_value(guest);
	if (ret) {
		pr_err("Unable to add IPC$ share\n");
		ret = -EINVAL;
		goto out;
	}
	return ret;

out:
	g_free(comment);
	g_free(guest);
	return ret;
}

static int __cp_parse_smbconfig(const char *smbconf, GHFunc cb, long flags)
{
	int ret;

	global_conf_default();

	ret = cp_smbconfig_hash_create(smbconf);
	if (ret)
		return ret;

	ret = cp_add_ipc_share();
	if (!ret) {
		global_group = g_hash_table_lookup(parser.groups, "global");

		if (global_group && (flags == GROUPS_CALLBACK_STARTUP_INIT))
			global_conf_create();

		g_hash_table_foreach(parser.groups,
				     groups_callback,
				     NULL);

		global_conf_fixup_missing();
	}
	cp_smbconfig_destroy();
	return ret;
}

int cp_parse_reload_smbconf(const char *smbconf)
{
	return __cp_parse_smbconfig(smbconf,
				    groups_callback,
				    GROUPS_CALLBACK_REINIT);
}

int cp_parse_smbconf(const char *smbconf)
{
	return __cp_parse_smbconfig(smbconf,
				    groups_callback,
				    GROUPS_CALLBACK_STARTUP_INIT);
}

int cp_parse_pwddb(const char *pwddb)
{
	return __mmap_parse_file(pwddb, usm_add_update_user_from_pwdentry);
}

int cp_smbconfig_hash_create(const char *smbconf)
{
	int ret = init_smbconf_parser();

	if (ret)
		return ret;
	return __mmap_parse_file(smbconf, process_smbconf_entry);
}

int cp_parse_subauth(const char *subauth_path)
{
	return __mmap_parse_file(subauth_path, usm_add_subauth_global_conf);
}

void cp_smbconfig_destroy(void)
{
	release_smbconf_parser();
}

int cp_parse_external_smbconf_group(char *name, char *opts)
{
	char *pos;
	int i, len;

	if (init_smbconf_parser())
		return -EINVAL;

	if (!opts || !name)
		return -EINVAL;

	len = strlen(opts);
	/* fake smb.conf input */
	for (i = 0; i < KSMBD_SHARE_CONF_MAX; i++) {
		pos = strstr(opts, KSMBD_SHARE_CONF[i]);
		if (!pos)
			continue;
		if (pos != opts)
			*(pos - 1) = '\n';
	}

	if (add_new_group(name))
		goto error;

	/* split input and feed to normal process_smbconf_entry() */
	while (len) {
		char *delim = strchr(opts, '\n');

		if (delim) {
			*delim = 0x00;
			len -= delim - opts;
		} else {
			len = 0;
		}

		process_smbconf_entry(opts);
		if (delim)
			opts = delim + 1;
	}
	return 0;

error:
	cp_smbconfig_destroy();
	return -EINVAL;
}
