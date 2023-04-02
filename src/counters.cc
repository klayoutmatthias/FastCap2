
#include "counters.h"

//  Counters singleton
Counters counters;

Counters::Counters()
{
  prectime = 0.0;
  prsetime = 0.0;
  conjtime = 0.0;
  dirtime = 0.0;
  multime = 0.0;
  uptime = 0.0;
  downtime = 0.0;
  evaltime = 0.0;
  fulldirops = 0;
  lutime = 0.0;
  fullsoltime = 0.0;
  fullPqops = 0;
}
