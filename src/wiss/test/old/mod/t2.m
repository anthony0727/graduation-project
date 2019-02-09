
module main;

type	
	addr = integer;
	two  = array 0 : 1 of shortint;

	struct = record
		   offset : two;
		   length : two;
		   dtype  : integer;
		end;


var
	dummy : array 0 : 1 of struct;

begin

	printf(" offset =  %d, length = %d, type = %d, next offset = %d\n",
		adr(dummy[0].offset) - adr(dummy[0]),
		adr(dummy[0].length) - adr(dummy[0]),
		adr(dummy[0].dtype) - adr(dummy[0]),
		adr(dummy[1]) - adr(dummy[0]));
		
end main.
	

