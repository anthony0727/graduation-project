/*
 * lock definition file for testandset() package.
 *
 * Author: Josef Burger
 */

struct testandset {
#if defined(hp800) || defined(hppa)
	/*
	 * gads, what a kludge
	 * 
	 * HPPA needs a 16 byte length aligned at a 16 byte boundary.
	 * The testandset() package will automatically align the address
	 * given to it to the NEXT 16 byte boundary.
	 * 
	 * If you use malloc, to allocate locks, it will return a
	 * 8-byte aligned pointer, and you can use the lock[6] definition
	 * for alignment to work correctly.
	 *
	 * If you use statically-allocated locks, either verify that 
	 * the compiler will guarantee a 8-byte alignment for the structure,
	 * or use a length of 32 (28 if space is tight) to guarantee
	 * enough space for alignment.
	 */
#if GUARANTEE_EIGHT
	int	lock[6];	/* MUST be 8-byte aligned */
#else
	int	lock[8];	/* if 4-aligned */
#endif
#endif /* hp800 */
#if defined(ibm032)
	short	lock;
#endif /* ibm032 */
#if defined(hp300) || defined(sparc) || defined(i386) || defined(vax)
	char	lock;
#endif	/* lots of stuff */
};
