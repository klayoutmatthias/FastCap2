
#include "mulGlobal.h"
#include "mulStruct.h"
#include "electric.h"

/*
  converts the voltage vector entries corresponding to panels on dielectric
     interfaces into electric field boundary condition checks: 
     eps_outer*E_outer - eps_inner*E_inner
  this routine might be improved by fixing the way dummy, permittivity and h
     information is stored (more arrays and less pointer chasing)
  also - infinitesimally thin conductors on a dielectric i/f (surface type 
     BOTH) are not supported
*/
void compute_electric_fields(ssystem *sys, charge *chglist)
{
  charge *cp, *dummy;
  double flux_density, *panel_voltages, *panel_charges;
  Surface *surf;

  /* for each dielectric panel, do two divided differences to get the */
  /*    gradient of the potential in the normal and anti-normal directions */
  /* store the divided difference where the real panel's voltage was */
  /* zero the dummy panel voltage entries so that iterative loop will be OK */
  /* - the zeros can be skipped in the iterative loop calculations */
  panel_voltages = sys->p;
  panel_charges = sys->q;
  for(cp = chglist; cp != NULL; cp = cp->next) {
    if(cp->dummy) continue;

    if((surf = cp->surf)->type == DIELEC) {
      dummy = cp->pos_dummy;
      /* area field is divided difference step h for dummy panels */
      if (NUMDPT == 3) {
        flux_density = surf->outer_perm *
         (panel_voltages[dummy->index] - panel_voltages[cp->index])/dummy->area;
      } else {
        /* figure the electric field without the panel (cancellation error?)
           - positive dummy taken as positive side (E arrow head on that side)
           - this is a Gaussian equation (stat-coulombs, stat-volts) */
        /* (\epsilon_{1R} - \epsilon_{2R})E_{across panel} */
        flux_density = (surf->outer_perm - surf->inner_perm)
            *((panel_voltages[cp->pos_dummy->index]
               - panel_voltages[cp->neg_dummy->index])/(cp->pos_dummy->area
                                                        + cp->neg_dummy->area));
        /* - (\epsilon_{1R} +\epsilon_{2R}) 2\pi q/A */
        flux_density -= ((surf->inner_perm + surf->outer_perm)
                         *2*M_PI*panel_charges[cp->index]/cp->area);
      }

      if (sys->dmpele && NUMDPT == 3) {
        sys->msg(
                "Electric flux density evaluation at (%g %g %g), panel %d\n",
                cp->x, cp->y, cp->z, cp->index);
        sys->msg("  pos_dummy at (%g %g %g), potential = %g\n",
                dummy->x, dummy->y, dummy->z, panel_voltages[dummy->index]);
        sys->msg("  normal deriv on + side = %g(%g - %g)/%g = %g\n",
                surf->outer_perm,
                panel_voltages[dummy->index], panel_voltages[cp->index],
                dummy->area, flux_density);
      }

      panel_voltages[dummy->index] = 0.0;

      dummy = cp->neg_dummy;

      if (sys->dmpele && NUMDPT == 3) {
        sys->msg("  neg_dummy at (%g %g %g), potential = %g\n",
                dummy->x, dummy->y, dummy->z, panel_voltages[dummy->index]);
        sys->msg("  normal deriv on - side = %g(%g - %g)/%g = %g\n",
                surf->inner_perm,
                panel_voltages[cp->index], panel_voltages[dummy->index],
                dummy->area, surf->inner_perm *
         (panel_voltages[cp->index] - panel_voltages[dummy->index])/dummy->area);
      }

      /* area field is divided difference step h for dummy panels */
      if (NUMDPT == 3) {
        flux_density -= (surf->inner_perm *
         (panel_voltages[cp->index] - panel_voltages[dummy->index])/dummy->area);
      }
      panel_voltages[dummy->index] = 0.0;

      /* store the normal flux density difference */
      panel_voltages[cp->index] = flux_density;

      if (sys->dmpele && NUMDPT == 3) {
        sys->msg(
                "  flux density difference (pos side - neg side) = %g\n",
                flux_density);
      }
    }
  }
}
