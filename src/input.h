
#if !defined(input_H)
#define input_H

#include <string>

struct ssystem;
struct charge;
struct ITER;
struct Name;

void get_ps_file_base(ssystem *sys);
char *hack_path(char *str);
int want_this_iter(ITER *iter_list, int iter_num);

void populate_from_command_line(ssystem *sys);
charge *build_charge_list(ssystem *sys);

#endif
