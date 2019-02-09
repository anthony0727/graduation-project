
/********************************************************/
/*                                                      */
/*               WiSS Storage System                    */
/*          Version SystemV-4.0, September 1990	        */
/*                                                      */
/*              COPYRIGHT (C) 1990                      */
/*                David J. DeWitt 		        */
/*               Madison, WI U.S.A.                     */
/*                                                      */
/*	         ALL RIGHTS RESERVED                    */
/*                                                      */
/********************************************************/


#
/* Module : am_newopenindexscan
  	Opens an index scan on an opened data file.

   IMPORT: 
  	int am_openindexscan(openfilenum, indexfilenum, 
  			*indexkey, *LB, *UB, *booleanexpr, trans_id,
			lockup, lockmode, cond)
   EXPORT:
  	int am_newopenindexscan(openfilenum, indexfilenum, *indexkey, 
  			*LB, relop_lb, *UB, relop_ub, *booleanexpr, trans_id,
			lockup, lockmode, cond)
*/

#include <wiss.h>
#include <am.h>
#include <lockquiz.h>
#define epsilon .000001  
#define DUMMY 0
#define EQ_ 1
#define NE_ 2
#define GT_ 3
#define GE_ 4
#define LT_ 5
#define LE_ 6

int
am_newopenindexscan (openfilenum, indexfilenum, indexkey, lb, relop_lb, 
	ub, relop_ub, booleanexpr, trans_id, lockup, lockmode, cond)
int	openfilenum;	/* open file # of the data file */
int	indexfilenum;	/* open file # of the index file */
KEYINFO	*indexkey;	/* key attribute of the index */
KEY	*lb;		/* lower bound of scan */
int	relop_lb;/* relational operator associated with the lower bound */
KEY	*ub;		/* upper bound of scan */
int	relop_ub;/* relational operator associated with the upper bound */
BOOLEXP	*booleanexpr;   /* pointer to a search predicate */
int     trans_id;
short   lockup;
LOCKTYPE lockmode;
short	cond;

{
  int scanid;

  if (relop_lb == EQ_)
   {
    ub->type = lb->type;
    ub->length = lb->length;
    strncpy(ub->value, lb->value, MAXFIELD);
   }
  else
   {
    if (lb != NULL)
      convert_lower_bound(indexkey, lb, relop_lb);
    if (ub != NULL)
      convert_upper_bound(indexkey, ub, relop_ub);
   }
  scanid = am_openindexscan(openfilenum, indexfilenum, indexkey, lb, ub, 
	booleanexpr, trans_id, lockup, lockmode, cond);
  return(scanid);
}

convert_lower_bound(indexkey, bound, relop)
KEYINFO	*indexkey;
KEY	*bound;
int	relop;
{
   switch (relop)
   {
    case GT_ :
	      switch (indexkey->type)
	      {
    		case TINTEGER :
		case TLONG :
				*(int *)(bound->value) += 1;
		    		break;
                case TSHORT   :
				*(short *)(bound->value) += 1;
		    		break;
    		case TFLOAT   :
				*(float *)(bound->value) += epsilon;
		   		break;
    		case TSTRING  :
				addone(bound->value, bound->length);
		   		break;
	      }
    case DUMMY:
    case EQ_ :
    case GE_ : /* do nothing */
	      break;
    case LT_ :
    case LE_ :
    case NE_ : /* error in relop */
    default : printf("Relop on lower bound of index scan wrong\n");
	      break;
   }
}

convert_upper_bound(indexkey, bound, relop)
KEYINFO	*indexkey;
KEY	*bound;
int	relop;
{
   switch (relop)
   {
    case LT_ :
	      switch (indexkey->type)
	      {
    		case TINTEGER : 
		case TLONG :
				*(int *)(bound->value) -= 1;
		    		break;
                case TSHORT   :
				*(short *)(bound->value) -= 1;
		    		break;
    		case TFLOAT   :
				*(float *)(bound->value) -= epsilon;
		   		break;
    		case TSTRING  :
				subone(bound->value, bound->length);
		   		break;
	      }
    case DUMMY:
    case EQ_ :
    case LE_ : /* do nothing */
	      break;
    case GT_ :
    case GE_ :
    case NE_ : 
    default : printf("Wrong relop in Upper bound in index scan\n");
	      break;
   }
}

addone(value, length)
char 	*value;
int	length;
{
}

subone(value, length)
char	*value;
int	length;
{
}
