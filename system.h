/******************************************************************************/
/* System Level #define Macros                                                */
/******************************************************************************/

/* Microcontroller MIPs (FCY) */
#define _XTAL_FREQ        4000000

#define bit_set(var, bitno) ((var) |= 1UL << (bitno))
#define bit_clear(var, bitno) ((var) &= ~(1UL << (bitno)))

#define array_length( array ) ( sizeof( array ) / sizeof( array[0] ) )


/******************************************************************************/
/* System Function Prototypes                                                 */
/******************************************************************************/