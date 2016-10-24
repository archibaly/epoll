#ifndef _CONFIG_H_
#define _CONFIG_H_

/* example: */
/* name = "jacky liu" */
/* age = 25 */

int config_load(const char *filename);
int config_save(const char *filename);
void config_free(void);
void config_set_delim(char d);
char *config_get_value(const char *name);
int config_set_value(const char *name, const char *value);
void config_print_opt(const char *name);

#endif /* _CONFIG_H_ */
