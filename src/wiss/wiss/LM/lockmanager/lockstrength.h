
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


#include "locktype.h"
#include "lockquiz.h"
  
/*
 * The convertibility table
 */
short LM_Strength[MAXLOCKTYPES][MAXLOCKTYPES]= {
NL  { TRUE, TRUE, TRUE, TRUE, TRUE, TRUE },
IS  { FALSE, TRUE, FALSE, TRUE, TRUE, FALSE},
IX  { FALSE, FALSE, TRUE, FALSE, TRUE, TRUE },
S  { FALSE, FALSE, FALSE, TRUE, FALSE, FALSE},
SIX  { FALSE, FALSE, FALSE, FALSE, TRUE, TRUE},
X  { FALSE, FALSE, FALSE, FALSE, FALSE, TRUE}
};
/*
 * The compatibility table
 */
short LM_compat[MAXLOCKTYPES][MAXLOCKTYPES]= {
  { LEGAL, LEGAL, LEGAL, LEGAL, LEGAL, LEGAL},
  { LEGAL, LEGAL, LEGAL, LEGAL, LEGAL, ILLEGAL},
  { LEGAL, LEGAL, LEGAL, ILLEGAL, ILLEGAL, ILLEGAL},
  { LEGAL, LEGAL, ILLEGAL, LEGAL, ILLEGAL, ILLEGAL},
  { LEGAL, LEGAL, ILLEGAL, ILLEGAL, ILLEGAL, ILLEGAL},
  { LEGAL, ILLEGAL, ILLEGAL, ILLEGAL, ILLEGAL, ILLEGAL}
};
/*
 * The supremum table
 */
short LM_supr[MAXLOCKTYPES][MAXLOCKTYPES] = {
  { l_NL, l_IS, l_IX, l_S, l_SIX, l_X },
  { l_IS, l_IS, l_IX, l_S, l_SIX, l_X },
  { l_IX, l_IX, l_IX, l_SIX, l_SIX, l_X },
  { l_S, l_S, l_SIX, l_S, l_SIX, l_X},
  { l_SIX, l_SIX, l_SIX, l_SIX, l_SIX, l_X},
  { l_X, l_X, l_X, l_X, l_X, l_X}
};

char LM_locknames[MAXLOCKTYPES][4] = {
  "NL", "IS", "IX", "S", "SIX", "X"
  };

short LM_leaftype[MAXLOCKTYPES]= {
  FALSE, FALSE, FALSE, TRUE, FALSE, TRUE
  };
