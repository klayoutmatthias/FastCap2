
#include "mulGlobal.h"
#include "mulDisplay.h"
#include "mulStruct.h"
#include "zbufGlobal.h"
#include "input.h"
#include "calcp.h"
#include "patran.h"
#include "patran_f.h"
#include "quickif.h"

#include <cstdio>
#include <cstring>
#include <unistd.h>

/*
  reads an input file list file (a list of dielectric i/f and conductor 
    surface files with permittivities)
  returns linked list of file pointers and permittivities in surface structs
  surface list file is specified on the command line with `-l<filename>'
  each line in the list file specifies a surface filename and its permittivites
  if a list file line has the filename string `stdin' then stdin will be
    read (note that more than one `stdin' is not allowed)

  list file line formats
  conductor surface:
  C <filename> <outer rel permittivity> <tx> <ty> <tz> [+]

  dielectric surface:
  D <file> <outer rel perm> <inner rel perm> <tx> <ty> <tz> <rx> <ry> <rz> [-]

  thin conductor on dielectric boundary (B => "both"):
  B <file> <outer rel perm> <inner rel perm> <tx> <ty> <tz> <rx> <ry> <rz> [+-]

  group name specification line:
  G <group name>

  comment line:
  * <comment line>

  the <tx> <ty> <tz> are 3 components of a translation vector that is applied
    to all the panels in the file
  the <rx> <ry> <rz> specify a reference point on the outside of the 
    interface surface (all surface normals should point towards the point)
    the optional `-' indicates that the reference point is inside
    the surface (all surface normals should point away from the point)
    the reference point is used to figure which permittivity is on which side
    for type B surfaces, there must be no space between the `+' and `-' if
      both are used - note that D surfaces must never have a `+'
    since the reference point must be on one side, each file must contain
      a convex surface (ie any surface must be broken into such subsurfaces)
  the optional `+' indicates that the panels in the next conductor line file
    should be grouped together for the purposes of renumbering
    - if two files have two distinct conductors but both sets of panels
      are numbered `1' inside the two files then use something like
      C first_file 1.5 <tx> <ty> <tz>
      C second_file 1.5 <tx> <ty> <tz>
    - on the other hand, if parts of the same conductor are split into
      two files (say because the second part borders a different dielectric)
      then use something like
      C first_file 1.5 <tx> <ty> <tz> +
      C second_file 2.0 <tx> <ty> <tz>
      in this case it is up to the user to make sure first_file's panels
      and second_file's panels all have the same conductor number
    - to disable the renumbering entirely, use the `+' on all the 
      conductor lines:
      C first_file 3.0 <tx> <ty> <tz> +
      C second_file 4.0 <tx> <ty> <tz> +
      C last_file 3.0 <tx> <ty> <tz> +
    - files grouped together with the + option have their conductor names
      appended with the string ` (GROUP<number>)'
      - for example, the conductor name `BIT_LINE' shows up as
        `BIT_LINE (GROUP3)' if it is in the third group
      - a string other than `GROUP<number>' may be specified for the
        group name using G line `G <group name>' just before the group to
        be renamed; this is helpful when idenifying conductors to omit
        from capacitance calculations using the -k option
*/
static void read_list_file(ssystem *sys, surface **surf_list, int *num_surf, const char *list_file, int read_from_stdin)
{
  int linecnt, end_of_chain, ref_pnt_is_inside, group_cnt;
  FILE *fp;
  char tline[BUFSIZ], file_name[BUFSIZ], plus[BUFSIZ], group_name[BUFSIZ];
  double outer_perm, inner_perm, tx, ty, tz, rx, ry, rz;
  surface *cur_surf = 0;

  /* find the end of the current surface list */
  if(*surf_list != NULL) {
    for(cur_surf = *surf_list; cur_surf->next != NULL; 
        cur_surf = cur_surf->next);
  }
  
  /* attempt to open file list file */
  if((fp = fopen(list_file, "r")) == NULL) {
    sys->error("read_list_file: can't open list file\n  `%s'\nto read\n",
               list_file);
  }

  /* read file names and permittivities, build linked list */
  linecnt = 0;
  group_cnt = read_from_stdin + 1;
  sprintf(group_name, "GROUP%d", group_cnt);
  while(fgets(tline, sizeof(tline), fp) != NULL) {
    linecnt++;
    if(tline[0] == 'C' || tline[0] == 'c') {
      if(sscanf(&(tline[1]), "%s %lf %lf %lf %lf", 
                file_name, &outer_perm, &tx, &ty, &tz) != 5) {
        sys->error("read_list_file: bad conductor surface format, tline %d:\n%s\n",
                   linecnt, tline);
      }

      /* check if end of chain of surfaces with same conductor numbers */
      end_of_chain = TRUE;
      if(sscanf(&(tline[1]), "%s %lf %lf %lf %lf %s", 
                file_name, &outer_perm, &tx, &ty, &tz, plus) == 6) {
        if(!strcmp(plus, "+")) end_of_chain = FALSE;
      }

      /* allocate and load surface struct */
      if(*surf_list == NULL) {
        *surf_list = sys->heap.alloc<surface>(1, AMSC);
        cur_surf = *surf_list;
      }
      else {
        cur_surf->next = sys->heap.alloc<surface>(1, AMSC);
        cur_surf->next->prev = cur_surf;
        cur_surf = cur_surf->next;
      }
      
      cur_surf->type = CONDTR;
      cur_surf->trans[0] = tx;
      cur_surf->trans[1] = ty;
      cur_surf->trans[2] = tz;
      cur_surf->end_of_chain = end_of_chain;
      cur_surf->name = sys->heap.strdup(file_name);
      cur_surf->outer_perm = outer_perm;

      /* set up group name */
      cur_surf->group_name = sys->heap.strdup(group_name);

      /* update group name if end of chain */
      if(end_of_chain) {
        sprintf(group_name, "GROUP%d", ++group_cnt);
      }

      (*num_surf)++;
    }
    else if(tline[0] == 'B' || tline[0] == 'b') {
      if(sscanf(&(tline[1]), "%s %lf %lf %lf %lf %lf %lf %lf %lf", 
                file_name, &outer_perm, &inner_perm, &tx, &ty, &tz,
                &rx, &ry, &rz) != 9) {
        sys->error("read_list_file: bad thin conductor on dielectric interface surface format, line %d:\n%s\n",
                   linecnt, tline);
      }

      /* check if end of chain of surfaces with same conductor numbers */
      end_of_chain = TRUE;
      ref_pnt_is_inside = FALSE;
      if(sscanf(&(tline[1]), "%s %lf %lf %lf %lf %lf %lf %lf %lf %s", 
                file_name, &outer_perm, &inner_perm, &tx, &ty, &tz, 
                &rx, &ry, &rz, plus) 
         == 10) {
        if(!strcmp(plus, "+")) end_of_chain = FALSE;
        if(!strcmp(plus, "+-") || !strcmp(plus, "-+")) {
          end_of_chain = FALSE;
          ref_pnt_is_inside = TRUE;
        }
        if(!strcmp(plus, "-")) ref_pnt_is_inside = TRUE;
      }

      /* allocate and load surface struct */
      if(*surf_list == NULL) {
        *surf_list = sys->heap.alloc<surface>(1, AMSC);
        cur_surf = *surf_list;
      }
      else {
        cur_surf->next = sys->heap.alloc<surface>(1, AMSC);
        cur_surf->next->prev = cur_surf;
        cur_surf = cur_surf->next;
      }
      
      cur_surf->type = BOTH;
      cur_surf->trans[0] = tx;
      cur_surf->trans[1] = ty;
      cur_surf->trans[2] = tz;
      cur_surf->ref[0] = rx;
      cur_surf->ref[1] = ry;
      cur_surf->ref[2] = rz;
      cur_surf->ref_inside = ref_pnt_is_inside;
      cur_surf->end_of_chain = end_of_chain;
      cur_surf->name = sys->heap.strdup(file_name);
      cur_surf->outer_perm = outer_perm;
      cur_surf->inner_perm = inner_perm;

      /* set up group name */
      cur_surf->group_name = sys->heap.strdup(group_name);

      /* update group name if end of chain */
      if(end_of_chain) {
        sprintf(group_name, "GROUP%d", ++group_cnt);
      }

      (*num_surf)++;
    }
    else if(tline[0] == 'D' || tline[0] == 'd') {
      if(sscanf(&(tline[1]), "%s %lf %lf %lf %lf %lf %lf %lf %lf", 
                file_name, &outer_perm, &inner_perm, &tx, &ty, &tz,
                &rx, &ry, &rz) != 9) {
        sys->error("read_list_file: bad dielectric interface surface format, line %d:\n%s\n",
                   linecnt, tline);
      }

      /* check to see if reference point is negative side of surface */
      ref_pnt_is_inside = FALSE;
      if(sscanf(&(tline[1]), "%s %lf %lf %lf %lf %lf %lf %lf %lf %s", 
                file_name, &outer_perm, &inner_perm, &tx, &ty, &tz, 
                &rx, &ry, &rz, plus) 
         == 10) {
        if(!strcmp(plus, "-")) ref_pnt_is_inside = TRUE;
      }

      /* allocate and load surface struct */
      if(*surf_list == NULL) {
        *surf_list = sys->heap.alloc<surface>(1, AMSC);
        cur_surf = *surf_list;
      }
      else {
        cur_surf->next = sys->heap.alloc<surface>(1, AMSC);
        cur_surf->next->prev = cur_surf;
        cur_surf = cur_surf->next;
      }
      
      cur_surf->type = DIELEC;
      cur_surf->trans[0] = tx;
      cur_surf->trans[1] = ty;
      cur_surf->trans[2] = tz;
      cur_surf->ref[0] = rx;
      cur_surf->ref[1] = ry;
      cur_surf->ref[2] = rz;
      cur_surf->ref_inside = ref_pnt_is_inside;
      cur_surf->end_of_chain = TRUE;
      cur_surf->name = sys->heap.strdup(file_name);
      cur_surf->outer_perm = outer_perm;
      cur_surf->inner_perm = inner_perm;

      /* set up group name */
      cur_surf->group_name = sys->heap.strdup(group_name);

      /* update group name (DIELEC surface is always end of chain) */
      sprintf(group_name, "GROUP%d", ++group_cnt);

      (*num_surf)++;
    }
    else if(tline[0] == 'G' || tline[0] == 'g') {
      if(sscanf(&(tline[1]), "%s", group_name) != 1) {
        sys->error("read_list_file: bad group name format, line %d:\n%s\n",
                   linecnt, tline);
      }
    }
    else if(tline[0] == '%' || tline[0] == '*' ||
            tline[0] == '#'); /* ignore comments */
    else {
      sys->error("read_list_file: bad line format, line %d:\n%s\n",
                 linecnt, tline);
    }
  }
  fclose(fp);

}

/*
  add dummy panel structs to the panel list for electric field evaluation
  - assumes its handed a list of DIELEC or BOTH type panels
*/
static void add_dummy_panels(ssystem *sys, charge *panel_list)
{
  double h;
  charge *dummy_list = NULL;
  charge *cur_panel, *cur_dummy;

  for(cur_panel = panel_list; cur_panel != NULL; cur_panel = cur_panel->next) {
    cur_panel->dummy = FALSE;

    /* make 2 dummy panels for evaluation points needed to do div difference */
    /* make the first */
    if(dummy_list == NULL) {
      dummy_list = sys->heap.alloc<charge>(1, AMSC);
      cur_dummy = dummy_list;
    }
    else {
      cur_dummy->next = sys->heap.alloc<charge>(1, AMSC);
      cur_dummy = cur_dummy->next;
    }

    cur_dummy->dummy = TRUE;
    h = HPOS;
    cur_dummy->x = cur_panel->x + cur_panel->Z[0]*h;
    cur_dummy->y = cur_panel->y + cur_panel->Z[1]*h;
    cur_dummy->z = cur_panel->z + cur_panel->Z[2]*h;
    /* note ABUSE OF area field - used to store div dif distance */
    cur_dummy->area = h;

    cur_panel->pos_dummy = cur_dummy; /* link dummy to its real panel */

    /* make the second dummy struct */
    cur_dummy->next = sys->heap.alloc<charge>(1, AMSC);
    cur_dummy = cur_dummy->next;

    cur_dummy->dummy = TRUE;
    h = HNEG;
    cur_dummy->x = cur_panel->x - cur_panel->Z[0]*h;
    cur_dummy->y = cur_panel->y - cur_panel->Z[1]*h;
    cur_dummy->z = cur_panel->z - cur_panel->Z[2]*h;
    /* note ABUSE OF area field - used to store div dif distance */
    cur_dummy->area = h;

    cur_panel->neg_dummy = cur_dummy; /* link dummy to its real panel */
  }

  /* put the dummies in the list */
  for(cur_panel = panel_list; cur_panel->next != NULL; 
      cur_panel = cur_panel->next);
  cur_panel->next = dummy_list;
  
}

/* returns a pointer to a file name w/o the path (if present) */
char *hack_path(char *str)
{
  int i;
  int last_slash;

  for(i = last_slash = 0; str[i] != '\0'; i++) {
    if(str[i] == '/') last_slash = i;
  }

  if(str[last_slash] == '/') return(&(str[last_slash+1]));
  else return(str);
}

#if defined(UNUSED)
/*
  reassigns conductor numbers to a list of panels so that they'll
    be numbered contiguously from 1
  - also changes conductor numbers associated with conductor name structs
  - dummy panels are skipped
  - dielectric panels, with conductor number 0, are also skipped
*/
static void reassign_cond_numbers(ssystem *sys, charge *panel_list, NAME *name_list, char * /*surf_name*/)
{
  int i, cond_nums[MAXCON], num_cond, cond_num_found;
  charge *cur_panel;
  NAME *cur_name;

  /* get the conductor numbers currently being used */
  num_cond = 0;
  for(cur_panel = panel_list; cur_panel != NULL; cur_panel = cur_panel->next) {
    if(cur_panel->dummy || cur_panel->cond == 0) continue;

    cond_num_found = FALSE;
    for(i = 0; i < num_cond; i++) {
      if(cur_panel->cond == cond_nums[i]) {
        cond_num_found = TRUE;
        break;
      }
    }
    if(!cond_num_found) cond_nums[num_cond++] = cur_panel->cond;
  }

  /* rewrite all the conductor numbers to be their position in the array */
  for(cur_panel = panel_list; cur_panel != NULL; cur_panel = cur_panel->next) {
    if(cur_panel->dummy || cur_panel->cond == 0) continue;

    for(i = 0; i < num_cond && cur_panel->cond != cond_nums[i]; i++);
    if(i == num_cond) {
      sys->error("reassign_cond_numbers: cant find conductor number that must exist\n");
    }
    cur_panel->cond = i+1;
  }

  /* do the same for the name structs */
  for(cur_name = name_list; cur_name != NULL; cur_name = cur_name->next) {
    for(i = 0; 
        i < num_cond && cur_name->patch_list->conductor_ID != cond_nums[i]; 
        i++);
    if(i == num_cond) {
      sys->error("reassign_cond_numbers: cant find conductor number in name list\n");
    }
  }
}
#endif

#if defined(UNUSED)
/*
  negates all the conductor numbers - used to make a panel list's conds unique
    just before renumbering
*/
static void negate_cond_numbers(charge *panel_list, NAME *name_list)
{
  charge *cur_panel;
  NAME *cur_name;

  for(cur_panel = panel_list; cur_panel != NULL; cur_panel = cur_panel->next) {
    if(cur_panel->dummy) continue;

    cur_panel->cond = -cur_panel->cond;
  }

  for(cur_name = name_list; cur_name != NULL; cur_name = cur_name->next) {
    cur_name->patch_list->conductor_ID = -cur_name->patch_list->conductor_ID;
  }
}
#endif

#if defined(UNUSED)
/*
  for debug - dumps the iter list
*/
static int dump_ilist(ssystem *sys)
{
  ITER *cur_iter;

  /* check the list for the iter number passed in */
  sys->msg("Iter list:");
  for(cur_iter = sys->qpic_num_list; cur_iter != NULL; cur_iter = cur_iter->next) {
    sys->msg("%d ", cur_iter->iter);
  }
  sys->msg("\n");
  return(TRUE);
}
#endif

/*
  checks if a particular iter is in the list; returns TRUE if it is
*/
int want_this_iter(ITER *iter_list, int iter_num)
{
  ITER *cur_iter;

  for(cur_iter = iter_list; cur_iter != NULL; cur_iter = cur_iter->next) {
    if(cur_iter->iter == iter_num) {
      return(TRUE);
    }
  }

  return(FALSE);
}
/*
  sets up the ps file base string
*/
void get_ps_file_base(ssystem *sys)
{
  const char **argv = sys->argv;
  int argc = sys->argc;

  int i, j;
  char temp[BUFSIZ];

  /* - if no list file, use input file; otherwise use list file */
  /* - if neither present, use "stdin" */
  /*   check for list file */
  for(i = 1; i < argc; i++) {
    if(argv[i][0] == '-' && argv[i][1] == 'l') {
      strcpy(temp, &(argv[i][2]));
      /* go to end of string, walk back to first period */
      for(j = 0; temp[j] != '\0'; j++);
      for(; temp[j] != '.' && j >= 0; j--);
      if(temp[j] == '.') temp[j] = '\0';
      /* save list file base */
      sys->ps_file_base = sys->heap.strdup(hack_path(temp));
      break;
    }
    else if(argv[i][0] != '-') { /* not an option, must be input file */
      strcpy(temp, argv[i]);
      for(j = 0; temp[j] != '\0' && temp[j] != '.'; j++);
      temp[j] = '\0';
      /* save list file base */
      sys->ps_file_base = sys->heap.strdup(hack_path(temp));
      break;
    }
  }

  if(sys->ps_file_base == NULL) {       /* input must be stdin */
    sys->ps_file_base = sys->heap.strdup("stdin");
  }
}

/*
  open all the surface files and return a charge (panel) struct list
  set up pointers from each panel to its corresponding surface struct
  align the normals of all the panels in each surface so they point
    towards the same side as where the ref point is (dielectric files only)
*/
static charge *read_panels(ssystem *sys, surface *surf_list, int *num_cond)
{
  int patran_file, num_panels, stdin_read, num_dummies, num_quads, num_tris;
  charge *panel_list = NULL, *cur_panel, *c_panel;
  surface *cur_surf;
  FILE *fp;
  char surf_name[BUFSIZ];
  int patran_file_read;

  stdin_read = FALSE;
  for(cur_surf = surf_list; cur_surf != NULL; cur_surf = cur_surf->next) {
    if(!strcmp(cur_surf->name, "stdin")) {
      if(stdin_read) {
        sys->error("read_panels: attempt to read stdin twice\n");
      }
      else {
        stdin_read = TRUE;
        fp = stdin;
      }
    }
    else if((fp = fopen(cur_surf->name, "r")) == NULL) {
      sys->error("read_panels: can't open\n  `%s'\nto read\n",
                 cur_surf->name);
    }

    /* input the panel list */
    if(panel_list == NULL) {
      /* group names are set up in read_list_file() */
      sprintf(surf_name, "%%%s", cur_surf->group_name);
      panel_list = cur_panel 
          = patfront(sys, fp, &patran_file, cur_surf->type, cur_surf->trans, num_cond, surf_name);
      patran_file_read = patran_file;
    }
    else {
      if(cur_surf->prev->end_of_chain) {
        sprintf(surf_name, "%%%s", cur_surf->group_name);
        patran_file_read = FALSE;
      }
      cur_panel->next 
          = patfront(sys, fp, &patran_file, cur_surf->type, cur_surf->trans,
                     num_cond, surf_name);
      if(!patran_file && patran_file_read) {
        sys->error("read_panels: generic format file\n  `%s'\nread after neutral file(s) in same group---reorder list file entries\n", cur_surf->name);
      }
      patran_file_read = patran_file;
      cur_panel = cur_panel->next;
    }
    if(strcmp(cur_surf->name, "stdin") != 0) fclose(fp);

    cur_surf->panels = cur_panel;

    /* save the surface file's title */
    cur_surf->title = sys->heap.strdup(sys->title ? sys->title : "");
    sys->title = 0;             /* not sure if needed */

    /* if the surface is a DIELEC, make sure all conductor numbers are zero */
    /* - also link each panel to its surface */
    for(c_panel = cur_panel; c_panel != NULL; c_panel = c_panel->next) {
      if(cur_surf->type == DIELEC) c_panel->cond = 0;
      c_panel->surf = cur_surf;
    }
    
    /* align the normals and add dummy structs if dielec i/f */
    initcalcp(sys, cur_surf->panels);/* get normals, edges, perpendiculars */
    if(cur_surf->type == DIELEC || cur_surf->type == BOTH) {
      /* if(patran_file) align_normals(cur_surf->panels);
      align_normals(cur_surf->panels, cur_surf); */ /* now done in calcp */
      add_dummy_panels(sys, cur_surf->panels); /* add dummy panels for field calc */
    }

    /* make cur_panel = last panel in list, count panels */
    num_panels = num_dummies = num_tris = num_quads = 0;
    for(cur_panel = cur_surf->panels ; ; cur_panel = cur_panel->next) {
      num_panels++;
      if(cur_panel->dummy) num_dummies++;
      else if(cur_panel->shape == 3) num_tris++;
      else if(cur_panel->shape == 4) num_quads++;
      else {
        sys->error("read_panels: bad panel shape, %d\n",
                   cur_panel->shape);
      }
      if(cur_panel->next == NULL) break;
    }

    /*sys->msg("Surface %s has %d quads and %d tris\n",
            cur_surf->name, num_quads, num_tris);*/

    cur_surf->num_panels = num_panels;
    cur_surf->num_dummies = num_dummies;

  }
  return(panel_list);
}

/*
  returns either conductor number or one of two error codes
  NOTUNI => no group name given and name by itself is not unique
  NOTFND => neither name by itself nor with group name is not in list
  - any unique leading part of the name%group_name string may be specified 
*/
static int getUniqueCondNum(char *name, Name *name_list)
{
  int nlen, cond;
  char name_frag[BUFSIZ], *cur_alias;
  Name *cur_name;
  int i, j, times_in_list;

  nlen = strlen(name);
  times_in_list = 0;

  /* fish through name list for name---check first nlen chars for match */
  for(cur_name = name_list, i = 1; cur_name != NULL && times_in_list < 2;
      cur_name = cur_name->next, i++) {
    cur_alias = last_alias(cur_name);
    for(j = 0; j < nlen; j++) name_frag[j] = cur_alias[j];
    name_frag[j] = '\0';
    if(!strcmp(name_frag, name)) {
      times_in_list++;  /* increment times name in list count */
      cond = i;
    }
  }

  /* name can't be dealt with; return appropriate error code */
  if(times_in_list > 2) return(NOTUNI);
  else if(times_in_list == 1) return(cond);
  else return(NOTFND);
}


/*
  called after all conductor names have been resolved to get list
    of conductor numbers that whose columns will not be calculated
  parses the conductor kill list spec from command line arg saved before
  (conds that get no solve); puts result in kill_num_list
  list string format: 
  [<cond name>[%<group name>]],[<cond name>[%<group name>]]...
  - no spaces; group name may be omitted if conductor name is unique
    across all groups
  - conductor names can't have any %'s
  - redundant names are detected as errors
*/
static ITER *get_kill_num_list(ssystem *sys, Name *name_list, const char *kill_name_list)
{
  int i, j, start_token, end_token, cond;
  char name[BUFSIZ];
  ITER *kill_num_list;
  ITER *cur_cond;

  /* check for no name list given */
  if(kill_name_list == NULL) return(NULL);

  start_token = 0;
  kill_num_list = NULL;
  while(kill_name_list[start_token] != '\0') {

    /* loop until next comma or end of list */
    for(i = start_token; kill_name_list[i] != '\0' && kill_name_list[i] != ',';
        i++);
    end_token = i;

    /* extract the name%group_name string */
    /*   copy the name */
    for(i = start_token, j = 0; i < end_token; i++, j++) 
        name[j] = kill_name_list[i];
    name[j] = '\0';

    /* attempt to get conductor number from name and group_name */
    cond = getUniqueCondNum(name, name_list);
    if(cond == NOTUNI) {
      sys->error("get_kill_num_list: cannot find unique conductor name starting `%s'\n",
                 name);
    }
    else if(cond == NOTFND) {
      sys->error("get_kill_num_list: cannot find conductor name starting `%s'\n",
                 name);
    }

    /* add conductor name to list of conductors to omit */
    if(kill_num_list == NULL) {
      kill_num_list = sys->heap.alloc<ITER>(1, AMSC);
      cur_cond = kill_num_list;
    }
    else {
      cur_cond->next = sys->heap.alloc<ITER>(1, AMSC);
      cur_cond = cur_cond->next;
    }
    cur_cond->iter = cond;
      
    if(kill_name_list[end_token] == ',') start_token = end_token+1;
    else start_token = end_token;
  }
  return(kill_num_list);
}

/*
  command line parsing routine
*/
static void parse_command_line(ssystem *sys, const char **input_file, const char **surf_list_file, int *read_from_stdin)
{
  const char **argv = sys->argv;
  int argc = sys->argc;

  int cmderr, i;
  char **chkp, *chk;

  /* load default parameters */
  sys->azimuth = DEFAZM;             /* azimuth */
  sys->elevation = DEFELE;           /* elevation */
  sys->rotation = DEFROT;            /* rotation relative to image of z axis */
  sys->distance = DEFDST;            /* distance to view pnt = (1+distance)radius */
  sys->moffset[0] = OFFSETX;         /* puts the origin this dist from lower left */
  sys->moffset[1] = OFFSETY;
  sys->scale = DEFSCL;               /* master scaling - applied to 2d image */
  sys->linewd = DEFWID;              /* line width used in ps file */
  sys->axeslen = DEFAXE;             /* length of axes lines in 3d */
  sys->up_axis = DEFUAX;             /* upward-pointing axis in 2d image */
  sys->line_file = NULL;             /* file of lines/arrows in .fig format */
  sys->qpic_num_list = NULL;            /* list of cond nums to get shaded plots for */
  sys->qpic_name_list = NULL;   /* list of cond names to get shaded plots */
  sys->kq_num_list = NULL;              /* list of cond nums in shaded plots */
  sys->kq_name_list = NULL;             /* list of cond names in shaded plots */
  sys->s_ = sys->n_ = sys->g_ = sys->c_ = sys->x_ = sys->k_ = false;
  sys->rc_ = sys->rd_ = sys->rb_ = sys->q_ = sys->rk_ = sys->m_ = false;
  sys->f_ = sys->dd_ = false;

  sys->iter_tol = ABSTOL;
  sys->kill_num_list = sys->kinp_num_list = NULL;
  sys->kill_name_list = sys->kinp_name_list = NULL;
  cmderr = FALSE;
  chkp = &chk;                  /* pointers for error checking */

  for(i = 1; i < argc && cmderr == FALSE; i++) {
    if(argv[i][0] == '-') {
      if(argv[i][1] == 'o') {
        sys->order = (int) strtol(&(argv[i][2]), chkp, 10);
        if(*chkp == &(argv[i][2]) || sys->order < 0) {
          sys->info("%s: bad expansion order `%s'\n",
                  argv[0], &argv[i][2]);
          cmderr = TRUE;
          break;
        }
      }
      else if(argv[i][1] == 'd' && argv[i][2] == 'c') {
        sys->dd_ = true;
      }
      else if(argv[i][1] == 'd') {
        sys->depth = (int) strtol(&(argv[i][2]), chkp, 10);
        if(*chkp == &(argv[i][2]) || sys->depth < 0) {
          sys->info("%s: bad partitioning depth `%s'\n",
                  argv[0], &argv[i][2]);
          cmderr = TRUE;
          break;
        }
      }
      else if(argv[i][1] == 'p') {
        if(sscanf(&(argv[i][2]), "%lf", &sys->perm_factor) != 1) cmderr = TRUE;
        else if(sys->perm_factor <= 0.0) cmderr = TRUE;
        if(cmderr) {
          sys->info("%s: bad permittivity `%s'\n", argv[0], &argv[i][2]);
          break;
        }
      }
      else if(argv[i][1] == 'l') {
        *surf_list_file = &(argv[i][2]);
      }
      else if(argv[i][1] == 'r' && argv[i][2] == 's') {
        sys->kill_name_list = &(argv[i][3]);
      }
      else if(argv[i][1] == 'r' && argv[i][2] == 'i') {
        sys->kinp_name_list = &(argv[i][3]);
      }
      else if(argv[i][1] == '\0') {
        *read_from_stdin = TRUE;
      }
      else if(argv[i][1] == 'f') {
        sys->f_ = true;
      }
      else if(argv[i][1] == 'b') {
        sys->line_file = &(argv[i][2]);
      }
      else if(argv[i][1] == 'a') {
        if(sscanf(&(argv[i][2]), "%lf", &sys->azimuth) != 1) {
          sys->info("%s: bad view point azimuth angle '%s'\n",
                  argv[0], &argv[i][2]);
          cmderr = TRUE;
          break;
        }
      }
      else if(argv[i][1] == 'e') {
        if(sscanf(&(argv[i][2]), "%lf", &sys->elevation) != 1) {
          sys->info("%s: bad view point elevation angle '%s'\n",
                  argv[0], &argv[i][2]);
          cmderr = TRUE;
          break;
        }
      }
      else if(argv[i][1] == 't') {
        if(sscanf(&(argv[i][2]), "%lf", &sys->iter_tol) != 1 || sys->iter_tol <= 0.0) {
          sys->info("%s: bad iteration tolerence '%s'\n",
                  argv[0], &argv[i][2]);
          cmderr = TRUE;
          break;
        }
      }
      else if(argv[i][1] == 'r' && argv[i][2] == 'c') {
        sys->kq_name_list = &(argv[i][3]);
        sys->rc_ = true;
      }
      else if(!strcmp(&(argv[i][1]), "rd")) sys->rd_ = true;
      else if(!strcmp(&(argv[i][1]), "rk")) sys->rk_ = true;
      else if(argv[i][1] == 'r') {
        if(sscanf(&(argv[i][2]), "%lf", &sys->rotation) != 1) {
          sys->info("%s: bad image rotation angle '%s'\n",
                  argv[0], &argv[i][2]);
          cmderr = TRUE;
          break;
        }
      }
      else if(argv[i][1] == 'h') {
        if(sscanf(&(argv[i][2]), "%lf", &sys->distance) != 1) cmderr = TRUE;
        else if(sys->distance <= 0.0) cmderr = TRUE;
        if(cmderr) {
          sys->info("%s: bad view point distance '%s'\n",
                  argv[0], &argv[i][2]);
          break;
        }
      }
      else if(argv[i][1] == 's') {
        if(sscanf(&(argv[i][2]), "%lf", &sys->scale) != 1) cmderr = TRUE;
        else if(sys->scale <= 0.0) cmderr = TRUE;
        if(cmderr) {
          sys->info("%s: bad image scale factor '%s'\n",
                  argv[0], &argv[i][2]);
          break;
        }
      }
      else if(argv[i][1] == 'w') {
        if(sscanf(&(argv[i][2]), "%lf", &sys->linewd) != 1) {
                                /* no check for < 0 so dash (-1) is pos. */
          sys->info("%s: bad line width '%s'\n",
                  argv[0], &argv[i][2]);
          cmderr = TRUE;
          break;
        }
      }
      /* -x sets up axes of default length, -x<len> uses len as length */
      else if(argv[i][1] == 'x') {
        if(argv[i][2] == '\0') sys->x_ = true;
        else {
          if(sscanf(&(argv[i][2]), "%lf", &sys->axeslen) != 1) {
                                /* no check for < 0 so axes can flip */
            sys->info("%s: bad axes length '%s'\n",
                    argv[0], &argv[i][2]);
            cmderr = TRUE;
            break;
          }
          else sys->x_ = true;
        }
      }
      else if(argv[i][1] == 'v') sys->s_ = true;
      else if(argv[i][1] == 'n') sys->n_ = true;
      else if(argv[i][1] == 'g') sys->g_ = true;
      else if(argv[i][1] == 'c') sys->c_ = true;
      else if(argv[i][1] == 'm') sys->m_ = true;
      else if(argv[i][1] == 'q') {
        get_ps_file_base(sys); /* set up the output file base */
        sys->qpic_name_list = &(argv[i][2]);
        sys->q_ = true;
      }
      else if(argv[i][1] == 'u') {
        if(!strcmp(&(argv[i][2]), "x") || !strcmp(&(argv[i][2]), "X"))
            sys->up_axis = XI;
        else if(!strcmp(&(argv[i][2]), "y") || !strcmp(&(argv[i][2]), "Y"))
            sys->up_axis = YI;
        else if(!strcmp(&(argv[i][2]), "z") || !strcmp(&(argv[i][2]), "Z"))
            sys->up_axis = ZI;
        else {
          sys->info("%s: bad up axis type `%s' -- use x, y or z\n", argv[0], &(argv[i][2]));
          cmderr = TRUE;
          break;
        }
      }
      else {
        sys->info("%s: illegal option -- %s\n", argv[0], &(argv[i][1]));
        cmderr = TRUE;
        break;
      }
    }
    else {                      /* isn't an option, must be the input file */
      *input_file = argv[i];
    }
  }

  if (cmderr == TRUE) {
    if (sys->capvew) {
      sys->info(
              "Usage: '%s [-o<expansion order>] [-d<partitioning depth>] [<input file>]\n                [-p<permittivity factor>] [-rs<cond list>] [-ri<cond list>]\n                [-] [-l<list file>] [-t<iter tol>] [-a<azimuth>] [-e<elevation>]\n                [-r<rotation>] [-h<distance>] [-s<scale>] [-w<linewidth>]\n                [-u<upaxis>] [-q<cond list>] [-rc<cond list>] [-x<axeslength>]\n                [-b<.figfile>] [-m] [-rk] [-rd] [-dc] [-c] [-v] [-n] [-f] [-g]\n", argv[0]);
      sys->info("DEFAULT VALUES:\n");
      sys->info("  expansion order = %d\n", DEFORD);
      sys->info("  partitioning depth = set automatically\n");
      sys->info("  permittivity factor = 1.0\n");
      sys->info("  iterative loop ||r|| tolerance = %g\n", ABSTOL);
      sys->info("  azimuth = %g\n  elevation = %g\n  rotation = %g\n",
              DEFAZM, DEFELE, DEFROT);
      sys->info(
              "  distance = %g (0 => 1 object radius away from center)\n",
              DEFDST);
      sys->info("  scale = %g\n  linewidth = %g\n",
              DEFSCL, DEFWID);
      if(DEFUAX == XI) sys->info("  upaxis = x\n");
      else if(DEFUAX == YI) sys->info("  upaxis = y\n");
      else if(DEFUAX == ZI) sys->info("  upaxis = z\n");
      sys->info("  axeslength = %g\n", DEFAXE);
      sys->info("OPTIONS:\n");
      sys->info("  -   = force conductor surface file read from stdin\n");
      sys->info("  -rs = remove conductors from solve list\n");
      sys->info("  -ri = remove conductors from input\n");
      sys->info(
            "  -q  = select conductors for at-1V charge distribution .ps pictures\n");
      sys->info(
            "  -rc = remove conductors from all charge distribution .ps pictures\n");
      sys->info(
            "  -b  = superimpose lines, arrows and dots in .figfile on all .ps pictures\n");
      sys->info("  -m  = switch to dump-ps-picture-file-only mode\n");
      sys->info(
            "  -rk = remove key in shaded .ps picture file (use with -q option)\n");
      sys->info(
            "  -rd = remove DIELEC type surfaces from all .ps picture files\n");
      sys->info(
            "  -dc = display total charges in shaded .ps picture file (use with -q option)\n");
      sys->info("  -c  = print command line in .ps picture file\n");
      sys->info("  -v  = suppress showpage in all .ps picture files\n");
      sys->info("  -n  = number faces with input order numbers\n");
      sys->info("  -f  = do not fill in faces (don't rmv hidden lines)\n");
      sys->info("  -g  = dump depth graph and quit\n");
    } else {
      sys->info(
            "Usage: '%s [-o<expansion order>] [-d<partitioning depth>] [<input file>]\n                [-p<permittivity factor>] [-rs<cond list>] [-ri<cond list>]\n                [-] [-l<list file>] [-t<iter tol>]\n", argv[0]);
      sys->info("DEFAULT VALUES:\n");
      sys->info("  expansion order = %d\n", DEFORD);
      sys->info("  partitioning depth = set automatically\n");
      sys->info("  permittivity factor = 1.0\n");
      sys->info("  iterative loop ||r|| tolerance = %g\n", ABSTOL);
      sys->info("OPTIONS:\n");
      sys->info("  -   = force conductor surface file read from stdin\n");
      sys->info("  -rs = remove conductors from solve list\n");
      sys->info("  -ri = remove conductors from input\n");
    }
    sys->info("  <cond list> = [<name>],[<name>],...,[<name>]\n");
    dumpConfig(sys, stderr, argv[0]);
    sys->error("Command line parsing failed.");
  }
}

/*
  surface information input routine - panels are read by read_panels()
*/
static surface *read_all_surfaces(ssystem *sys, const char *input_file, const char *surf_list_file, int read_from_stdin, char *infile)
{
  int num_surf, i;
  char group_name[BUFSIZ];
  surface *surf_list, *cur_surf;

  /* get the surfaces from stdin, the list file or the file on cmd line */
  /* the `- ' option always forces the first cond surf read from stdin */
  /* can also read from stdin if there's no list file and no cmd line file */
  infile[0] = '\0';
  num_surf = 0;
  surf_list = NULL;
  strcpy(group_name, "GROUP1");
  if(read_from_stdin || (input_file == NULL && surf_list_file == NULL)) {
    surf_list = sys->heap.alloc<surface>(1, AMSC);
    surf_list->type = CONDTR;   /* only conductors can come in stdin */
    surf_list->name = sys->heap.strdup("stdin");
    surf_list->outer_perm = sys->perm_factor;
    surf_list->end_of_chain = TRUE;

    /* set up group name */
    surf_list->group_name = sys->heap.strdup(group_name);
    strcpy(group_name, "GROUP2");

    cur_surf = surf_list;

    strcpy(infile, "stdin");
    num_surf++;
  }

  /* set up to read from command line file, if necessary */
  if(input_file != NULL) {
    if(surf_list == NULL) {
      surf_list = sys->heap.alloc<surface>(1, AMSC);
      cur_surf = surf_list;
    }
    else {
      cur_surf->next = sys->heap.alloc<surface>(1, AMSC);
      cur_surf = cur_surf->next;
    }
    cur_surf->type = CONDTR;
    cur_surf->name = sys->heap.strdup(input_file);
    cur_surf->outer_perm = sys->perm_factor;
    cur_surf->end_of_chain = TRUE;

    /* set up group name */
    cur_surf->group_name = sys->heap.strdup(group_name);

    for(i = 0; infile[i] != '\0'; i++);
    if(infile[0] != '\0') sprintf(&(infile[i]), ", %s", input_file);
    else sprintf(&(infile[i]), "%s", input_file);
    num_surf++;
    read_from_stdin++;
  }

  /* read list file if present */
  if(surf_list_file != NULL) {
    read_list_file(sys, &surf_list, &num_surf, surf_list_file, read_from_stdin);
    for(i = 0; infile[i] != '\0'; i++);
    if(infile[0] != '\0') sprintf(&(infile[i]), ", %s", surf_list_file);
    else sprintf(&(infile[i]), "%s", surf_list_file);
  }

  return(surf_list);
}

/*
  surface input routine and command line parser
  - inputs surfaces (ie file names whose panels are read in read_panels)
  - sets parameters accordingly
*/
static surface *input_surfaces(ssystem *sys, char *infile)
{
  int read_from_stdin;
  const char *surf_list_file, *input_file;

  /* initialize defaults */
  surf_list_file = input_file = NULL;
  read_from_stdin = FALSE;

  parse_command_line(sys, &input_file, &surf_list_file, &read_from_stdin);

  return(read_all_surfaces(sys, input_file, surf_list_file,
                           read_from_stdin, infile));
}

/*
  dump the data associated with the input surfaces
*/
static void dumpSurfDat(ssystem *sys, surface *surf_list)
{
  surface *cur_surf;

  sys->msg("  Input surfaces:\n");
  for(cur_surf = surf_list; cur_surf != NULL; cur_surf = cur_surf->next) {

    /* possibly write group name */
    if(cur_surf == surf_list) sys->msg("   %s\n", cur_surf->group_name);
    else if(cur_surf->prev->end_of_chain)
        sys->msg("   %s\n", cur_surf->group_name);

    /* write file name */
    sys->msg("    %s", hack_path(cur_surf->name));
    if(cur_surf->type == CONDTR) {
      sys->msg(", conductor\n");
      sys->msg("      title: `%s'\n", cur_surf->title);
      sys->msg("      outer permittivity: %g\n",
              cur_surf->outer_perm);
    }
    else if(cur_surf->type == DIELEC) {
      sys->msg(", dielectric interface\n");
      sys->msg("      title: `%s'\n", cur_surf->title);
      sys->msg("      permittivities: %g (inner) %g (outer)\n",
              cur_surf->inner_perm, cur_surf->outer_perm);
    }
    else if(cur_surf->type == BOTH) {
      sys->msg(", thin conductor on dielectric interface\n");
      sys->msg("      title: `%s'\n", cur_surf->title);
      sys->msg("      permittivities: %g (inner) %g (outer)\n",
              cur_surf->inner_perm, cur_surf->outer_perm);
    }
    else {
      sys->error("dumpSurfDat: bad surface type\n");
    }
    sys->msg("      number of panels: %d\n",
            cur_surf->num_panels - cur_surf->num_dummies);
    sys->msg("      number of extra evaluation points: %d\n",
            cur_surf->num_dummies);
    sys->msg("      translation: (%g %g %g)\n",
            cur_surf->trans[0], cur_surf->trans[1], cur_surf->trans[2]);

  }
}

/*
  replaces name (and all aliases) corresponding to "num" with unique string
*/
static void remove_name(ssystem *sys, Name **name_list, int num)
{
  static char str[] = "%`_^#$REMOVED";
  Name *cur_name, *cur_alias;
  int i, slen;

  slen = strlen(str);

  for(i = 1, cur_name = *name_list; cur_name != NULL; 
      cur_name = cur_name->next, i++) {
    if(i == num) {

      /* overwrite name */
      if(strlen(cur_name->name) < (size_t)slen) {
        cur_name->name = sys->heap.alloc<char>(slen+1, AMSC);
      }
      strcpy(cur_name->name, str);

      /* overwrite aliases */
      for(cur_alias = cur_name->alias_list; cur_alias != NULL;
          cur_alias = cur_alias->next) {
        if(strlen(cur_alias->name) < (size_t)slen) {
          cur_alias->name = sys->heap.alloc<char>(slen+1, AMSC);
        }
        strcpy(cur_alias->name, str);
      }
    }
  }

}
        
/*
  removes (unlinks from linked list) panels that are on conductors to delete
*/
static void remove_conds(ssystem *sys, charge **panels, ITER *num_list, Name **name_list)
{
  ITER *cur_num;
  charge *cur_panel, *prev_panel;

  for(cur_panel = prev_panel = *panels; cur_panel != NULL; 
      cur_panel = cur_panel->next) {
    if(cur_panel->dummy) continue;
    if(cur_panel->surf->type == CONDTR || cur_panel->surf->type == BOTH) {
      if(want_this_iter(num_list, cur_panel->cond)) {
        /* panel's conductor is to be removed, so unlink the panel */
        /* - if panel to be removed is first panel, rewrite head pointer */
        if(cur_panel == *panels) *panels = cur_panel->next;
        /* - otherwise bypass cur_panel with next pointers */
        else prev_panel->next = cur_panel->next;
      }
      else prev_panel = cur_panel;
    }
  }

  /* remove all -ri'd conductor names from master name list
     - required to get rid of references in capsolve()
     - actually, name and all its aliases are replaced by ugly string
       (not the cleanest thing) */
  for(cur_num = num_list; cur_num != NULL; cur_num = cur_num->next) {
    remove_name(sys, name_list, cur_num->iter);
  }
}

/*
  checks for kill lists with inconsistent demands
  -rs list: can't remove a conductor physically removed from computation w/-ri
  -q list: can't dump q plot for cond physically rmed or rmed from comp
  -rc list: no restrictions
  -ri/-rs: can't exhaust all conductors with combination of these lists
*/
static void resolve_kill_lists(ssystem *sys, ITER *rs_num_list, ITER *q_num_list, ITER *ri_num_list)
{
  int i, lists_exhaustive;
  ITER *cur_num;

  /* check for anything in -rs list in -ri list */
  for(cur_num = ri_num_list; cur_num != NULL; cur_num = cur_num->next) {
    if(want_this_iter(rs_num_list, cur_num->iter)) {
      sys->error("resolve_kill_lists: a conductor removed with -ri is in the -rs list\n");
    }
  }

  /* check for anything in -q list in -ri or -rs list
     - recall that -q by itself means plot for all active, 
       so null q_num_list always ok */
  for(cur_num = q_num_list; cur_num != NULL; cur_num = cur_num->next) {
    if(want_this_iter(rs_num_list, cur_num->iter)
       || want_this_iter(ri_num_list, cur_num->iter)) {
      sys->error("resolve_kill_lists: a conductor removed with -ri or -rs is in the -q list\n");
    }
  }

  /* check that -rs and -ri lists don't exhaust all conductors */
  lists_exhaustive = TRUE;
  for(i = 1; i <= sys->num_cond; i++) {
    if(!want_this_iter(rs_num_list, i) && !want_this_iter(ri_num_list, i)) {
      lists_exhaustive = FALSE;
      break;
    }
  }
  if(lists_exhaustive && !sys->m_) {
    sys->error("resolve_kill_lists: all conductors either in -ri or -rs list\n");
  }
}

/*
  main input routine, returns a list of panels in the problem
*/
charge *input_problem(ssystem *sys)
{
  surface *surf_list;
  char infile[BUFSIZ], hostname[BUFSIZ];
  charge *chglist;
  long clock;

  /* read the conductor and dielectric interface surface files, parse cmds */
  surf_list = input_surfaces(sys, infile);

  if (sys->dirsol || sys->expgcr) {
    sys->depth = 0;                /* put all the charges in first cube */
  }

  strcpy(hostname, "18Sep92");
  sys->msg("Running %s %.1f (%s)\n  Input: %s\n",
          sys->argv[0], VERSION, hostname, infile);

  /* input the panels from the surface files */
  sys->num_cond = 0;                /* initialize conductor count */
  chglist = read_panels(sys, surf_list, &sys->num_cond);

  /* set up the lists of conductors to remove from solve list */
  sys->kill_num_list = get_kill_num_list(sys, sys->cond_names, sys->kill_name_list);

  /* remove the panels on specified conductors from input list */
  sys->kinp_num_list = get_kill_num_list(sys, sys->cond_names, sys->kinp_name_list);
  remove_conds(sys, &chglist, sys->kinp_num_list, &sys->cond_names);

  /* set up the lists of conductors to dump shaded plots for */
  sys->qpic_num_list = get_kill_num_list(sys, sys->cond_names, sys->qpic_name_list);

  /* set up the lists of conductors to eliminate from shaded plots */
  sys->kq_num_list = get_kill_num_list(sys, sys->cond_names, sys->kq_name_list);

  /* check for inconsistencies in kill lists */
  resolve_kill_lists(sys, sys->kill_num_list, sys->qpic_num_list, sys->kinp_num_list);

  if (sys->dissrf) {
    dumpSurfDat(sys, surf_list);
  }

  time(&clock);
  sys->msg("  Date: %s", ctime(&clock));
  if(gethostname(hostname, BUFSIZ) != -1)
      sys->msg("  Host: %s\n", hostname);
  else sys->msg("  Host: ? (gethostname() failure)\n");

  if (sys->cfgdat) {
    dumpConfig(sys, stdout, sys->argv[0]);
  }

  /* return the panels from the surface files */
  return(chglist);
}
