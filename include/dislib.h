/* this is to be included for all bastard disassembly libraries
   it contains the instruction flags and code structure def's 
   it should also contain lib prototypes and be included in the
   bastard, hence the #ifdef */

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    DWORD;

#ifndef IAMTHEBASTARD
/* IMPORTANT:
   Anything in this section is replicated in bdb.h and api.h in the main
	bastard source distribution. Changes here MUST be propagated to the
	bastard include files!!!!
*/


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
	 unsigned long    destConst;
    unsigned long    srcConst;
    unsigned long    auxConst;
};

 /* Operand and instruction types */
#define OP_R         0x0001   /* operand is READ */
#define OP_W         0x0002   /* operand is WRITTEN */
#define OP_X         0x0004   /* operand is EXECUTED */
#define OP_REG			0x0100	/* register */
#define OP_IMM			0x0200	/* immediate value */
#define OP_IND			0x0300	/* indirect memory reference */
#define OP_REL			0x0400	/* relative offset */
#define OP_BPTR		0x0500	/* BYTE Pointer */
#define OP_WPTR		0x0600	/* WORD Pointer */
#define OP_DPTR		0x0700	/* DWORD Pointer */
#define OP_STRPTR    0x0800   /* STRING Pointer */
#define OP_EXPR      0x0900   /* Address Expression [e.g. SIB byte] */
#define OP_UNK			0x0A00		
#define OP_SIGNED    0x1000   /* operand is signed */
#define OP_UNSIGNED  0x2000   /* operand is unsigned */
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
#define BIG_ENDIAN_ORD	 	0
#define LITTLE_ENDIAN_ORD	1

#endif

/* opcode tables */
#define  x86_MAIN 0
#define  x86_0F   1
#define  x86_80   2
//...and so on....


/* exported functions */

int DisasmAddr( BYTE *c, BYTE n, struct code *inst, DWORD rva );
int DisasmInit(char *arg);
int GetFunctionPrologue(struct code **table);
int GetFunctionEpilogue(struct code **table);
int GetByteOrder();
int GetAddrSize();
int GetByteSize();
int GetWordSize();
int GetDwordSize();
char* GetSP();
char* GetIP();

/* predef 's for use with dynamic loading */
typedef int (*disfunc)(BYTE *, BYTE, struct code *, DWORD);
typedef int (*disinit)(char *);
typedef int (*getsz)();
typedef char* (*getreg)();
typedef int (*getcode)(struct code **);
