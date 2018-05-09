

module main;

type
	two = array 0 : 1 of char;
	(* two = array 0 : 1 of shortint; *)

var 
	twobyte : two;
	four    : integer;

#define assign2(to , from)\
	to[0] := char(from mod 400B); to[1] := char((from mod 2000B)/400B)
begin
	
	four := 10;
	assign2(twobyte, four);
	printf(" twobyte = 0x%x, 0x%x; four = %d\n",
		twobyte[0], twobyte[1], four);

	four := 127;
	assign2(twobyte, four);
	printf(" twobyte = 0x%x, 0x%x; four = %d\n",
		twobyte[0], twobyte[1], four);

	four := 128;
	assign2(twobyte, four);
	printf(" twobyte = 0x%x, 0x%x; four = %d\n",
		twobyte[0], twobyte[1], four);

end main.

