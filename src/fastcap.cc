
#include "mulStruct.h"
#include "fastcap_solve.h"
#include "input.h"

#include <stdexcept>

int main(int argc, char *argv[])
{
  try {

    ssystem sys;
    sys.argv = (const char **) argv;
    sys.argc = argc;
    sys.log = stdout;

    /* read the conductor and dielectric interface surface files, parse cmds */
    populate_from_command_line(&sys);

    /* produces the solution */
    solve(&sys);

    return 0;

  } catch (std::exception &ex) {
    fputs("ERROR: ", stderr);
    fputs(ex.what(), stderr);
    return -1;
  }
}
