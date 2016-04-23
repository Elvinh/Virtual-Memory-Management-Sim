/* 
  vmms_error.h - This file contains some common declarations for vmms, mmc, and user programs.
*/

#define VMMS_SUCCESS		  0 // success RC
#define	OUT_OF_MEM			100 // Out of memory.
#define	MEM_TOO_SMALL		101 // Destination memory buffer is too small for this operation.
#define	INVALID_DEST_ADDR 	102 // Invalid destination memory address.
#define	INVALID_CPY_ADDR 	103 // Invalid destination & source memory address.
#define INVALID_MEM_ADDR	104 // Invalid memory address. (includes freeing the same pointer twice)

