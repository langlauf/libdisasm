#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include "i386.h"

/* -------------------------- Utility/Setup Routines ---------------------- */
int DisasmInit(char *arg){
		  //act based on parameters
		  //  For x86: arg[] = cpu type, which has the format:
		  //  struct CPU_TYPE{
		  //       char vendor;
		  //       char model[12];
		  //  }
		  // Vendor ID:       Models: 
		  //  i = Intel   8086, 8088, 286, 386, 486, 586, 586PRO, 586MMX, 686...
		  //  a = AMD
		  //  c = Cyrix
		  //  b = IBM
		  // ...These will be used w/model to determine appropriate opcodes.
		  return(0);
}


/* -------------------------- Architecture Information Routines ----------- */
int GetByteSize()  { return(8); } // # bits  in machine BYTE
int GetWordSize()  { return(2); } // # bytes in machine WORD
int GetDwordSize() { return(4); } // # bytes in machine DWORD
int GetAddrSize()  { return(4); } // # bytes in Relative Virtual Address
// above should be modified to support 16/20-bit addressing schemes
int GetByteOrder() { return(LITTLE_ENDIAN_ORDER); } 

/* ------------------------- Instruction Decoding ------------------------ */
int SIBDecode(unsigned char c, struct EA *e ){ 
	/* Decodes SIB byte c and returns a character
	 * representation in buf; uses status of modR/M
	 * 'mod' field [mod] to resolve EBP bases */
   struct SIB_byte s;
	char str[32]="\0";
	char base[4]="\0";

	s.scale = c >> 6;
	s.index = (c & 56) >> 3;
	s.base  = c & 7;

	//(scale * index) + base
	if (sib_idx[s.index] == NO_INDEX) e->mode |= NO_INDEX;
	if (sib_bas[s.base] == NO_BASE) e->mode |= NO_BASE;
	if (sib_idx[s.index] != NO_INDEX) {
		if (sib_scl[s.scale] > 0) 
		sprintf(str, "(%d * %s)", sib_scl[s.scale], genRegs32[sib_idx[s.index]]);
		else sprintf(str, "%s", genRegs32[sib_idx[s.index]]);
		if ((sib_bas[s.base] == NO_BASE && e->mod != 0) || 
							 sib_bas[s.base] != NO_BASE )
			sprintf(str, "%s+%s", str, genRegs32[s.base]);
	} else if ((sib_bas[s.base] == NO_BASE && e->mod != 0) || 
							 sib_bas[s.base] != NO_BASE )
			     sprintf(str, "%s", genRegs32[s.base]);
	if (str[0] != 0) strcpy(e->sib, str);    //sprintf(buf, "[%s]",str);
	return 1;
}


int ModRMDecode(unsigned char c, struct EA *e) {
	struct modRM_byte m;
	m.mod = c >> 6;
	m.reg = (c & 56) >> 3;
	m.rm  = c & 7;

	e->mod = m.mod;
	e->rm  = modrm_rm[m.rm];
	e->reg = modrm_reg[m.reg];
	e->mode |= modrm_mod[m.mod];
	// kludge to fix registers ruined by flags
	if ( e->rm > 7) e->rm = m.rm;
	if ( e->mod == 0 && e->rm == 5) e->mode |= NO_REG;
	if ( modrm_rm[m.rm] > 7 && !(modrm_mod[m.mod] & DIRECT_REG))  
			  e->mode |= modrm_rm[m.rm];
	return 1;
}

void setreg( char *buf, int f, char *reg8, char *reg16, char*reg32){
		  /* utility routine. Based on flags f, determines which register
			* string to apply to current argument */
		  if (f & FLAGS_8BIT) strcpy(buf, reg8);
		  else if (f & FLAGS_16BIT) strcpy(buf, reg16);
		  else strcpy(buf, reg32);
		  return;
}

int InstDecode( instr t, BYTE *c, struct code *inst) {
	/* Decodes everything following the opcode bytes -- modR/M, SIB,
	 * Displacement, and Immediate values-- then copies the mnemonic
	 * and arguments into the external code structure. */
	int x, b = 0, n = 0;
	long l = 0;
	char dStr[32], sStr[32], aStr[32], tmpStr[32];
	char d; //delimiter, i.e. comma
	struct EA e;
	struct OPERAND ops[3] = {
	{ dStr, t.destType, &inst->destType, inst->dest},
	{ sStr, t.srcType,  &inst->srcType,  inst->src },
	{ aStr, t.auxType,  &inst->auxType,  inst->aux } };

	t.flags |= FLAGS_32BIT;	
	e.mode = GENREG_32;	
	tmpStr[0]=0; //otherwise this gets corrupt ;(
	memset(&e, 0, sizeof(struct EA));
	
	/* If modR/M, inc byte count and process it */
	if (t.flags & FLAGS_MODRM) {
	  b+= ModRMDecode(c[b], &e); 

    	/* if SIB... */
	  if (e.mode & HAS_SIB){
	      b += SIBDecode(c[b], &e);
	  }

	  /* if displacement ... */
	  if (e.mode & DISP8) n = 1; 
	  else if( e.mode & DISP32) {
	    if (t.flags & FLAGS_16BIT ) n = 2;
	    else n=4;                 //t.flags & FLAGS_32BIT;
	  }
	    
	  if ( n > 0) {
	  	memcpy(&l, &c[b], n);
		if (e.mode & HAS_SIB)
	  	  sprintf(e.sib, "%s + %X", e.sib, l);
		else
	  	  sprintf(e.sib, "%X",l);
	  	b+=n; 
	  }
	}
	
	/* save mnemonic and arguments */
	sprintf(inst->mnemonic, "%s%s", inst->mnemonic, t.mnem);
	for ( x = 0; x < 3; x++) {
      if ( ops[x].str[0] != 0) ops[x].str[0] = 0; 
	   d = ( x > 0 ) ? ',' : ' ';	
		switch (  ops[x].type ) {
		case ARG_NONE :
				  ops[x].str[0] = 0; d = ' '; break;
		case ARG_IMM :
	  	case ARG_IMM_1 :
		case ARG_RELIMM :
				if  (t.flags & FLAGS_8BIT) 
		      {memcpy(&l, &c[b], 1); b+=1;}
				else if  (t.flags & FLAGS_16BIT) 
		      {memcpy(&l, &c[b], 2); b+=2;}
				else //  (t.flags & FLAGS_32BIT) 
		      {memcpy(&l, &c[b], 4); b+=4;}
			   sprintf(ops[x].str, "%X", l);
				break;
		case ARG_IMM8 : case ARG_SIMM8 : case ARG_RELIMM8 : case ARG_IMM8_IND :
				sprintf(ops[x].str, "%X", c[b]); b+=1; break;
		case ARG_IMM16 : case ARG_IMM16_A :
				sprintf(ops[x].str, "%X", (short) c[b]); b+=2; break;
		case ARG_IMM32 :
				sprintf(ops[x].str, "%X", (int) c[b]); b+=4; break;
		case ARG_REG :
				  setreg(ops[x].str, t.flags, genRegs8[e.reg],
				            genRegs16[e.reg], genRegs32[e.reg]); break;
		case ARG_REG_AX :
				  setreg(ops[x].str, t.flags, "ax", "ax", "eax"); break;
		case ARG_REG_BX :
				  setreg(ops[x].str, t.flags, "bx", "bx", "ebx"); break;
		case ARG_REG_CX :
				  setreg(ops[x].str, t.flags, "cx", "cx", "ecx"); break;
		case ARG_REG_DX :
				  setreg(ops[x].str, t.flags, "dx", "dx", "edx"); break;
		case ARG_REG_SP :
				  setreg(ops[x].str, t.flags, "sp", "sp", "esp"); break;
		case ARG_REG_BP :
				  setreg(ops[x].str, t.flags, "bp", "bp", "ebp"); break;
		case ARG_REG_SI :
				  setreg(ops[x].str, t.flags, "si", "si", "esi"); break;
		case ARG_REG_DI :
				  setreg(ops[x].str, t.flags, "di", "di", "edi"); break;
		case ARG_REG_ES :
				  strcpy(ops[x].str, "es");break;
		case ARG_REG_CS :
				  strcpy(ops[x].str, "cs");break;
		case ARG_REG_SS :
				  strcpy(ops[x].str, "ss");break;
		case ARG_REG_DS :
				  strcpy(ops[x].str, "ds");break;
		case ARG_REG_FS :
				  strcpy(ops[x].str, "fs");break;
		case ARG_REG_GS :
				  strcpy(ops[x].str, "gs");break;
		case ARG_REG_AL :
				  strcpy(ops[x].str, "al");break;
		case ARG_REG_CL :
				  strcpy(ops[x].str, "cl");break;
		case ARG_REG_DL :
				  strcpy(ops[x].str, "dl");break;
		case ARG_REG_BL :
				  strcpy(ops[x].str, "bl");break;
		case ARG_REG_AH :
				  strcpy(ops[x].str, "ah");break;
		case ARG_REG_CH :
				  strcpy(ops[x].str, "ch");break;
		case ARG_REG_DH :
				  strcpy(ops[x].str, "dh");break;
		case ARG_REG_BH :
				  strcpy(ops[x].str, "bh");break;
		case ARG_MODREG :
				  setreg(ops[x].str, t.flags, genRegs8[e.rm], genRegs16[e.rm], 
										   genRegs32[e.rm]); 
				  break;
		case ARG_CREG :
				  strcpy(ops[x].str, controlRegs[e.reg]); break;
		case ARG_DREG :
				  strcpy(ops[x].str, debugRegs[e.reg]); break;
				  break;
		case ARG_TREG_67 :
				  break;
		case ARG_TREG :
				  strcpy(ops[x].str, testRegs[e.reg]); break;
				  break;
		case ARG_MREG :
				  strcpy(ops[x].str, mmxRegs[e.reg]); break;
				  break;
		case ARG_SREG :
				  strcpy(ops[x].str, segRegs[e.reg]); break;
				  break;
		case ARG_16REG_DX :
				  break;
		case ARG_REG_ST0 :
				  break;
		case ARG_MODRM :
				  setreg(tmpStr, t.flags, genRegs8[e.rm], 
										genRegs16[e.rm], genRegs32[e.rm]); 
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  else if (e.mode & DIRECT_REG)
					  strcpy(ops[x].str, tmpStr);
				  else {
					  if (e.mode & NO_REG) 
						   strcpy(ops[x].str, e.sib);
					  else if (e.mode & DISP32 || e.mode & DISP8)
							sprintf(ops[x].str, "[%s + %s]", tmpStr, e.sib);
					  else sprintf(ops[x].str, "[%s]", tmpStr);
				  }
				  break;
		case ARG_MODRM8 :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRM16 :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_XMMMODRM :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
			break;
		case ARG_MMXMODRM :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRM_FPTR :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRM_S :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRMM512 :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRMQ :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRM_SREAL :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRM_PTR :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRM_WORD :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRM_SINT :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRM_EREAL :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRM_DREAL :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRM_WINT :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRM_LINT :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_MODRM_BCD :
				  if (e.mode & HAS_SIB)
					  strcpy(ops[x].str, e.sib);
				  break;
		case ARG_FADDR :
				  break;
		case ARG_MEMLOC :
				  break;
		case ARG_MEMLOC16 :
				  break;
		case ARG_BIT :
				  break;
		case ARG_FREG :
				  break;
		case ARG_REG_A :
				  break;
		case ARG_REG_B :
				  break;
		case ARG_REG_C :
				  break;
		case ARG_REG_D :
				  break;
		case ARG_REG_E :
				  break;
		case ARG_REG_H :
				  break;
		case ARG_REG_L :
				  break;
		case ARG_REG_I :
				  break;
		case ARG_REG_R :
				  break;
		case ARG_REG_AF :
				  break;
		case ARG_REG_AF2 :
				  break;
		case ARG_REG_BC :
				  break;
		case ARG_REG_DE :
				  break;
		case ARG_REG_HL :
				  break;
		case ARG_REG_DE_IND :
				  break;
		case ARG_REG_HL_IND :
				  break;
		case ARG_REG_BC_IND :
				  break;
		case ARG_REG_SP_IND :
				  break;
		case ARG_REG_IX :
				  break;
		case ARG_REG_IX_IND :
				  break;
		case ARG_REG_IY :
				  break;
		case ARG_REG_IY_IND :
				  break;
		case ARG_REG_C_IND :
				  break;
		case ARG_STRING :
				  break;
		case ARG_NONEBYTE :
				  break;
		}
		strcpy(ops[x].text, ops[x].str);
	}
   return b;
}

int DisasmAddress( BYTE *c, BYTE n, struct code *inst, DWORD rva){
	BYTE x;
	int b = 1;
	instr *t;

	t = tables86[n].table;
	x = c[0];
	//add table-specific lookup algos here
	if (tables86[n].divisor) {x /=  tables86[n].divisor; b--;};
	if (tables86[n].mask) x  &= tables86[n].mask;

	if ( t[x].mnem[0] != 0){
			//replace with a handle pfx routine
			if ( t[x].flags & FLAGS_PREFIX){
				sprintf(inst->mnemonic, "%s ", t[x].mnem);
				b += 1 + DisasmAddress(&c[1], n, inst, rva);
			} else if ( t[x].flags & FLAGS_SEGPREFIX){
				// set flag
				sprintf(inst->mnemonic, "%s ", t[x].mnem);
				b += 1 + DisasmAddress(&c[1], n, inst, rva);
         } else if ( t[x].flags & FLAGS_OPERPREFIX){
				// set flag
				//sprintf("op pfx");
				b += 1 + DisasmAddress(&c[1], n, inst, rva);
			} else if ( t[x].flags & FLAGS_ADDRPREFIX){
				// set flag
				//sprintf("addr pfx");
				b += 1 + DisasmAddress(&c[1], n, inst, rva);
			} else b += InstDecode( t[x], &c[b], inst); 
	} else if ( t[x].flags != 0 )
			  b += DisasmAddress(&c[1], t[x].flags, inst, rva);
	else 
			  strcpy(inst->mnemonic, "invalid\n"); 
	        //A change this to appropriate 'invalid opcode' behavior 
			  //....set flags and such
	return b;
}
