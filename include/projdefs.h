
/*
 * Defines the prototype to which task functions must conform.  Defined in this
 * file to ensure the type is known before portable.h is included.
 */
typedef void (*TaskFunction_t)( void * );

/* Converts a time in milliseconds to a time in ticks. */
#define pdMS_TO_TICKS( xTimeInMs ) ( ( ( TickType_t ) ( xTimeInMs ) * configTICK_RATE_HZ ) / ( TickType_t ) 1000 )
#define pdTICKS_TO_MS( xTicks )   ( ( uint32_t ) ( xTicks ) * 1000 / configTICK_RATE_HZ )

#define pdFALSE			( ( BaseType_t ) 0 )
#define pdTRUE			( ( BaseType_t ) 1 )

#define pdPASS			( pdTRUE )
#define pdFAIL			( pdFALSE )
#define errQUEUE_EMPTY	( ( BaseType_t ) 0 )
#define errQUEUE_FULL	( ( BaseType_t ) 0 )

/* Error definitions. */
#define errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY	( -1 )
#define errQUEUE_BLOCKED						( -4 )
#define errQUEUE_YIELD							( -5 )

/* Macros used for basic data corruption checks. */
#ifndef configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES
	#define configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES 0
#endif

#if( configUSE_16_BIT_TICKS == 1 )
	#define pdINTEGRITY_CHECK_VALUE 0x5a5a
#else
	#define pdINTEGRITY_CHECK_VALUE 0x5a5a5a5aUL
#endif





