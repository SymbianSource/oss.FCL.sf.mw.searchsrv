/*

Copyright (c) 2001, Dr Martin Porter
Copyright (c) 2002, Richard Boulton
All rights reserved.

Redistribution and use in source and binary forms, with 
or without modification, are permitted provided that the 
following conditions are met:

*	Redistributions of source code must retain the 
	above copyright notice, this list of conditions and 
	the following disclaimer. 
*	Redistributions in binary form must reproduce 
	above copyright notice, this list of conditions and
 	the following disclaimer in the documentation and/or 
	other materials provided with the distribution. 
*	Neither the name of the <ORGANIZATION> nor the 
	names of its contributors may be used to endorse or 
	promote products derived from this software without
	specific rior written permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH 
DAMAGE.

*/

static struct system_word vocab[80+1] = {
  { 0, (byte *)"", 80+1},

  { 1, (byte *)"$",             c_dollar },
  { 1, (byte *)"(",             c_bra },
  { 1, (byte *)")",             c_ket },
  { 1, (byte *)"*",             c_multiply },
  { 1, (byte *)"+",             c_plus },
  { 1, (byte *)"-",             c_minus },
  { 1, (byte *)"/",             c_divide },
  { 1, (byte *)"<",             c_ls },
  { 1, (byte *)"=",             c_assign },
  { 1, (byte *)">",             c_gr },
  { 1, (byte *)"?",             c_debug },
  { 1, (byte *)"[",             c_leftslice },
  { 1, (byte *)"]",             c_rightslice },
  { 2, (byte *)"!=",            c_ne },
  { 2, (byte *)"*=",            c_multiplyassign },
  { 2, (byte *)"+=",            c_plusassign },
  { 2, (byte *)"-=",            c_minusassign },
  { 2, (byte *)"->",            c_sliceto },
  { 2, (byte *)"/*",            c_comment2 },
  { 2, (byte *)"//",            c_comment1 },
  { 2, (byte *)"/=",            c_divideassign },
  { 2, (byte *)"<+",            c_insert },
  { 2, (byte *)"<-",            c_slicefrom },
  { 2, (byte *)"<=",            c_le },
  { 2, (byte *)"==",            c_eq },
  { 2, (byte *)"=>",            c_assignto },
  { 2, (byte *)">=",            c_ge },
  { 2, (byte *)"as",            c_as },
  { 2, (byte *)"do",            c_do },
  { 2, (byte *)"or",            c_or },
  { 3, (byte *)"and",           c_and },
  { 3, (byte *)"for",           c_for },
  { 3, (byte *)"get",           c_get },
  { 3, (byte *)"hex",           c_hex },
  { 3, (byte *)"hop",           c_hop },
  { 3, (byte *)"non",           c_non },
  { 3, (byte *)"not",           c_not },
  { 3, (byte *)"set",           c_set },
  { 3, (byte *)"try",           c_try },
  { 4, (byte *)"fail",          c_fail },
  { 4, (byte *)"goto",          c_goto },
  { 4, (byte *)"loop",          c_loop },
  { 4, (byte *)"next",          c_next },
  { 4, (byte *)"size",          c_size },
  { 4, (byte *)"test",          c_test },
  { 4, (byte *)"true",          c_true },
  { 5, (byte *)"among",         c_among },
  { 5, (byte *)"false",         c_false },
  { 5, (byte *)"limit",         c_limit },
  { 5, (byte *)"unset",         c_unset },
  { 6, (byte *)"atmark",        c_atmark },
  { 6, (byte *)"attach",        c_attach },
  { 6, (byte *)"cursor",        c_cursor },
  { 6, (byte *)"define",        c_define },
  { 6, (byte *)"delete",        c_delete },
  { 6, (byte *)"gopast",        c_gopast },
  { 6, (byte *)"insert",        c_insert },
  { 6, (byte *)"maxint",        c_maxint },
  { 6, (byte *)"minint",        c_minint },
  { 6, (byte *)"repeat",        c_repeat },
  { 6, (byte *)"sizeof",        c_sizeof },
  { 6, (byte *)"tomark",        c_tomark },
  { 7, (byte *)"atleast",       c_atleast },
  { 7, (byte *)"atlimit",       c_atlimit },
  { 7, (byte *)"decimal",       c_decimal },
  { 7, (byte *)"reverse",       c_reverse },
  { 7, (byte *)"setmark",       c_setmark },
  { 7, (byte *)"strings",       c_strings },
  { 7, (byte *)"tolimit",       c_tolimit },
  { 8, (byte *)"booleans",      c_booleans },
  { 8, (byte *)"integers",      c_integers },
  { 8, (byte *)"routines",      c_routines },
  { 8, (byte *)"setlimit",      c_setlimit },
  { 9, (byte *)"backwards",     c_backwards },
  { 9, (byte *)"externals",     c_externals },
  { 9, (byte *)"groupings",     c_groupings },
  { 9, (byte *)"stringdef",     c_stringdef },
  { 9, (byte *)"substring",     c_substring },
 { 12, (byte *)"backwardmode",  c_backwardmode },
 { 13, (byte *)"stringescapes", c_stringescapes }
};
