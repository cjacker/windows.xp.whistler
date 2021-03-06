/***********************************************************************
* Microsoft Lego
*
* Microsoft Confidential.  Copyright 1994 Microsoft Corporation.
*
* Component:
*
* File: mips.h
*
* File Comments:
*
*
***********************************************************************/

struct _IMAGE_RUNTIME_FUNCTION_ENTRY;


enum TRMTMIPS
{
   trmtmipsUnknown,
   trmtmipsFallThrough,
   trmtmipsBraInd,
   trmtmipsCallInd,
   trmtmipsTrap,
   trmtmipsTrapCc,
   trmtmipsBraDef,
   trmtmipsBraIndDef,
   trmtmipsBraCcDef,
   trmtmipsBraCcLikely,
   trmtmipsCallDef,
   trmtmipsCallIndDef,
   trmtmipsCallCcDef,
   trmtmipsCallCcLikely,
};


union MIPSIW			       // MIPS Instruction Word
{
   DWORD    dw;

   struct
   {
      DWORD Target : 26;
      DWORD Opcode : 6;
   } j_format;

   struct
   {
      DWORD Uimmediate : 16;
      DWORD Rt : 5;
      DWORD Rs : 5;
      DWORD Opcode : 6;
   } u_format;

   struct
   {
      DWORD Function : 6;
      DWORD Re : 5;
      DWORD Rd : 5;
      DWORD Rt : 5;
      DWORD Rs : 5;
      DWORD Opcode : 6;
   } r_format;

   struct
   {
      DWORD Function : 6;
      DWORD Re : 5;
      DWORD Rd : 5;
      DWORD Rt : 5;
      DWORD Format : 4;
      DWORD Fill1 : 1;
      DWORD Opcode : 6;
   } f_format;

   struct
   {
      DWORD Function : 6;
      DWORD Fd : 5;
      DWORD Fs : 5;
      DWORD Ft : 5;
      DWORD Format : 4;
      DWORD Fill1 : 1;
      DWORD Opcode : 6;
   } c_format;
};


class DISMIPS : public DIS
{
public:
	    DISMIPS(ARCHT = archtR4000);

	    // Methods inherited from DIS

	    ADDR Addr() const;
	    ADDR AddrJumpTable() const;
	    ADDR AddrAddress() const;
	    ADDR AddrTarget() const;
	    ARCHT Archt() const;
	    size_t Cb() const;
	    size_t CbDisassemble(ADDR, const BYTE *, size_t);
	    size_t CbGenerateLoadAddress(BYTE *, size_t, size_t * = NULL) const;
	    size_t CbJumpEntry() const;
	    size_t CbMemoryReference() const;
	    size_t CchFormatAddr(ADDR, char *, size_t) const;
	    size_t CchFormatBytes(char *, size_t) const;
	    size_t CchFormatBytesMax() const;
	    size_t CchFormatInstr(char *, size_t) const;
	    MEMREFT Memreft() const;
	    PFNCCHADDR PfncchaddrSet(PFNCCHADDR);
	    PFNCCHFIXUP PfncchfixupSet(PFNCCHFIXUP);
	    void *PvClient() const;
	    void *PvClientSet(void *);
	    TRMT Trmt() const;
	    TRMTA Trmta() const;

#ifdef DEBUG
	    void Dump() const;
	    BOOL FValidate() const;
#endif

private:
	    enum OPCLS		       // Operand Class
	    {
	       opclsNone,	       // No operand
	       opclsRegRs,	       // General purpose register Rs
	       opclsRegRt,	       // General purpose register Rt
	       opclsRegRd,	       // General purpose register Rd
	       opclsImmRt,	       // Immediate value of Rt
	       opclsImmRe,	       // Immediate value of Re
	       opclsImm,	       // Immediate value
	       opclsMem,	       // Memory reference
	       opclsMem_w,	       // Memory reference
	       opclsMem_r,	       // Memory reference
	       opclsAddrBra,	       // Branch instruction target
	       opclsAddrJmp,	       // Jump instruction target
	       opclsCprRt,	       // Coprocessor general register Rt
	       opclsCprRd,	       // Coprocessor general register Rd
	       opclsRegFs,	       // Floating point general register Fs
	       opclsRegFt,	       // Floating point general register Ft
	       opclsRegFd,	       // Floating point general register Fd
	    };

	    enum ICLS		       // Instruction Class
	    {
		    // Invalid Class

	       iclsInvalid,

		    // Immediate Class
		    //
		    // Text Format:	    ADDIU   rt,rs,immediate
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    Rt

	       iclsImmediate,

		    // Immediate Class
		    //
		    // Text Format:	    ADDI    rt,rs,immediate
		    //
		    // Termination Type:    trmtmipsTrapCc
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    Rt

	       iclsImmTrapCc,

		    // Immediate (BraCc-1) Class
		    //
		    // Text Format:	    BEQ     rt,rs,Target
		    //
		    // Termination Type:    trmtmipsBraCcDef
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:

	       iclsImmBraCc1,

		    // Immediate (BraCc-2) Class
		    //
		    // Text Format:	    BEQL    rt,rs,Target
		    //
		    // Termination Type:    trmtmipsBraCcLikely
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:

	       iclsImmBraCc2,

		    // Immediate (BraCc-3) Class
		    //
		    // Text Format:	    BGTZ    rs,Target
		    //
		    // Termination Type:    trmtmipsBraCcDef
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Constraints:	    Rt must be zero

	       iclsImmBraCc3,

		    // Immediate (BraCc-4) Class
		    //
		    // Text Format:	    BGTZL   rs,Target
		    //
		    // Termination Type:    trmtmipsBraCcLikely
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Constraints:	    Rt must be zero

	       iclsImmBraCc4,

		    // Immediate (BraCc-5) Class
		    //
		    // Text Format:	    BGEZ    rs,Target
		    //
		    // Termination Type:    trmtmipsBraCcDef
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Note:		    The Rt field is a function code

	       iclsImmBraCc5,

		    // Immediate (BraCc-6) Class
		    //
		    // Text Format:	    BGEZL   rs,Target
		    //
		    // Termination Type:    trmtmipsBraCcLikely
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Note:		    The Rt field is a function code

	       iclsImmBraCc6,

		    // Immediate (CallCc-1) Class
		    //
		    // Text Format:	    BGEZAL  rs,Target
		    //
		    // Termination Type:    trmtmipsCallCcDef
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    R31
		    //
		    // Note:		    The Rt field is a function code

	       iclsImmCallCc1,

		    // Immediate (CallCc-2) Class
		    //
		    // Text Format:	    BGEZALL rs,Target
		    //
		    // Termination Type:    trmtmipsCallCcLikely
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    R31
		    //
		    // Note:		    The Rt field is a function code

	       iclsImmCallCc2,

		    // Immediate (CACHE) Class
		    //
		    // Text Format:	    CACHE   op,offset(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Note:		    The Rt field stores the opcode

	       iclsImmCache,

		    // Immediate (Load) Class
		    //
		    // Text Format:	    LB	    rt,offset(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    Rt

	       iclsImmLoad,

		    // Immediate (Load Coprocessor) Class
		    //
		    // Text Format:	    LDC0    rt,offset(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    Coprocessor general register Rt

	       iclsImmLoadCp,

		    // Immediate (LUI) Class
		    //
		    // Text Format:	    LUI     rt,immediate
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:
		    // Registers Set:	    Rt
		    //
		    // Note:		    The Rs field is unused (UNDONE: Must be zero?)

	       iclsImmLui,

		    // Immediate (Store) Class
		    //
		    // Text Format:	    SB	    rt,offset(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:

	       iclsImmStore,

		    // Immediate (SC) Class
		    //
		    // Text Format:	    SC	    rt,offset(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:	    Rt

	       iclsImmSc,

		    // Immediate (Store Coprocessor) Class
		    //
		    // Text Format:	    SDC0    rt,offset(rs)
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs, Coprocessor general register Rt
		    // Registers Set:

	       iclsImmStoreCp,

		    // Immediate (Trap) Class
		    //
		    // Text Format:	    TEQI    rs,immediate
		    //
		    // Termination Type:    trmtmipsTrapCc
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Note:		    The Rt field is a function code

	       iclsImmTrap,

		    // Jump Class
		    //
		    // Text Format:	    J	    Target
		    //
		    // Termination Type:    trmtmipsBraDef
		    //
		    // Registers Used:
		    // Registers Set:

	       iclsJump,

		    // Jump (JAL) Class
		    //
		    // Text Format:	    JAL     Target
		    //
		    // Termination Type:    trmtmipsCallDef
		    //
		    // Registers Used:
		    // Registers Set:	    R31

	       iclsJumpJal,

		    // Register Class
		    //
		    // Text Format:	    ADDU    rd,rs,rt
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    Shift ammount must be zero

	       iclsRegister,

		    // Register Class
		    //
		    // Text Format:	    ADD     rd,rs,rt
		    //
		    // Termination Type:    trmtmipsTrapCc
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    Shift ammount must be zero

	       iclsRegTrapCc,

		    // Register (BREAK) Class
		    //
		    // Text Format:	    BREAK   immediate
		    //
		    // Termination Type:    trmtmipsTrap
		    //
		    // Registers Used:
		    // Registers Set:
		    //
		    // Note:		    MIPS does not use an operand for the immediate

	       iclsRegBreak,

		    // Register (JALR) Class
		    //
		    // Text Format:	    JALR    rd,rs
		    //
		    // Termination Type:    trmtmipsCallInd
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    Rt and shift ammount must be zero

	       iclsRegJalr,

		    // Register (JR) Class
		    //
		    // Text Format:	    JR	    rs
		    //
		    // Termination Type:    trmtmipsBraIndDef
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Constraints:	    Rd, Rt, and shift ammount must be zero

	       iclsRegJr,

		    // Register (MFHI) Class
		    //
		    // Text Format:	    MFHI    rd
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    HI
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    Rs, Rt, and shift ammount must be zero

	       iclsRegMfhi,

		    // Register (MFLO) Class
		    //
		    // Text Format:	    MFLO    rd
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    LO
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    Rs, Rt, and shift ammount must be zero

	       iclsRegMflo,

		    // Register (MTHI) Class
		    //
		    // Text Format:	    MTHI    rs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    HI
		    //
		    // Constraints:	    Rt, Rd, and shift ammount must be zero

	       iclsRegMthi,

		    // Register (MTLO) Class
		    //
		    // Text Format:	    MTLO    rs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs
		    // Registers Set:	    LO
		    //
		    // Constraints:	    Rt, Rd, and shift ammount must be zero

	       iclsRegMtlo,

		    // Register (Multiply-Divide) Class
		    //
		    // Text Format:	    DDIV    rs,rt
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:	    HI, LO
		    //
		    // Constraints:	    Rd and shift ammount must be zero

	       iclsRegMulDiv,

		    // Register (Shift) Class
		    //
		    // Text Format:	    DSLL    rd,rt,sa
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rt
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    The Rs field must be zero

	       iclsRegShift,

		    // Register (Shift Variable) Class
		    //
		    // Text Format:	    DSLLV   rd,rt,rs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:	    Rd
		    //
		    // Constraints:	    Shift ammount must be zero

	       iclsRegShiftVar,

		    // Register (SYNC) Class
		    //
		    // Text Format:	    SYNC
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:
		    // Registers Set:
		    //
		    // Constraints:	    Rs, Rt, Rd, and shift ammount must be zero

	       iclsRegSync,

		    // Register (SYSCALL) Class
		    //
		    // Text Format:	    SYSCALL
		    //
		    // Termination Type:    trmtmipsTrap
		    //
		    // Registers Used:
		    // Registers Set:
		    //
		    // Constraints:	    Rs, Rt, Rd, and shift ammount must be zero

	       iclsRegSyscall,

		    // Register (Trap) Class
		    //
		    // Text Format:	    TEQ     rs,rt,immediate
		    //
		    // Termination Type:    trmtmipsTrapCc
		    //
		    // Registers Used:	    Rs, Rt
		    // Registers Set:
		    //
		    // Note:		    Rd and shift ammount contain the immediate
		    // Note:		    MIPS does not use an operand for the immediate

	       iclsRegTrap,

		    // Immediate (BraCc-7) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    BCzF    Target
		    //
		    // Termination Type:    trmtmipsBraCcDef
		    //
		    // Registers Used:
		    // Registers Set:
		    //
		    // Note:		    The coprocessor z condition is referenced
		    // Note:		    The Rs and Rt fields are function codes
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsImmBraCc7,

		    // Register (CFCz) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    CFCz    rt,rd
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor control register Rd
		    // Registers Set:	    Rt
		    //
		    // Constraints:	    Shift ammount and function must be zero
		    //
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsRegCfc,

		    // Register (CTCz) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    CTCz    rt,rd
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rt
		    // Registers Set:	    Coprocessor control register Rd
		    //
		    // Constraints:	    Shift ammount and function must be zero
		    //
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsRegCtc,

		    // Register (MFCz) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    DMFCz   rt,rd
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general register Rd
		    // Registers Set:	    Rt
		    //
		    // Constraints:	    Shift ammount and function must be zero
		    //
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsRegMfc,

		    // Register (MTCz) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    DMTCz   rt,rd
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Rt
		    // Registers Set:	    Coprocessor general register Rd
		    //
		    // Constraints:	    Shift ammount and function must be zero
		    //
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsRegMtc,

		    // Register (Cp0) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    TLBP
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:
		    // Registers Set:
		    //
		    // Constraints:	    This is valid for coprocessor 0 only
		    // Constraints:	    Rs must be 10000b
		    // Constraints:	    Rt, Rd, and shift ammount must be zero
		    //
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsRegCp0,

		    // Register (ERET) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    ERET
		    //
		    // Termination Type:    trmtmipsBraInd
		    //
		    // Registers Used:
		    // Registers Set:
		    //
		    // Constraints:	    This is valid for coprocessor 0 only
		    // Constraints:	    Rs must be 10000b
		    // Constraints:	    Rt, Rd, and shift ammount must be zero
		    //
		    // Note:		    The coprocessor must be set in the mnemonic

	       iclsRegEret,

		    // Register (Float-1) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    ADD.S  fd,fs,ft
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general registers Fs and Ft
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    Format must be Single or Double

	       iclsRegFloat1,

		    // Register (Float-2) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    SQRT.S fd,fs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general register Fs
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    Format must be Single or Double
		    // Constraints:	    Ft must be zero

	       iclsRegFloat2,

		    // Register (Float-3) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    MOV.S  fd,fs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general registers Fs and Ft
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    Format must be Single or Double or Word
		    // Constraints:	    Ft must be zero

	       iclsRegFloat3,

		    // Register (Float-4) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    CVT.S  fd,fs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general registers Fs and Ft
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    Format must be Double or Word
		    // Constraints:	    Ft must be zero

	       iclsRegFloat4,

		    // Register (Float-5) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    CVT.D  fd,fs
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general registers Fs and Ft
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    Format must be Single or Word
		    // Constraints:	    Ft must be zero

	       iclsRegFloat5,

		    // Register (Float-6) Class
		    //
		    // Coprocessor
		    //
		    // Text Format:	    C.F.S  fs,ft
		    //
		    // Termination Type:    trmtmipsFallThrough
		    //
		    // Registers Used:	    Coprocessor general register Fs
		    // Registers Set:	    Coprocessor general register Fd
		    //
		    // Constraints:	    Format must be Single or Double
		    // Constraints:	    Fd must be zero

	       iclsRegFloat6,
	    };

	    struct CLS
	    {
	       BYTE	   trmtmips;
	       BYTE	   opcls1;	  // Operand class for operand 1
	       BYTE	   opcls2;	  // Operand class for operand 2
	       BYTE	   opcls3;	  // Operand class for operand 3
	    };

	    struct OPCD
	    {
	       const char  *szMnemonic;
	       BYTE	   icls;
	    };

   static   const TRMT mptrmtmipstrmt[];

   static   const CLS rgcls[];

   static   const OPCD rgopcd[];
   static   const OPCD rgopcdSpecial[];
   static   const OPCD rgopcdRegimm[];
   static   const OPCD rgopcdCop[];
   static   const OPCD rgopcdBc[];
   static   const OPCD rgopcdCp0[];
   static   const OPCD rgopcdCp1[];

   static   const char rgszFormat[5][4];
   static   const char *rgszGpr[32];

	    void FormatOperand(char *sz, OPCLS opcls) const;
   static   const OPCD *PopcdDecode(MIPSIW);
	    TRMTMIPS Trmtmips() const;

	    ARCHT m_archt;

	    PFNCCHADDR m_pfncchaddr;
	    PFNCCHFIXUP m_pfncchfixup;
	    void *m_pvClient;

	    ADDR m_addr;
	    MIPSIW m_mipsiw;
	    const OPCD *m_popcd;
};
