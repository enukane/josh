#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

#include <jansson.h>

#if DEBUG
#define dprintf(fmt, ...) printf("DEBUG>> %u@%s: " fmt "\n", __LINE__, __func__, ##__VA_ARGS__)
#else
#define dprintf(fmt, ...)
#endif

char *
match_to_new_string(const char *str, regmatch_t *match)
{
	size_t len = match->rm_eo - match->rm_so;
	char *substr = NULL;

	dprintf("new string from '%s' %lld - %lld", str, match->rm_eo, match->rm_so);

	substr = (char *)malloc(len+1);
	if (substr == NULL) {
		return NULL;
	}

	strncpy(substr, str + match->rm_so, len);
	substr[len] = '\0';

	dprintf("new string is '%s'", substr);
	return substr;
}

json_t *
parse_json_arg(char *arg, int opt_array, int opt_booloff)
{
	const char *pattern_obj = "^\\{.+\\}$";
	const char *pattern_array = "^\\[.+\\]$";
	const char *pattern_truefalse = "^(true|false)$";
	const char *pattern_num = "^[0-9]+$";
	const char *pattern_eq = "^(.+)=(.+)$";

	regex_t reg;
	regmatch_t match[3];
	int match_elms = sizeof match / sizeof match[0];

	json_t *json = NULL;
	json_t *j_value = NULL;
	json_error_t j_error;

	char *key = NULL;
	char *value = NULL;

	/* object */
	regcomp(&reg, pattern_obj, REG_EXTENDED|REG_NOSUB);
	if (regexec(&reg, arg, 0, NULL, 0) != REG_NOMATCH) {
		dprintf("match object ('%s')", arg);
		json = json_loads(arg, 0, &j_error);
		if (json == NULL) {
			fprintf(stderr, "ERROR: failed to parse object (error=%s, str=%s)\n",
				j_error.text, arg);
		}
		return json;
	}
	regfree(&reg);

	/* array */
	regcomp(&reg, pattern_array, REG_EXTENDED|REG_NOSUB);
	if (regexec(&reg, arg, 0, NULL, 0) != REG_NOMATCH) {
		dprintf("match array ('%s')", arg);
		json = json_loads(arg, 0, &j_error);
		if (json == NULL) {
			fprintf(stderr, "ERROR: failed to parse arrayt (error=%s, str=%s)\n",
				j_error.text, arg);
		}
		return json;
	}
	regfree(&reg);

	/* true/false */
	regcomp(&reg, pattern_truefalse, REG_EXTENDED|REG_NOSUB);
	if (regexec(&reg, arg, 0, NULL, 0) != REG_NOMATCH) {
		dprintf("match boolean ('%s', opt_booloff=%d)", arg, opt_booloff);
		if (opt_booloff) {
			return json_string(arg);
		}
		if (strcmp(arg, "true") == 0) {
			return json_true();
		} else {
			return json_false();
		}
	}
	regfree(&reg);

	/* number */
	regcomp(&reg, pattern_num, REG_EXTENDED|REG_NOSUB);
	if (regexec(&reg, arg, 0, NULL, 0) != REG_NOMATCH) {
		dprintf("match integer ('%s')", arg);
		return json_integer(atoi(arg));
	}
	regfree(&reg);

	/* eq */
	regcomp(&reg, pattern_eq, REG_EXTENDED);
	if (regexec(&reg, arg, match_elms, match, 0) != REG_NOMATCH) {
		dprintf("match keyvalue ('%s')", arg);
		key = match_to_new_string(arg, &match[1]);
		value = match_to_new_string(arg, &match[2]);

		json = json_object();
		if (json == NULL) {
			fprintf(stderr, "ERROR: failed to allocate object\n");
			goto done_eq;
		}

		j_value = parse_json_arg(value, opt_array, opt_booloff);
		if (j_value == NULL) {
			fprintf(stderr, "ERROR: failed to parse value (value=%s)\n",
				value);
			goto done_eq;
		}

		dprintf("set eq '%s' => '%s'", key, value);
		if (json_object_set_new(json, key, j_value) < 0) {
			fprintf(stderr, "ERROR: failed to set value to object "
				"(key=%s, value=%s)\n", key, value);
			json_decref(j_value);
			goto done_eq;
		}

done_eq:
		free(key);
		free(value);
		return json;
	}

	/* string */
	dprintf("match string ('%s')", arg);
	json = json_string(arg);
	if (json == NULL) {
		fprintf(stderr, "ERROR: failed to parse string (string=%s)\n",
			arg);
		return NULL;
	}

	return json;
}

json_t *
parse_json_args(int head, int argc, char **argv, int opt_array, int opt_booloff)
{
	int idx = head;
	json_t *j_root = NULL;
	json_t *j_elm = NULL;

	if (opt_array) {
		j_root = json_array();
	} else {
		j_root = json_object();
	}

	if (j_root == NULL) {
		fprintf(stderr, "ERROR: failed to allocate root %s\n",
			opt_array ? "array" : "object");
		return NULL;
	}

	while (idx < argc) {
		j_elm = parse_json_arg(argv[idx], opt_array, opt_booloff);
		if (j_elm == NULL) {
			fprintf(stderr, "ERROR: failed to parse %d th arg "
				"(arg=%s)", idx - head, argv[idx]);
			return NULL;
		}

		if (opt_array) {
			json_array_append_new(j_root, j_elm);
		} else {
			if (!json_is_object(j_elm)) {
				fprintf(stderr, "ERROR: appending non-object into object "
					"(elm=%s)\n", argv[idx]);
				return NULL;
			}
			json_object_update(j_root, j_elm);
			json_decref(j_elm);
		}
		idx++;
	}

	return j_root;
}

void
usage_and_exit()
{
	printf("usage: jojo [option] <json string>\n");
	printf("\n");
	printf("    %-25s %-20s\n", "-a", "compose array");
	printf("    %-25s %-20s\n", "-B", "don't parse boolean (true or false)");
	printf("    %-25s %-20s\n", "-p", "pretty print");
	exit(1);
}

int
main(int argc, char **argv)
{
	int ch;
	uint8_t opt_array = 0;
	uint8_t opt_booloff = 0;
	uint8_t opt_pretty = 0;

	json_t *json = NULL;
	size_t json_dump_flags = JSON_COMPACT|JSON_ENSURE_ASCII|JSON_PRESERVE_ORDER;

	while ((ch = getopt(argc, argv, "aBp")) != -1) {
		switch (ch) {
		case 'a':
			opt_array = 1;
			break;
		case 'B':
			opt_booloff = 1;
			break;
		case 'p':
			opt_pretty = 1;
			break;
		default:
			usage_and_exit();
			break;
		}
	}

	json = parse_json_args(optind, argc, argv, opt_array, opt_booloff);
	if (json == NULL) {
		fprintf(stderr, "ERROR: failed to parse json\n");
		exit(1);
	}

	if (opt_pretty) {
		json_dump_flags |= JSON_INDENT(2);
	}
	if (json_dumpf(json, stdout, json_dump_flags) < 0) {
		fprintf(stderr, "ERROR: failed to dump json\n");
		exit(1);
	}

	json_decref(json);

	return 0;
}
