
#if !defined(input_H)
#define input_H

#include <string>

struct ssystem;
struct charge;
struct Surface;
struct ITER;
struct Name;

void get_ps_file_base(ssystem *sys);
char *hack_path(char *str);

void populate_from_command_line(ssystem *sys);
charge *build_charge_list(ssystem *sys);
void read_list_file(ssystem *sys, Surface **surf_list, const char *list_file);

void dumpSurfDat(ssystem *sys);

#endif
