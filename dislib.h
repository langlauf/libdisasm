/* this is to be included for all bastard disassembly libraries
   it contains the instruction flags and code structure def's 
   it should also contain lib prototypes and be included in the
   bastard, hence the #ifdef */
   
#ifndef IAMTHEBASTARD
/* IMPORTANT:
   Anything in this section is replicated in bdb.h and api.h in the main
	bastard source distribution. Changes here MUST be propagated to the
	bastard include files!!!!
*/
typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    DWORD;

struct code {  /* size 100 */
    unsigned long    rva;
    unsigned short   flags;
    char    mnemonic[16];
    char    dest[32];
    char    src[32];
    char    aux[32];
    int     mnemType;
    int     destType;
    int     srcType;
    int     auxType;
};

 /* Operand and instruction types */
#define OP_REG			0x100		/* register */
#define OP_IMM			0x200		/* immediate value */
#define OP_IND			0x300		/* indirect memory reference */
#define OP_BPTR		0x400		/* BYTE Pointer */
#define OP_WPTR		0x500		/* WORD Pointer */
#define OP_DPTR		0x600		/* DWORD Pointer */
#define OP_UNK			0x900		
//#define INS_INVALID	0x00	/* Not a valid instruction */
   /* Branch Instruction types */
#define INS_BRANCH	0x01	/* Unconditional branch */
#define INS_COND		0x02	/* Conditional branch */
#define INS_SUB		0x04	/* Jump to subroutine */
#define INS_RET		0x08	/* Return from subroutine */
   /* modify ( 'w' ) instructions */
#define INS_ARITH 	0x10 /* Arithmetic inst */
#define INS_LOGIC 	0x20 /* logical inst */
#define INS_FPU   	0x40 /* Floating Point inst */
#define INS_FLAG  	0x80 /* Modify flags */
   /* misc Instruction Types */
#define INS_MOVE		0x0100
#define INS_ARRAY    0x0200   /* String and XLAT ops */
#define INS_PTR      0x0400   /* Load EA/pointer */
#define INS_STACK 	0x1000	/* PUSH, POP, etc */
#define INS_FRAME	   0x2000	/* ENTER, LEAVE, etc */
#define INS_SYSTEM	0x4000	/* CPUID, WBINVD, etc */

/* Other info */
#define BIG_ENDIAN_ORDER 0
#define LITTLE_ENDIAN_ORDER 1

#endif

/* opcode tables */
#define  x86_MAIN 0
#define  x86_0F   1
#define  x86_80   2
//...and so on....


/* exported functions */

int DisasmAddr( BYTE *c, BYTE n, struct code *inst, DWORD rva );

//Not yet written:
int DisasmInit(char *arg);
int GetByteSize();
int GetWordSize();
int GetDwordSize();
int GetAddrSize();
int GetByteOrder();

