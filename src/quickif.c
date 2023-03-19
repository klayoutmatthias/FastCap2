/*!\page LICENSE LICENSE
 
Copyright (C) 2003 by the Board of Trustees of Massachusetts Institute of Technology, hereafter designated as the Copyright Owners.
 
License to use, copy, modify, sell and/or distribute this software and
its documentation for any purpose is hereby granted without royalty,
subject to the following terms and conditions:
 
1.  The above copyright notice and this permission notice must
appear in all copies of the software and related documentation.
 
2.  The names of the Copyright Owners may not be used in advertising or
publicity pertaining to distribution of the software without the specific,
prior written permission of the Copyright Owners.
 
3.  THE SOFTWARE IS PROVIDED "AS-IS" AND THE COPYRIGHT OWNERS MAKE NO
REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED, BY WAY OF EXAMPLE, BUT NOT
LIMITATION.  THE COPYRIGHT OWNERS MAKE NO REPRESENTATIONS OR WARRANTIES OF
MERCHANTABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF THE
SOFTWARE WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS TRADEMARKS OR OTHER
RIGHTS. THE COPYRIGHT OWNERS SHALL NOT BE LIABLE FOR ANY LIABILITY OR DAMAGES
WITH RESPECT TO ANY CLAIM BY LICENSEE OR ANY THIRD PARTY ON ACCOUNT OF, OR
ARISING FROM THE LICENSE, OR ANY SUBLICENSE OR USE OF THE SOFTWARE OR ANY
SERVICE OR SUPPORT.
 
LICENSEE shall indemnify, hold harmless and defend the Copyright Owners and
their trustees, officers, employees, students and agents against any and all
claims arising out of the exercise of any rights under this Agreement,
including, without limiting the generality of the foregoing, against any
damages, losses or liabilities whatsoever with respect to death or injury to
person or damage to property arising from or out of the possession, use, or
operation of Software or Licensed Program(s) by LICENSEE or its customers.
 
*/

#include "mulGlobal.h"
#include "quickif.h"

/*
  tells if any conductor name alias matches a string
*/
int alias_match(cur_name, name)
char *name;
Name *cur_name;
{
  Name *cur_alias;

  for(cur_alias = cur_name->alias_list; cur_alias != NULL;
      cur_alias = cur_alias->next) {
    if(!strcmp(name, cur_alias->name)) return(TRUE);
  }
  return(FALSE);
}

/*
  tells if any conductor name alias matches a string only up to length(name)
*/
int alias_match_name(cur_name, name)
char *name;
Name *cur_name;
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

/*
  adds an alias 
*/
void add_to_alias(cur_name, new_name)
Name *cur_name;
char *new_name;
{
  Name *last_alias = NULL, *cur_alias;

  for(cur_alias = cur_name->alias_list; cur_alias != NULL;
      cur_alias = cur_alias->next) {
    last_alias = cur_alias;
  }

  if(last_alias == NULL) {
    CALLOC(cur_name->alias_list, 1, Name, ON, AMSC);
    CALLOC(cur_name->alias_list->name, strlen(new_name)+1, char, ON, AMSC);
    strcpy(cur_name->alias_list->name, new_name);
  }
  else {
    CALLOC(last_alias, 1, Name, ON, AMSC);
    CALLOC(last_alias->name, strlen(new_name)+1, char, ON, AMSC);
    strcpy(last_alias->name, new_name);
  }
}

/*
  return pointer to last alias string (= current name for conductor)
*/
char *last_alias(cur_name)
Name *cur_name;
{
  Name *cur_alias, *last_alias;

  last_alias = NULL;
  for(cur_alias = cur_name->alias_list; cur_alias != NULL;
      cur_alias = cur_alias->next) {
    last_alias = cur_alias;
  }

  if(last_alias == NULL) last_alias = cur_name;
  return(last_alias->name);
}

/*
  manages the conductor name list
*/
int getConductorNum(char *name, Name **name_list, int *num_cond)
{
  Name *cur_name, *prev_name;
  int i, alias_match();

  /* if this is the very first name, make and load struct on *name_list */
  if(*num_cond == 0) {
    CALLOC(*name_list, 1, Name, ON, AMSC);
    CALLOC((*name_list)->name, strlen(name)+1, char, ON, AMSC);
    strcpy((*name_list)->name, name);
    (*name_list)->next = NULL;
    *num_cond = 1;
    return(1);
  }

  /* check to see if name is present */
  for(cur_name = *name_list, i = 1; cur_name != NULL;
      cur_name = cur_name->next, i++) {
    if(!strcmp(cur_name->name, name) || alias_match(cur_name, name)) 
	return(i); /* return conductor number */
    prev_name = cur_name;
  }

  /* add the new name to the list */
  CALLOC(prev_name->next, 1, Name, ON, AMSC);
  CALLOC(prev_name->next->name, strlen(name)+1, char, ON, AMSC);
  strcpy(prev_name->next->name, name);
  prev_name->next->next = NULL;
  *num_cond = i;
  return(i);
}


/*
  checks to see if name is present in conductor name list
  - does not add if it isnt present
  - returns NOTFND if name not found
  - name must have group name appended
*/
int getConductorNumNoAdd(char *name, Name *name_list)
{
  Name *cur_name, *prev_name;
  int i, alias_match();

  /* check to see if name is present */
  for(cur_name = name_list, i = 1; cur_name != NULL;
      cur_name = cur_name->next, i++) {
    if(!strcmp(cur_name->name, name) || alias_match(cur_name, name)) 
	return(i); /* return conductor number */
    prev_name = cur_name;
  }

  return(NOTFND);
}

/*
  gets the name (aliases are ignored) corresponding to a conductor number
*/
char *getConductorName(int cond_num, Name **name_list)
{
  Name *cur_name;
  char *last_alias();
  int i;

  /* check to see if conductor number is present */
  for(cur_name = *name_list, i = 1; cur_name != NULL;
      cur_name = cur_name->next, i++) {
    if(i == cond_num) return(last_alias(cur_name));
  }

  /* number not found */
  fprintf(stderr, 
	  "getConductorName: conductor no. %d not defined\n", cond_num);
  return(NULL);
}

/*
  renames a conductor
*/
int oldrenameConductor(char *old_name, char *new_name, Name **name_list, int *num_cond)
{
  Name *cur_name, *cur_name2, *prev_name;
  int i, j;

  /* check to see if old name is present */
  prev_name = NULL;
  for(cur_name = *name_list, i = 1; cur_name != NULL;
      cur_name = cur_name->next, i++) {
    if(!strcmp(cur_name->name, old_name)) {
      /* old name is present; check to see if new name is present */
      for(cur_name2 = *name_list, j = 1; cur_name2 != NULL;
	  cur_name2 = cur_name2->next, j++) {
	if(!strcmp(cur_name2->name, new_name)) {
	  /* new name already present; just delete old name entry */
	  if(prev_name != NULL) prev_name->next = cur_name->next;
	  else *name_list = cur_name->next;
	  (*num_cond)--;
	  return(TRUE);
	}
      }
      /* substitute the new name */
      if(strlen(cur_name->name) < strlen(new_name))
	  CALLOC(cur_name->name, strlen(new_name)+1, char, ON, AMSC);
      strcpy(cur_name->name, new_name);
      return(TRUE);
    }
    prev_name = cur_name;
  }

  /* name not found */
  fprintf(stderr, "renameConductor: unknown conductor `%s'\n", old_name);
  return(FALSE);
}

/*
  renames a conductor
*/
int renameConductor(char *old_name, char *new_name, Name **name_list, int *num_cond)
{
  Name *cur_name, *cur_name2, *prev_name;
  int alias_match();
  int i, j;

  /* check to see if old name is present in names or their aliases */
  /* if it is, add old name to the alias list */
  for(cur_name = *name_list, i = 1; cur_name != NULL;
      cur_name = cur_name->next, i++) {
    if(!strcmp(cur_name->name, old_name) || alias_match(cur_name, old_name)) {
      /* old name is cur name or old name is an alias */
      /* if new name isn't in alias list, add it to alias list */
      if(!alias_match(cur_name, new_name)) 
	  add_to_alias(cur_name, new_name);
      return(TRUE);
    }
  }

  /* old name not found in entire list */
  fprintf(stderr, "renameConductor: unknown conductor `%s'\n", old_name);
  return(FALSE);

}

/*
  returns list of charge structs derived from quick input format:
  0 <Title string>       (must be first line, others in any order OK)
  Q <cond name string> X1 Y1 Z1 X2 Y2 Z2 X3 Y3 Z3 X4 Y4 Z4
  T <cond name string> X1 Y1 Z1 X2 Y2 Z2 X3 Y3 Z3
  N <cond name string> <Rename string>
  * <Comment string>
*/
charge *quickif(fp, line, title, surf_type, trans, num_cond, name_list,
		name_suffix)
int surf_type, *num_cond;
char *line, *title, *name_suffix; /* suffix for all cond names read */
double *trans;
Name *name_list;		/* name list pointer */
FILE *fp;
{
  int linecnt = 2, i, cond;
  char *delcr(), temp[BUFSIZ], temp2[BUFSIZ], line1[BUFSIZ], *cp, *strtok();
  char condstr[BUFSIZ];
  double x1, x2, x3, x4, y1, y2, y3, y4, z1, z2, z3, z4;
  quadl *fstquad = NULL, *curquad;
  tri *fsttri = NULL, *curtri;
  charge *chglst, *nq;
  NAME *cname;

  chglst = NULL;

  /* save the title, strip leading '0' */
  if(title[0] == '\0') strcpy(title, delcr(&line[1]));
  
  /* read in and load structs */
  while(fgets(line1, sizeof(line1), fp) != NULL) {
    if(line1[0] == 'Q' || line1[0] == 'q') {
      if(sscanf(line1, 
		"%s %s %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
		temp, condstr, 
		&x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3, &x4, &y4, &z4)
	 != 14) {
	fprintf(stderr, "quickif: bad quad format, line %d:\n%s\n", 
		linecnt, line1);
	exit(0);
      }

      /* add suffix */
      strcat(condstr, name_suffix);

      /* allocate quad struct */
      if(fstquad == NULL) {
	CALLOC(fstquad, 1, quadl, ON, AMSC);
	curquad = fstquad;
      }
      else {
	CALLOC(curquad->next, 1, quadl, ON, AMSC);
	curquad = curquad->next;
      }
      
      /* load quad struct */
      if(surf_type == CONDTR || surf_type == BOTH)
	  curquad->cond = getConductorNum(condstr, name_list, num_cond);
      else curquad->cond = 0;
      curquad->x1 = x1 + trans[0];
      curquad->x2 = x2 + trans[0];
      curquad->x3 = x3 + trans[0];
      curquad->x4 = x4 + trans[0];
      curquad->y1 = y1 + trans[1];
      curquad->y2 = y2 + trans[1];
      curquad->y3 = y3 + trans[1];
      curquad->y4 = y4 + trans[1];
      curquad->z1 = z1 + trans[2];
      curquad->z2 = z2 + trans[2];
      curquad->z3 = z3 + trans[2];
      curquad->z4 = z4 + trans[2];

      linecnt++;
    }
    else if(line1[0] == 'T' || line1[0] == 't') {
      if(sscanf(line1, "%s %s %lf %lf %lf %lf %lf %lf %lf %lf %lf",
		temp, condstr, &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3) 
	 != 11) {
	fprintf(stderr, "quickif: bad tri format, line %d:\n%s\n", 
		linecnt, line1);
	exit(0);
      }

      /* allocate tri struct */
      if(fsttri == NULL) {
	CALLOC(fsttri, 1, tri, ON, AMSC);
	curtri = fsttri;
      }
      else {
	CALLOC(curtri->next, 1, tri, ON, AMSC);
	curtri = curtri->next;
      }

      /* add suffix */
      strcat(condstr, name_suffix);
      
      /* load tri struct */
      if(surf_type == CONDTR || surf_type == BOTH)
	  curtri->cond = getConductorNum(condstr, name_list, num_cond);
      else curquad->cond = 0;
      curtri->x1 = x1 + trans[0];
      curtri->x2 = x2 + trans[0];
      curtri->x3 = x3 + trans[0];
      curtri->y1 = y1 + trans[1];
      curtri->y2 = y2 + trans[1];
      curtri->y3 = y3 + trans[1];
      curtri->z1 = z1 + trans[2];
      curtri->z2 = z2 + trans[2];
      curtri->z3 = z3 + trans[2];

      linecnt++;
    }
    else if(line1[0] == 'N' || line1[0] == 'n') {
      if(sscanf(line1, "%s %s %s", temp, condstr, temp2) != 3) {
	fprintf(stderr, "quickif: bad rename format, line %d:\n%s\n", 
		linecnt, line1);
	exit(0);
      }

      if(surf_type != DIELEC) {
	/* rename only if surface is BOTH or CONDTR type */
	/* - eventually will be used when DIELEC surfs get names (someday) */

	/* add suffix */
	strcat(condstr, name_suffix);
	strcat(temp2, name_suffix);

	if(renameConductor(condstr, temp2, name_list, num_cond) == FALSE) 
	    exit(0);
      }

      linecnt++;
    }
    else if(line1[0] == '%' || line1[0] == '*' ||
	    line1[0] == '#') linecnt++; /* ignore comments */
    else {
      fprintf(stderr, "quickif: bad line format, line %d:\n%s\n", 
		linecnt, line1);
      exit(0);
    }
  }
	
  /* setup tris in charge structs */
  for(curtri = fsttri; curtri != NULL; curtri = curtri->next) {

    /* allocate charge struct to fill in */
    if(chglst == NULL) {
      CALLOC(chglst, 1, charge, ON, AMSC);
      nq = chglst;
    }
    else {
      CALLOC(nq->next, 1, charge, ON, AMSC);
      nq = nq->next;
    }

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
    nq->cond = curtri->cond;
  }

  /* setup quads in charge structs */
  for(curquad = fstquad; curquad != NULL; curquad = curquad->next) {

    /* allocate charge struct to fill in */
    if(chglst == NULL) {
      CALLOC(chglst, 1, charge, ON, AMSC);
      nq = chglst;
    }
    else {
      CALLOC(nq->next, 1, charge, ON, AMSC);
      nq = nq->next;
    }

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
    nq->cond = curquad->cond;
  }
  return(chglst);
}

