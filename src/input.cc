
#include "mulGlobal.h"
#include "mulDisplay.h"
#include "mulStruct.h"
#include "zbufGlobal.h"
#include "input.h"
#include "calcp.h"
#include "patran_f.h"
#include "quickif.h"

#include <cstdio>
#include <cstring>
#include <cassert>
#include <string>
#include <sstream>

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
void read_list_file(ssystem *sys, Surface **surf_list, const char *list_file)
{
  int linecnt = 0, end_of_chain = TRUE, ref_pnt_is_inside = FALSE;
  FILE *fp;
  char tline[BUFSIZ], file_name[BUFSIZ], plus[BUFSIZ], group_name[BUFSIZ];
  double outer_perm = 1.0, inner_perm = 1.0, tx = 0.0, ty = 0.0, tz = 0.0, rx = 0.0, ry = 0.0, rz = 0.0;
  Surface *cur_surf = 0;

  /* find the end of the current surface list */
  if(*surf_list != NULL) {
    for(cur_surf = *surf_list; cur_surf->next != NULL; 
        cur_surf = cur_surf->next);
  }
  
  /* attempt to open file list file */
  if((fp = fopen(list_file, "r")) == NULL) {
    sys->error("read_list_file: can't open list file\n  `%s'\nto read", list_file);
  }

  /* read file names and permittivities, build linked list */
  linecnt = 0;
  sprintf(group_name, "GROUP%d", ++sys->group_cnt);
  while(fgets(tline, sizeof(tline), fp) != NULL) {
    linecnt++;
    if(tline[0] == 'C' || tline[0] == 'c') {
      if(sscanf(&(tline[1]), "%s %lf %lf %lf %lf", 
                file_name, &outer_perm, &tx, &ty, &tz) != 5) {
        sys->error("read_list_file: bad conductor surface format, tline %d:\n%s",
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
        *surf_list = sys->heap.create<Surface>(AMSC);
        cur_surf = *surf_list;
      }
      else {
        cur_surf->next = sys->heap.create<Surface>(AMSC);
        cur_surf->next->prev = cur_surf;
        cur_surf = cur_surf->next;
      }
      
      cur_surf->type = CONDTR;
      cur_surf->trans = Vector3d(tx, ty, tz);
      cur_surf->end_of_chain = end_of_chain;
      cur_surf->name = sys->heap.strdup(file_name);
      cur_surf->outer_perm = outer_perm;

      /* set up group name */
      cur_surf->group_name = sys->heap.strdup(group_name);

      /* update group name if end of chain */
      if(end_of_chain) {
        sprintf(group_name, "GROUP%d", ++sys->group_cnt);
      }

    }
    else if(tline[0] == 'B' || tline[0] == 'b') {
      if(sscanf(&(tline[1]), "%s %lf %lf %lf %lf %lf %lf %lf %lf", 
                file_name, &outer_perm, &inner_perm, &tx, &ty, &tz,
                &rx, &ry, &rz) != 9) {
        sys->error("read_list_file: bad thin conductor on dielectric interface surface format, line %d:\n%s",
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
        *surf_list = sys->heap.create<Surface>(AMSC);
        cur_surf = *surf_list;
      }
      else {
        cur_surf->next = sys->heap.create<Surface>(AMSC);
        cur_surf->next->prev = cur_surf;
        cur_surf = cur_surf->next;
      }
      
      cur_surf->type = BOTH;
      cur_surf->trans = Vector3d(tx, ty, tz);
      cur_surf->ref = Vector3d(rx, ry, rz);
      cur_surf->ref_inside = ref_pnt_is_inside;
      cur_surf->end_of_chain = end_of_chain;
      cur_surf->name = sys->heap.strdup(file_name);
      cur_surf->outer_perm = outer_perm;
      cur_surf->inner_perm = inner_perm;

      /* set up group name */
      cur_surf->group_name = sys->heap.strdup(group_name);

      /* update group name if end of chain */
      if(end_of_chain) {
        sprintf(group_name, "GROUP%d", ++sys->group_cnt);
      }

    }
    else if(tline[0] == 'D' || tline[0] == 'd') {
      if(sscanf(&(tline[1]), "%s %lf %lf %lf %lf %lf %lf %lf %lf", 
                file_name, &outer_perm, &inner_perm, &tx, &ty, &tz,
                &rx, &ry, &rz) != 9) {
        sys->error("read_list_file: bad dielectric interface surface format, line %d:\n%s",
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
        *surf_list = sys->heap.create<Surface>(AMSC);
        cur_surf = *surf_list;
      }
      else {
        cur_surf->next = sys->heap.create<Surface>(AMSC);
        cur_surf->next->prev = cur_surf;
        cur_surf = cur_surf->next;
      }
      
      cur_surf->type = DIELEC;
      cur_surf->trans = Vector3d(tx, ty, tz);
      cur_surf->ref = Vector3d(rx, ry, rz);
      cur_surf->ref_inside = ref_pnt_is_inside;
      cur_surf->end_of_chain = TRUE;
      cur_surf->name = sys->heap.strdup(file_name);
      cur_surf->outer_perm = outer_perm;
      cur_surf->inner_perm = inner_perm;

      /* set up group name */
      cur_surf->group_name = sys->heap.strdup(group_name);

      /* update group name (DIELEC surface is always end of chain) */
      sprintf(group_name, "GROUP%d", ++sys->group_cnt);

    }
    else if(tline[0] == 'G' || tline[0] == 'g') {
      if(sscanf(&(tline[1]), "%s", group_name) != 1) {
        sys->error("read_list_file: bad group name format, line %d:\n%s",
                   linecnt, tline);
      }
    }
    else if(tline[0] == '%' || tline[0] == '*' ||
            tline[0] == '#'); /* ignore comments */
    else {
      sys->error("read_list_file: bad line format, line %d:\n%s",
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
  sets up the ps file base string
*/
void get_ps_file_base(ssystem *sys)
{
  const char **argv = sys->argv;
  int argc = sys->argc;

  int i, j;
  char temp[BUFSIZ];

  sys->ps_file_base = 0;

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

  if(!sys->ps_file_base) {       /* input must be stdin */
    sys->ps_file_base = sys->heap.strdup("stdin");
  }
}

/*
  open all the surface files and return a charge (panel) struct list
  set up pointers from each panel to its corresponding surface struct
  align the normals of all the panels in each surface so they point
    towards the same side as where the ref point is (dielectric files only)
*/
static charge *read_panels(ssystem *sys)
{
  int patran_file, num_panels, stdin_read, num_dummies, num_quads, num_tris;
  charge *panel_list = 0, *cur_panel = 0, *c_panel;
  Surface *cur_surf;
  FILE *fp;
  char surf_name[BUFSIZ];
  int patran_file_read = FALSE;

  sys->num_cond = 0;
  sys->cond_names = NULL;

  stdin_read = FALSE;
  for(cur_surf = sys->surf_list; cur_surf != NULL; cur_surf = cur_surf->next) {

    charge *panels_read = 0;
    char *title = 0;

    if (cur_surf->name) {

      if(!strcmp(cur_surf->name, "stdin")) {
        if(stdin_read) {
          sys->error("read_panels: attempt to read stdin twice");
        }
        else {
          stdin_read = TRUE;
          fp = stdin;
        }
      }
      else if((fp = fopen(cur_surf->name, "r")) == NULL) {
        sys->error("read_panels: can't open\n  `%s'\nto read",
                   cur_surf->name);
      }

      /* input the panel list */

      if(!cur_surf->prev || cur_surf->prev->end_of_chain) {
        sprintf(surf_name, "%%%s", cur_surf->group_name);
        patran_file_read = FALSE;
      }

      /* figure out input file type and read it in */
      char header[BUFSIZ];
      fgets(header, BUFSIZ, fp);

      if (header[0] == '0') {
        patran_file = FALSE;
        panels_read = quickif(sys, fp, header, cur_surf->type, cur_surf->rot, cur_surf->trans, surf_name, &title);
      } else {
        patran_file = TRUE;
        panels_read = patfront(sys, fp, header, cur_surf->type, cur_surf->rot, cur_surf->trans, surf_name, &title);
      }

      if(!patran_file && patran_file_read) {
        sys->error("read_panels: generic format file\n  `%s'\nread after neutral file(s) in same group - reorder list file entries", cur_surf->name);
      }
      patran_file_read = patran_file;

      if(fp != stdin) {
        fclose(fp);
      }

    } else {

      assert(cur_surf->surf_data);
      patran_file_read = FALSE;

      int cond_num = 0;
      if (cur_surf->type == CONDTR || cur_surf->type == BOTH) {
        std::ostringstream os;
        if (cur_surf->surf_data->name) {
          os << cur_surf->surf_data->name;
        }
        os << "%";
        os << cur_surf->group_name;
        cond_num = sys->get_conductor_number(os.str().c_str());
      }

      if (cur_surf->surf_data->title) {
        title = sys->heap.strdup(cur_surf->surf_data->title);
      }

      panels_read = quickif2charges(sys, cur_surf->surf_data->quads, cur_surf->surf_data->tris, cur_surf->rot, cur_surf->trans, cond_num);

    }

    if (!panel_list) {
      panel_list = panels_read;
    } else {
      cur_panel->next = panels_read;
    }
    cur_panel = panels_read;
    cur_surf->panels = cur_panel;

    /* save the surface file's title */
    if (! title) {
      title = sys->heap.strdup(sys->title ? sys->title : "");
    }
    cur_surf->title = title;

    /* if the surface is a DIELEC, make sure all conductor numbers are zero */
    /* - also link each panel to its surface */
    for(c_panel = cur_panel; c_panel; c_panel = c_panel->next) {
      if(cur_surf->type == DIELEC) c_panel->cond = 0;
      c_panel->surf = cur_surf;
    }
    
    /* align the normals and add dummy structs if dielec i/f */
    initcalcp(sys, cur_surf->panels);/* get normals, edges, perpendiculars */
    if(cur_surf->type == DIELEC || cur_surf->type == BOTH) {
      add_dummy_panels(sys, cur_surf->panels); /* add dummy panels for field calc */
    }

    /* make cur_panel = last panel in list, count panels */
    num_panels = num_dummies = num_tris = num_quads = 0;
    while (cur_panel) {
      num_panels++;
      if (cur_panel->dummy) num_dummies++;
      else if (cur_panel->shape == 3) num_tris++;
      else if (cur_panel->shape == 4) num_quads++;
      else {
        sys->error("read_panels: bad panel shape, %d", cur_panel->shape);
      }
      if (!cur_panel->next) {
        break;
      }
      cur_panel = cur_panel->next;
    }

    /*sys->msg("Surface %s has %d quads and %d tris\n",
            cur_surf->name, num_quads, num_tris);*/

    cur_surf->num_panels = num_panels;
    cur_surf->num_dummies = num_dummies;

  }

  return(panel_list);
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

  get_ps_file_base(sys); /* set up the output file base */

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
    dumpConfig(sys, argv[0]);
    sys->error("Command line parsing failed.");
  }
}

/*
  surface information input routine - panels are read by read_panels()
*/
static Surface *read_all_surfaces(ssystem *sys, const char *input_file, const char *surf_list_file, int read_from_stdin, std::string &infiles)
{
  char group_name[BUFSIZ];
  Surface *surf_list = NULL, *cur_surf = 0;

  /* get the surfaces from stdin, the list file or the file on cmd line */
  /* the `- ' option always forces the first cond surf read from stdin */
  /* can also read from stdin if there's no list file and no cmd line file */
  if(read_from_stdin || (input_file == NULL && surf_list_file == NULL)) {
    surf_list = sys->heap.create<Surface>(AMSC);
    surf_list->type = CONDTR;   /* only conductors can come in stdin */
    surf_list->name = sys->heap.strdup("stdin");
    //  surf_list->outer_perm = sys->perm_factor;  //  TODO: shouldn't that be 1 because the perm_factor is added later?
    surf_list->outer_perm = 1.0;
    surf_list->end_of_chain = TRUE;

    /* set up group name */
    sprintf(group_name, "GROUP%d", ++sys->group_cnt);
    surf_list->group_name = sys->heap.strdup(group_name);

    cur_surf = surf_list;

    infiles = "stdin";
  }

  /* set up to read from command line file, if necessary */
  if(input_file != NULL) {
    if(surf_list == NULL) {
      surf_list = sys->heap.create<Surface>(AMSC);
      cur_surf = surf_list;
    }
    else {
      cur_surf->next = sys->heap.create<Surface>(AMSC);
      cur_surf = cur_surf->next;
    }
    cur_surf->type = CONDTR;
    cur_surf->name = sys->heap.strdup(input_file);
    //  cur_surf->outer_perm = sys->perm_factor;    //  TODO: shouldn't that be 1 because the perm_factor is added later?
    cur_surf->outer_perm = 1.0;
    cur_surf->end_of_chain = TRUE;

    /* set up group name */
    sprintf(group_name, "GROUP%d", ++sys->group_cnt);
    cur_surf->group_name = sys->heap.strdup(group_name);

    if (!infiles.empty()) {
      infiles += ",";
    }
    infiles += input_file;
  }

  /* read list file if present */
  if(surf_list_file != NULL) {
    read_list_file(sys, &surf_list, surf_list_file);
    if (!infiles.empty()) {
      infiles += ",";
    }
    infiles += surf_list_file;
  }

  return(surf_list);
}

/*
  surface input routine and command line parser
  - inputs surfaces (ie file names whose panels are read in read_panels)
  - sets parameters accordingly
*/
void populate_from_command_line(ssystem *sys)
{
  int read_from_stdin;
  const char *surf_list_file, *input_file;
  std::string infiles;                  /* comma-separated list of input files */

  /* initialize defaults */
  surf_list_file = input_file = NULL;
  read_from_stdin = FALSE;

  parse_command_line(sys, &input_file, &surf_list_file, &read_from_stdin);

  sys->surf_list = read_all_surfaces(sys, input_file, surf_list_file, read_from_stdin, infiles);

  sys->msg("Running %s %.1f\n  Input: %s\n",
          sys->argv[0], VERSION, infiles.c_str());
}

/*
  dump the data associated with the input surfaces
*/
void dumpSurfDat(ssystem *sys)
{
  Surface *cur_surf;

  sys->msg("  Input surfaces:\n");
  for(cur_surf = sys->surf_list; cur_surf != NULL; cur_surf = cur_surf->next) {

    /* possibly write group name */
    if(cur_surf == sys->surf_list) sys->msg("   %s\n", cur_surf->group_name);
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
      sys->error("dumpSurfDat: bad surface type");
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
  replaces name (and all aliases) corresponding to one from "num_list" with unique (ugly) string
*/
static void remove_names(ssystem *sys, const std::set<int> &num_list)
{
  static char str[] = "%`_^#$REMOVED";   //  TODO: why not empty?

  for (auto i = num_list.begin(); i != num_list.end(); ++i) {
    Name *name = sys->conductor_name(*i);
    if (name) {
      name->name = sys->heap.strdup(str);
      for (Name *a = name->alias_list; a; a = a->next) {
        a->name = sys->heap.strdup(str);
      }
    }
  }
}
        
/*
  removes (unlinks from linked list) panels that are on conductors to delete
*/
static void remove_conds(ssystem *sys, charge **panels, const std::set<int> &num_list, Name **name_list)
{
  charge *cur_panel, *prev_panel;

  for (cur_panel = prev_panel = *panels; cur_panel; cur_panel = cur_panel->next) {
    if (cur_panel->dummy) {
      continue;
    }
    if (cur_panel->surf->type == CONDTR || cur_panel->surf->type == BOTH) {
      if (num_list.find(cur_panel->cond) != num_list.end()) {
        //  panel's conductor is to be removed, so unlink the panel
        //  - if panel to be removed is first panel, rewrite head pointer
        if (cur_panel == *panels) {
          *panels = cur_panel->next;
        } else {
          //  - otherwise bypass cur_panel with next pointers
          prev_panel->next = cur_panel->next;
        }
      } else {
        prev_panel = cur_panel;
      }
    }
  }

  //  TODO: appears to be working without this ...

  //  remove all -ri'd conductor names from master name list
  //  - required to get rid of references in capsolve()
  //  - actually, name and all its aliases are replaced by ugly string
  //    (not the cleanest thing)
  remove_names(sys, num_list);
}

/*
  checks for kill lists with inconsistent demands
  -rs list: can't remove a conductor physically removed from computation w/-ri
  -q list: can't dump q plot for cond physically rmed or rmed from comp
  -rc list: no restrictions
  -ri/-rs: can't exhaust all conductors with combination of these lists
*/
static void resolve_kill_lists(const ssystem *sys, const std::set<int> &rs_num_list, const std::set<int> &q_num_list, const std::set<int> &ri_num_list)
{
  //  check for anything in -rs list in -ri list
  for (auto i = ri_num_list.begin(); i != ri_num_list.end(); ++i) {
    if (rs_num_list.find(*i) != rs_num_list.end()) {
      sys->error("resolve_kill_lists: a conductor removed with -ri is in the -rs list");
    }
  }

  //  check for anything in -q list in -ri or -rs list
  //  - recall that -q by itself means plot for all active,
  //    so null q_num_list always ok
  for (auto i = q_num_list.begin(); i != q_num_list.end(); ++i) {
    if(rs_num_list.find(*i) != rs_num_list.end() || ri_num_list.find(*i) != ri_num_list.end()) {
      sys->error("resolve_kill_lists: a conductor removed with -ri or -rs is in the -q list");
    }
  }

  //  check that -rs and -ri lists don't exhaust all conductors
  bool lists_exhaustive = true;
  for (int i = 1; i <= sys->num_cond; i++) {
    if (rs_num_list.find(i) == rs_num_list.end() && ri_num_list.find(i) == ri_num_list.end()) {
      lists_exhaustive = false;
      break;
    }
  }
  if (lists_exhaustive && !sys->m_) {
    sys->error("resolve_kill_lists: all conductors either in -ri or -rs list");
  }
}

/*
  main input routine, returns a list of panels in the problem
*/
charge *build_charge_list(ssystem *sys)
{
  //  used cached panels if possible
  if (sys->panels) {
    return sys->panels;
  }

  //  input the panels from the surface files
  //  as a side effect this will fill the conductor name list
  charge *chglist = read_panels(sys);

  //  build the selector lists by number
  sys->kinp_num_list = sys->get_conductor_number_set(sys->kinp_name_list);
  sys->kill_num_list = sys->get_conductor_number_set(sys->kill_name_list);
  sys->qpic_num_list = sys->get_conductor_number_set(sys->qpic_name_list);
  sys->kq_num_list = sys->get_conductor_number_set(sys->kq_name_list);

  //  remove the panels on specified conductors from input list
  remove_conds(sys, &chglist, sys->kinp_num_list, &sys->cond_names);

  //  check for inconsistencies in kill lists on this occasion
  resolve_kill_lists(sys, sys->kill_num_list, sys->qpic_num_list, sys->kinp_num_list);

  //  return the panels from the surface files
  sys->panels = chglist;
  return chglist;
}
