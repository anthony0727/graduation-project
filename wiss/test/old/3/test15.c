

main()
{
	int	i;

	for (i = -100; i > -4000 ; i--)
	{
		printf(" error code %d ", i);
		am_error(" testing", i);
	}
}
