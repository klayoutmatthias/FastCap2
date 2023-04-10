
#include "mulGlobal.h"
#include "mulStruct.h"
#include "quickif.h"
#include "heap.h"

#include <cstring>
#include <cctype>
#include <memory>

// --------------------------------------------------------------------------

SurfaceData::SurfaceData()
  : name(0), title(0), quads(0), tris(0)
{
}

SurfaceData *SurfaceData::clone(Heap &heap)
{
  SurfaceData *new_data = heap.alloc<SurfaceData>(1);
  new (new_data) SurfaceData();

  if (name) {
    new_data->name = heap.strdup(name);
  }
  if (title) {
    new_data->title = heap.strdup(title);
  }

  quadl *prevq = 0;
  for (quadl *q = quads; q; q = q->next) {
    quadl *new_quad = heap.alloc<quadl>(1);
    *new_quad = *q;
    new_quad->next = 0;
    if (prevq) {
      prevq->next = new_quad;
    } else {
      new_data->quads = new_quad;
    }
    prevq = new_quad;
  }

  tri *prevt = 0;
  for (tri *t = tris; t; t = t->next) {
    tri *new_tri = heap.alloc<tri>(1);
    *new_tri = *t;
    new_tri->next = 0;
    if (prevt) {
      prevt->next = new_tri;
    } else {
      new_data->tris = new_tri;
    }
    prevt = new_tri;
  }

  return new_data;
}

// --------------------------------------------------------------------------

/*
  makes 1st \n in a string = \0 and then deletes all trail/leading wh space
*/
static char *simplify(char *str)
{
  // skip leading white space
  while (*str && isspace(*str)) {
    ++str;
  }

  // erase trailing whitespace
  char *s = str + strlen(str);
  for ( ; s != str; --s) {
    if (!isspace(s[-1])) {
      break;
    }
  }
  *s = 0;

  return (str);
}

// --------------------------------------------------------------------------

#if defined(UNUSED)
/*
  tells if any conductor name alias matches a string only up to length(name)
*/
static int alias_match_name(Name *cur_name, char *name)
{
  Name *cur_alias;
  char name_frag[BUFSIZ];
  int nlen, j;

  nlen = strlen(name);

  for(cur_alias = cur_name->alias_list; cur_alias != NULL;
      cur_alias = cur_alias->next) {
    for(j = 0; j < nlen; j++) name_frag[j] = cur_alias->name[j];
    name_frag[j] = '\0';
    if(!strcmp(name, name_frag)) return(TRUE);
  }
  return(FALSE);
}
#endif

/*
  returns list of charge structs derived from quick input format:
  0 <Title string>       (must be first line, others in any order OK)
  Q <cond name string> X1 Y1 Z1 X2 Y2 Z2 X3 Y3 Z3 X4 Y4 Z4
  T <cond name string> X1 Y1 Z1 X2 Y2 Z2 X3 Y3 Z3
  N <cond name string> <Rename string>
  * <Comment string>
*/
charge *quickif(ssystem *sys, FILE *fp, const char *header, int surf_type, double *trans, const char *name_suffix, char **title)
/* char *name_suffix: suffix for all cond names read */
{
  quadl *fstquad = 0, *curquad = 0;
  tri *fsttri = 0, *curtri = 0;
  int linecnt = 2;
  char temp[BUFSIZ], temp2[BUFSIZ], line1[BUFSIZ];
  char condstr[BUFSIZ];
  double x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4;

  /* save the title, strip leading '0' */
  *title = simplify(sys->heap.strdup(header + 1));
  
  /* read in and load structs */
  while(fgets(line1, sizeof(line1), fp) != NULL) {
    if(line1[0] == 'Q' || line1[0] == 'q') {
      if(sscanf(line1, 
                "%s %s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                temp, condstr, 
                &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3, &x4, &y4, &z4)
         != 14) {
        sys->error("quickif: bad quad format, line %d:\n%s",
                   linecnt, line1);
      }

      /* add suffix */
      strcat(condstr, name_suffix);

      /* allocate quad struct */
      if(fstquad == NULL) {
        fstquad = sys->heap.alloc<quadl>(1, AMSC);
        curquad = fstquad;
      }
      else {
        curquad->next = sys->heap.alloc<quadl>(1, AMSC);
        curquad = curquad->next;
      }
      
      /* load quad struct */
      if(surf_type == CONDTR || surf_type == BOTH)
          curquad->cond = sys->get_conductor_number(condstr);
      else curquad->cond = 0;
      curquad->x1 = x1;
      curquad->x2 = x2;
      curquad->x3 = x3;
      curquad->x4 = x4;
      curquad->y1 = y1;
      curquad->y2 = y2;
      curquad->y3 = y3;
      curquad->y4 = y4;
      curquad->z1 = z1;
      curquad->z2 = z2;
      curquad->z3 = z3;
      curquad->z4 = z4;

      linecnt++;
    }
    else if(line1[0] == 'T' || line1[0] == 't') {
      if(sscanf(line1, "%s %s %lf %lf %lf %lf %lf %lf %lf %lf %lf",
                temp, condstr, &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3) 
         != 11) {
        sys->error("quickif: bad tri format, line %d:\n%s",
                   linecnt, line1);
      }

      /* allocate tri struct */
      if(fsttri == NULL) {
        fsttri = sys->heap.alloc<tri>(1, AMSC);
        curtri = fsttri;
      }
      else {
        curtri->next = sys->heap.alloc<tri>(1, AMSC);
        curtri = curtri->next;
      }

      /* add suffix */
      strcat(condstr, name_suffix);
      
      /* load tri struct */
      if(surf_type == CONDTR || surf_type == BOTH)
          curtri->cond = sys->get_conductor_number(condstr);
      else curquad->cond = 0;
      curtri->x1 = x1;
      curtri->x2 = x2;
      curtri->x3 = x3;
      curtri->y1 = y1;
      curtri->y2 = y2;
      curtri->y3 = y3;
      curtri->z1 = z1;
      curtri->z2 = z2;
      curtri->z3 = z3;

      linecnt++;
    }
    else if(line1[0] == 'N' || line1[0] == 'n') {
      if(sscanf(line1, "%s %s %s", temp, condstr, temp2) != 3) {
        sys->error("quickif: bad rename format, line %d:\n%s",
                   linecnt, line1);
      }

      if(surf_type != DIELEC) {
        /* rename only if surface is BOTH or CONDTR type */
        /* - eventually will be used when DIELEC surfs get names (someday) */

        /* add suffix */
        strcat(condstr, name_suffix);
        strcat(temp2, name_suffix);

        if(!sys->rename_conductor(condstr, temp2)) {
          sys->error("quickif: error renaming conductor");
        }
      }

      linecnt++;
    }
    else if(line1[0] == '%' || line1[0] == '*' ||
            line1[0] == '#') linecnt++; /* ignore comments */
    else {
      sys->error("quickif: bad line format, line %d:\n%s",
                 linecnt, line1);
    }
  }

  return quickif2charges(sys, fstquad, fsttri, trans, -1);
}

charge *quickif2charges(ssystem *sys, quadl *fstquad, tri *fsttri, double *trans, int cond_num)
{
  quadl *curquad = 0;
  tri *curtri = 0;
  charge *chglst = 0, *nq = 0;

  /* setup tris in charge structs */
  for (curtri = fsttri; curtri != NULL; curtri = curtri->next) {

    /* allocate charge struct to fill in */
    charge *c = sys->heap.alloc<charge>(1, AMSC);
    if (!chglst) {
      chglst = c;
    } else {
      nq->next = c;
    }
    nq = c;

    /* fill in corners */
    (nq->corner[0])[0] = curtri->x1;
    (nq->corner[0])[1] = curtri->y1;
    (nq->corner[0])[2] = curtri->z1;
    (nq->corner[1])[0] = curtri->x2;
    (nq->corner[1])[1] = curtri->y2;
    (nq->corner[1])[2] = curtri->z2;
    (nq->corner[2])[0] = curtri->x3;
    (nq->corner[2])[1] = curtri->y3;
    (nq->corner[2])[2] = curtri->z3;

    /* fill in remaining */
    nq->shape = 3;
    nq->cond = (cond_num >= 0 ? cond_num : curtri->cond);
  }

  /* setup quads in charge structs */
  for (curquad = fstquad; curquad != NULL; curquad = curquad->next) {

    /* allocate charge struct to fill in */
    charge *c = sys->heap.alloc<charge>(1, AMSC);
    if (!chglst) {
      chglst = c;
    } else {
      nq->next = c;
    }
    nq = c;

    /* fill in corners */
    (nq->corner[0])[0] = curquad->x1;
    (nq->corner[0])[1] = curquad->y1;
    (nq->corner[0])[2] = curquad->z1;
    (nq->corner[1])[0] = curquad->x2;
    (nq->corner[1])[1] = curquad->y2;
    (nq->corner[1])[2] = curquad->z2;
    (nq->corner[2])[0] = curquad->x3;
    (nq->corner[2])[1] = curquad->y3;
    (nq->corner[2])[2] = curquad->z3;
    (nq->corner[3])[0] = curquad->x4;
    (nq->corner[3])[1] = curquad->y4;
    (nq->corner[3])[2] = curquad->z4;

    /* fill in remaining */
    nq->shape = 4;
    nq->cond = (cond_num >= 0 ? cond_num : curquad->cond);
  }

  /* transform the corners */
  for (nq = chglst; nq; nq = nq->next) {
    for (int c = 0; c < nq->shape; ++c) {
      for (int i = 0; i < 3; ++i) {
        (nq->corner[c])[i] += trans[i];
      }
    }
  }

  return (chglst);
}

