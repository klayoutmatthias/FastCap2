
#include "mulStruct.h"
#include "fastcap_solve.h"
#include "zbuf2fastcap.h"
#include "input.h"

#include <stdexcept>
#include <cstring>

int main(int argc, char *argv[])
{
  try {

    ssystem sys;
    sys.argv = (const char **) argv;
    sys.argc = argc;
    sys.log = stdout;

    /* read the conductor and dielectric interface surface files, parse cmds */
    populate_from_command_line(&sys);

    if (sys.capvew && sys.m_ && sys.ps_file_base) {

      /* if no fastcap run is to be done, just dump the psfile */
      charge *chglist = build_charge_list(&sys);
      if (!chglist) {
        throw std::runtime_error("No surfaces present - cannot dump to PS");
      }

      std::string ps_file_name = std::string (sys.ps_file_base) + ".ps";
      dump_ps_geometry(&sys, ps_file_name.c_str (), chglist, NULL, sys.dd_);

    } else {

      /* produces the solution */
      fastcap_solve(&sys);

    }

    return 0;

  } catch (std::exception &ex) {
    fputs("ERROR: ", stderr);
    fputs(ex.what(), stderr);
    return -1;
  }
}
