#include	<stdio.h>
#define 	TEMPFILE	".sema_temp"

main()
{
	FILE *fp;
	char buf[125];
	char digit[7];

	system("ipcs > .sema_temp");
	fp = fopen(TEMPFILE, "r");
	while( fgets(buf, 125, fp) > 0 )
	{
		puts(buf);
		if ( buf[1] != ' ' ) continue;
		switch(buf[0])
		{
		case 'm' :
		case 's' :
			strncpy(digit, &buf[2], 6);
			digit[6] = '\0';
			sprintf(buf,"ipcrm -%c %s",buf[0],digit);
			system(buf);
			break;
		default :
			puts("I have NO idea");
		}
	}
	unlink(TEMPFILE);
	puts("========== RESULT ===========");
	system("ipcs");
}
