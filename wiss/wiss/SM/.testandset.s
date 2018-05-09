 








































 













.seg "text" ; .proc 14 ; .global  testandset  ;  testandset  : 
	retl
	ldstub	[%o0],%o0	 

.seg "text" ; .proc 14 ; .global  testandset_release  ;  testandset_release  : 
	retl
	stb	%g0,[%o0]

.seg "text" ; .proc 14 ; .global  testandset_init  ;  testandset_init  : 
	retl
	stb	%g0,[%o0]

.seg "text" ; .proc 14 ; .global  testandset_examine  ;  testandset_examine  : 
	retl
	ldub	[%o0],%o0













