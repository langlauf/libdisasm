/* Instruction format:
 * __________________________________________________________________________
 | Inst  | AddrSize | OpSize | SegOverRide | Opcode | R/M | SIB | Disp | Imm |
 |  Pfx  |    Pfx   |   Pfx  |     Pfx     |        |     |     |      |     |
 * --------------------------------------------------------------------------
 * */

#include "dislib.h"

/* struct used in Init routine */
struct CPU_TYPE{
	char vendor;
	char model[12];
};

// TODO:
// cpu_type flags -- these should be made to work w/ struct above
#define cpu_80386      0x01
#define cpu_80486      0x02
#define cpu_PENTIUM    0x04
#define cpu_PENTMMX    0x08
#define cpu_PENTPRO    0x10
#define cpu_PENTIUM2   0x20
#define cpu_PENTIUM3   0x40
#define cpu_PENTIUM4   0x80

// TODO:
//instruction flags for opcode map -- these should be resolved w/ dislib.h
#define FLAGS_MODRM      0x00001  //contains mod r/m byte
#define FLAGS_8BIT       0x00002  //force 8-bit arguments
#define FLAGS_16BIT      0x00004  //force 16-bit arguments
#define FLAGS_32BIT      0x00008  //force 32-bit arguments
#define FLAGS_REAL       0x00010  //real mode only
#define FLAGS_PMODE      0x00020  //protected mode only
#define FLAGS_PREFIX     0x00040  //for lock and rep prefix
#define FLAGS_MMX        0x00080  //mmx instruction/registers
#define FLAGS_FPU        0x00100  //fpu instruction/registers
#define FLAGS_CJMP       0x00200  //codeflow - conditional jump
#define FLAGS_JMP        0x00400  //codeflow - jump
#define FLAGS_IJMP       0x00800  //codeflow - indexed jump
#define FLAGS_CALL       0x01000  //codeflow - call
#define FLAGS_ICALL      0x02000  //codeflow - indexed call
#define FLAGS_RET        0x04000  //codeflow - return
#define FLAGS_SEGPREFIX  0x08000  //segment prefix
#define FLAGS_OPERPREFIX 0x10000  //operand prefix
#define FLAGS_ADDRPREFIX 0x20000  //address prefix
#define FLAGS_OMODE16    0x40000  //16-bit operand mode only
#define FLAGS_OMODE32    0x80000  //32-bit operand mode only

// TODO:
//argument types from opcode map -- these need to be consolidated!
#define  ARG_REG 0x10
#define  ARG_IMM 0x20
#define  ARG_NONE 0x30
#define  ARG_MODRM 0x40
#define  ARG_REG_AX 0x50
#define  ARG_REG_ES 0x60
#define  ARG_REG_CS 0x70
#define  ARG_REG_SS 0x80
#define  ARG_REG_DS 0x90
#define  ARG_REG_FS 0xA0
#define  ARG_REG_GS 0xB0
#define  ARG_REG_BX 0xC0
#define  ARG_REG_CX 0xD0
#define  ARG_REG_DX 0xE0
#define  ARG_REG_SP 0xF0
#define  ARG_REG_BP 0x100
#define  ARG_REG_SI 0x110
#define  ARG_REG_DI 0x120
#define  ARG_IMM8 0x130
#define  ARG_RELIMM8 0x140
#define  ARG_FADDR 0x150
#define  ARG_REG_AL 0x160
#define  ARG_MEMLOC 0x170
#define  ARG_SREG 0x180
#define  ARG_RELIMM 0x190
#define  ARG_16REG_DX 0x1A0
#define  ARG_REG_CL 0x1B0
#define  ARG_REG_DL 0x1C0
#define  ARG_REG_BL 0x1D0
#define  ARG_REG_AH 0x1E0
#define  ARG_REG_CH 0x1F0
#define  ARG_REG_DH 0x200
#define  ARG_REG_BH 0x210
#define  ARG_MODREG 0x220
#define  ARG_CREG 0x230
#define  ARG_DREG 0x240
#define  ARG_TREG_67 0x250
#define  ARG_TREG 0x260
#define  ARG_MREG 0x270
#define  ARG_MMXMODRM 0x280
#define  ARG_MODRM8 0x290
#define  ARG_IMM_1 0x2A0
#define  ARG_MODRM_FPTR 0x2B0
#define  ARG_MODRM_S 0x2C0
#define  ARG_MODRMM512 0x2D0
#define  ARG_MODRMQ 0x2E0
#define  ARG_MODRM_SREAL 0x2F0
#define  ARG_REG_ST0 0x300
#define  ARG_FREG 0x310 
#define  ARG_MODRM_PTR 0x320
#define  ARG_MODRM_WORD 0x330
#define  ARG_MODRM_SINT 0x340
#define  ARG_MODRM_EREAL 0x350
#define  ARG_MODRM_DREAL 0x360
#define  ARG_MODRM_WINT 0x370
#define  ARG_MODRM_LINT 0x380
#define  ARG_REG_BC 0x390
#define  ARG_REG_DE 0x3A0
#define  ARG_REG_HL 0x3B0
#define  ARG_REG_DE_IND 0x3C0
#define  ARG_REG_HL_IND 0x3D0
#define  ARG_REG_BC_IND 0x3E0
#define  ARG_REG_SP_IND 0x3F0
#define  ARG_REG_A 0x400
#define  ARG_REG_B 0x410
#define  ARG_REG_C 0x420
#define  ARG_REG_D 0x430
#define  ARG_REG_E 0x440
#define  ARG_REG_H 0x450
#define  ARG_REG_L 0x460
#define  ARG_IMM16 0x470
#define  ARG_REG_AF 0x480
#define  ARG_REG_AF2 0x490
#define  ARG_MEMLOC16 0x4A0
#define  ARG_IMM8_IND 0x4B0
#define  ARG_BIT 0x4C0
#define  ARG_REG_IX 0x4D0
#define  ARG_REG_IX_IND 0x4E0
#define  ARG_REG_IY 0x4F0
#define  ARG_REG_IY_IND 0x500
#define  ARG_REG_C_IND 0x510
#define  ARG_REG_I 0x520
#define  ARG_REG_R 0x530
#define  ARG_IMM16_A 0x540 
#define  ARG_MODRM16 0x550 
#define  ARG_SIMM8 0x560
#define  ARG_IMM32 0x570
#define  ARG_STRING 0x580
#define  ARG_MODRM_BCD 0x590
#define  ARG_PSTRING 0x5A0
#define  ARG_DOSSTRING 0x5B0
#define  ARG_CUNICODESTRING 0x5C0
#define  ARG_PUNICODESTRING 0x5D0
#define  ARG_NONEBYTE 0x5E0
#define  ARG_XREG 0x5F0
#define  ARG_XMMMODRM 0x600
  
// TODO:  
// add clocks !!! 
typedef struct x86_inst {
	int flags;
	int destType, srcType, auxType;
	int cpu_type;
	int inst_type;
	char mnem[12];
	char dest[4], src[4], aux[4];
} instr;




/* --------------------------------------------------------- */
/*  modR/M + SIB decoding stuff                              */

// Registers
#define GENREG_8      0x0001
#define GENREG_16     0x0002
#define GENREG_32     0x0004
#define SEGREG        0x0008
#define MMXREG        0x0010
#define SIMDREG       0x0020
#define DEBUGREG      0x0040
#define CONTROLREG    0x0080
#define TESTREG       0x0100

char *genRegs8[]    = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh" };
char *genRegs16[]   = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
char *genRegs32[]   = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi" };
char *mmxRegs[]     = {"mm0", "mm1", "mm2", "mm3", "mm4", "mm5", "mm6", "mm7"};
char *simdRegs[]    = {"xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7"};
char *debugRegs[]   = {"dr0","dr1","dr2","dr3","dr4","dr5","dr6","dr7"};
char *controlRegs[] = {"cr0","cr1","cr2","cr3","cr4","cr5","cr6","cr7"};
char *testRegs[]    = {"tr0","tr1","tr2","tr3","tr4","tr5","tr6","tr7"};
char *segRegs[]     = {"es","cs","ss","ds","fs","gs"};
char *idxRegs[]={"bx+si","bx+di","bp+si","bp+di","si","di","bp","bx"};
char *fpuRegs[]={"st(0)","st(1)","st(2)","st(3)","st(4)","st(5)","st(6)","st(7)"};
// why was this in Borg? :
//char *regzascii[]={"b","c","d","e","h","l","(hl)","a"};

//ModR/M and SIB flags [internal]
#define NO_REG     0x100
#define DIRECT_REG 0x200
#define NO_BASE    0x400
#define NO_INDEX   0x800
#define DISP8     0x1000
#define DISP32    0x2000
#define HAS_SIB   0x4000
#define HAS_MODRM 0x8000 

/* Convenience struct for handling multiple args */
struct OPERAND {    	//arg1, arg2, arg3
   char * str;			//temporary buffer for building arg text
   int    type;		//argument type
   int *  flag;		//pointer to CODE arg flags
   char * text;		//pointer to CODE arg text
};

/* Convenience struct for generating SIB byte */
struct EA {		//effective address [SIB/disp]
   int mode, flags;
   int mod, rm, reg;
   long disp;
   char sib[32];
};

/* Convenience struct for modR/M bitfield */
struct modRM_byte {	
   unsigned int mod : 2;
   unsigned int reg : 3;
   unsigned int rm  : 3;
};

int modrm_rm[] = {0,1,2,3,HAS_SIB,DISP32,6,7};
int modrm_reg[] = {0, 1, 2, 3, 4, 5, 6, 7};
int modrm_mod[]  = {0, DISP8, DISP32, DIRECT_REG};

/* Convenience struct for SIB bitfield */
struct SIB_byte {
   unsigned int scale : 2;
   unsigned int index : 3;
   unsigned int base  : 3;
};

int sib_scl[] = {0, 2, 4, 8};
int sib_idx[] = {0, 1, 2, 3, NO_INDEX, 5, 6, 7 };
int sib_bas[] = {0, 1, 2, 3, 4, NO_BASE, 6, 7 };


#include "i386.opcode.map"

/* Convenience struct for opcode tables */
typedef struct x86_table {             //Assembly instruction tables
  instr *table;      //Pointer to table of instruction encodings
  char divisor;            // number to divide by for look up
  char mask;               // bit mask for look up
  char minlim,maxlim;      // limits on min/max entries.
  char modrmpos;           // modrm byte position plus
} asmtable;

// if .mnem == NULL then .flags = offset in table
asmtable tables86[]={ 
{tbl_Main,0,0xff,0,0xff,0},					/* 0 */
  {tbl_0F,0,0xff,0,0xff,1},
  {tbl_80,8,0x07,0,0xff,0},
  {tbl_81,8,0x07,0,0xff,0},
  {tbl_82,8,0x07,0,0xff,0},
  {tbl_83,8,0x07,0,0xff,0},					/* 5 */
  {tbl_C0,8,0x07,0,0xff,0},
  {tbl_C1,8,0x07,0,0xff,0},
  {tbl_D0,8,0x07,0,0xff,0},
  {tbl_D1,8,0x07,0,0xff,0},
  {tbl_D2,8,0x07,0,0xff,0},					/* 10 */
  {tbl_D3,8,0x07,0,0xff,0},
  {tbl_F6,8,0x07,0,0xff,0},
  {tbl_F7,8,0x07,0,0xff,0},
  {tbl_FE,8,0x07,0,0xff,0},
  {tbl_FF,8,0x07,0,0xff,0},					/* 15 */
  {tbl_0F00,8,0x07,0,0xff,1},
  {tbl_0F01,8,0x07,0,0xff,1},
  {tbl_0F18,8,0x07,0,0xff,1},
  {tbl_0F71,8,0x07,0,0xff,1},
  {tbl_0F72,8,0x07,0,0xff,1},		/* 20 */
  {tbl_0F73,8,0x07,0,0xff,1},
  {tbl_0FAE,8,0x07,0,0xff,1},
  {tbl_0FBA,8,0x07,0,0xff,1},
  {tbl_0FC2,0,0x00,0,0xff,1},
  {tbl_0FC7,8,0x07,0,0xff,1}		/* 25 */
  //{tbl_D8a,TABLE_EXT,0xd8,0,8,0x07,0,0xbf,0},        /* CoProc Tables */
  //{tbl_D8b,TABLE_EXT,0xd8,0,8,0x1f,0xc0,0xff,0},
  //{tbl_D9a,TABLE_EXT,0xd9,0,8,0x07,0,0xbf,0},
  //{tbl_D9b,TABLE_EXT,0xd9,0,8,0x1f,0xc0,0xff,0},
  //{tbl_D9c,TABLE_EXT,0xd9,0,1,0xff,0xc0,0xff,1},		/* 30 */
  //{tbl_DAa,TABLE_EXT,0xda,0,8,0x07,0,0xbf,0},
  //{tbl_DAb,TABLE_EXT,0xda,0,8,0x1f,0xc0,0xff,0},
  //{tbl_DAc,TABLE_EXT,0xda,0,1,0xff,0xc0,0xff,1},
  //{tbl_DBa,TABLE_EXT,0xdb,0,8,0x07,0,0xbf,0},
  //{tbl_DBb,TABLE_EXT,0xdb,0,8,0x1f,0xc0,0xff,0},		/* 35 */
  //{tbl_DBc,TABLE_EXT,0xdb,0,1,0xff,0xc0,0xff,1},
  //{tbl_DCa,TABLE_EXT,0xdc,0,8,0x07,0,0xbf,0},
  //{tbl_DCb,TABLE_EXT,0xdc,0,8,0x1f,0xc0,0xff,0},
  //{tbl_DDa,TABLE_EXT,0xdd,0,8,0x07,0,0xbf,0},
  //{tbl_DDb,TABLE_EXT,0xdd,0,8,0x1f,0xc0,0xff,0},		/* 40 */
  //{tbl_DEa,TABLE_EXT,0xde,0,8,0x07,0,0xbf,0},
  //{tbl_DEb,TABLE_EXT,0xde,0,8,0x1f,0xc0,0xff,0},
  //{tbl_DEc,TABLE_EXT,0xde,0,1,0xff,0xc0,0xff,1},
  //{tbl_DFa,TABLE_EXT,0xdf,0,8,0x07,0,0xbf,0},
  //{tbl_DFb,TABLE_EXT,0xdf,0,8,0x1f,0xc0,0xff,0},		/* 45 */
  //{tbl_DFc,TABLE_EXT,0xdf,0,1,0xff,0xc0,0xff,1}
};


