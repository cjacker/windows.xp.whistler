
        title  "Interval Clock Interrupt"
;++
;
; Copyright (c) 2000  Microsoft Corporation
;
; Module Name:
;
;    cb2stall.asm
;
; Abstract:
;
;    This module implements the code necessary to field and process the
;    interval clock interrupt.
;
; Author:
;
;    Shie-Lin Tzong (shielint) 12-Jan-1990
;
; Environment:
;
;    Kernel mode only.
;
; Revision History:
;
;   bryanwi 20-Sep-90
;
;       Add KiSetProfileInterval, KiStartProfileInterrupt,
;       KiStopProfileInterrupt procedures.
;       KiProfileInterrupt ISR.
;       KiProfileList, KiProfileLock are declared here.
;
;   shielint 10-Dec-90
;       Add performance counter support.
;       Move system clock to irq8, ie we now use RTC to generate system
;         clock.  Performance count and Profile use timer 1 counter 0.
;         The interval of the irq0 interrupt can be changed by
;         KiSetProfileInterval.  Performance counter does not care about the
;         interval of the interrupt as long as it knows the rollover count.
;       Note: Currently I implemented 1 performance counter for the whole
;       i386 NT.
;
;   John Vert (jvert) 11-Jul-2000
;       Moved from ke\i386 to hal\i386.  Removed non-HAL stuff
;
;   shie-lin tzong (shielint) 13-March-92
;       Move System clock back to irq0 and use RTC (irq8) to generate
;       profile interrupt.  Performance counter and system clock use time1
;       counter 0 of 8254.
;
;   Landy Wang (corollary!landy) 04-Dec-92
;       Move much code into separate modules for easy inclusion by various
;       HAL builds.
;
;--

.386p
        .xlist
include hal386.inc
include callconv.inc                    ; calling convention macros
include i386\ix8259.inc
include i386\kimacro.inc
include mac386.inc
include i386\ixcmos.inc
include cbus.inc
        .list

        EXTRNP  _HalEndSystemInterrupt,2
        EXTRNP  _HalBeginSystemInterrupt,3
if DBG
        EXTRNP  _HalDisplayString,1
endif
ifdef CBC_REV1
        EXTRNP  _Cbus2RequestSoftwareInterrupt,1
endif

;
; Constants used to initialize timer 0
;

TIMER1_DATA_PORT0       EQU     40H     ; Timer1, channel 0 data port
TIMER1_CONTROL_PORT0    EQU     43H     ; Timer1, channel 0 control port
TIMER2_DATA_PORT0       EQU     48H     ; Timer1, channel 0 data port
TIMER2_CONTROL_PORT0    EQU     4BH     ; Timer1, channel 0 control port
TIMER1_IRQ              EQU     0       ; Irq 0 for timer1 interrupt

COMMAND_8254_COUNTER0   EQU     00H     ; Select count 0
COMMAND_8254_RW_16BIT   EQU     30H     ; Read/Write LSB firt then MSB
COMMAND_8254_MODE2      EQU     4       ; Use mode 2
COMMAND_8254_BCD        EQU     0       ; Binary count down
COMMAND_8254_LATCH_READ EQU     0       ; Latch read command

PERFORMANCE_FREQUENCY   EQU     10000000

REGISTER_B_ENABLE_PERIODIC_INTERRUPT EQU     01000010B
                                        ; RT/CMOS Register 'B' Init byte
                                        ; Values for byte shown are
                                        ;  Bit 7 = Update inhibit
                                        ;  Bit 6 = Periodic interrupt enable
                                        ;  Bit 5 = Alarm interrupt disable
                                        ;  Bit 4 = Update interrupt disable
                                        ;  Bit 3 = Square wave disable
                                        ;  Bit 2 = BCD data format
                                        ;  Bit 1 = 24 hour time mode
                                        ;  Bit 0 = Daylight Savings disable

REGISTER_B_DISABLE_PERIODIC_INTERRUPT EQU    00000010B

;
; RegisterAInitByte sets 8Hz clock rate, used during init to set up
; KeStallExecutionProcessor, etc.  (See RegASystemClockByte below.)
;

RegisterAInitByte       EQU     00101101B ; RT/CMOS Register 'A' init byte
                                        ; 32.768KHz Base divider rate

                                        ;  8Hz int rate, period = 125.0ms
PeriodInMicroSecond     EQU     125000  ;

CMOS_CONTROL_PORT       EQU     70h     ; command port for cmos
CMOS_DATA_PORT          EQU     71h     ; cmos data port
D_INT032                EQU     8E00h   ; access word for 386 ring 0 int gate

;
; ==== Values used for System Clock ====
;


_DATA   SEGMENT  DWORD PUBLIC 'DATA'

;
; The following array stores the per microsecond loop count for each
; central processor.
;

        public HalpPerfCounterLow, HalpPerfCounterHigh
if DBG
        public CbusClockCount
        public CbusClockLate
        public _CbusCatchClock
endif
Cbus2PerfInit                   dd      0
HalpPerfCounterLow              dd      0
HalpPerfCounterHigh             dd      0
if DBG
CbusClockCount                  dd      0
CbusClockLate                   dd      0
_CbusCatchClock                 dd      0
endif

        public HalpCurrentRollOver, HalpCurrentTimeIncrement
HalpCurrentRollOver             dd      0
HalpCurrentTimeIncrement        dd      0


;
; Convert the interval to rollover count for 8254 Timer1 device.
; Timer1 counts down a 16 bit value at a rate of 1.193181667M counts-per-sec.
;
; The best fit value closest to 10ms is 10.0144012689ms:
;   ROLLOVER_COUNT      11949
;   TIME_INCREMENT      100144
;   Calculated error is -.0109472 s/day
;
;
; The following table contains 8254 values timer values to use at
; any given ms setting from 1ms - 15ms.  All values work out to the
; same error per day (-.0109472 s/day).
;

        public HalpRollOverTable

        ;                    RollOver   Time
        ;                    Count      Increment   MS
HalpRollOverTable       dd      1197,   10032       ;  1 ms
                        dd      2394,   20064       ;  2 ms
                        dd      3591,   30096       ;  3 ms
                        dd      4767,   39952       ;  4 ms
                        dd      5964,   49984       ;  5 ms
                        dd      7161,   60016       ;  6 ms
                        dd      8358,   70048       ;  7 ms
                        dd      9555,   80080       ;  8 ms
                        dd     10731,   89936       ;  9 ms
                        dd     11949,  100144       ; 10 ms
                        dd     13125,  110000       ; 11 ms
                        dd     14322,  120032       ; 12 ms
                        dd     15519,  130064       ; 13 ms
                        dd     16695,  139920       ; 14 ms
                        dd     17892,  149952       ; 15 ms

TimeIncr equ    4
RollOver equ    0

        public HalpLargestClockMS, HalpNextMSRate, HalpPendingMSRate
HalpLargestClockMS      dd      10      ; Table goes to 15MS, but since we
                                        ; use only 486 & above, limit to 10ms.
HalpNextMSRate          dd      0
HalpPendingMSRate       dd      0

_DATA   ends


INIT    SEGMENT DWORD PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

if DBG
RTC_toofast     db      'RTC IRQ8 HARDWARE ERROR\n', 0
endif

        page ,132
        subttl  "Initialize Clock"
;++
;
; VOID
; Cbus2InitializeClock (
;    )
;
; Routine Description:
;
;    This routine initialize system time clock using 8254 timer1 counter 0
;    to generate an interrupt at every 15ms interval at 8259 irq0.
;
;    See the definitions of TIME_INCREMENT and ROLLOVER_COUNT if clock rate
;    needs to be changed.
;
;    All processors call this routine, but only the first call needs
;    to do anything.
;
; Arguments:
;
;    None
;
; Return Value:
;
;    None.
;
;--
cPublicProc _Cbus2InitializeClock      ,0
        cmp     dword ptr PCR[PcHal.PcrNumber], 0

        jz      @f
        stdRET  _Cbus2InitializeClock

@@:
        mov     eax, HalpLargestClockMS
        mov     ecx, HalpRollOverTable.TimeIncr
        mov     edx, HalpRollOverTable[eax*8-8].TimeIncr
        mov     eax, HalpRollOverTable[eax*8-8].RollOver

        mov     HalpCurrentTimeIncrement, edx

;
; (ecx) = Min time_incr
; (edx) = Max time_incr
; (eax) = max roll over count
;

        push    eax
        stdCall _KeSetTimeIncrement, <edx, ecx>
        pop     ecx

        pushfd                          ; save caller's eflag
        cli                             ; make sure interrupts are disabled

;
; Set clock rate
; (ecx) = RollOverCount
;

        mov     al,COMMAND_8254_COUNTER0+COMMAND_8254_RW_16BIT+COMMAND_8254_MODE2
        out     TIMER1_CONTROL_PORT0, al ;program count mode of timer 0
        IoDelay
        mov     al, cl
        out     TIMER1_DATA_PORT0, al   ; program timer 0 LSB count
        IoDelay
        mov     al,ch
        out     TIMER1_DATA_PORT0, al   ; program timer 0 MSB count

        popfd                             ; restore caller's eflag
        mov     HalpCurrentRollOver, ecx  ; Set RollOverCount & initialized

        ;
        ; Set up the performance counter mechanism as well:
        ; Zero the global system timer for all of the processors.
        ;
        mov     eax, dword ptr [_Cbus2TimeStamp]
        mov     dword ptr [eax], 0
        mov     Cbus2PerfInit, 1                        ; calibration done

        stdRET    _Cbus2InitializeClock

stdENDP _Cbus2InitializeClock

INIT   ends

_TEXT$03   SEGMENT DWORD PUBLIC 'CODE'
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING

        page ,132
        subttl  "System Clock Interrupt"
;++
;
; Routine Description:
;
;    This routine is entered as the result of an interrupt generated by CLOCK.
;    Its function is to dismiss the interrupt, raise system Irql to
;    CLOCK2_LEVEL, update performance counter and transfer control to the
;    standard system routine to update the system time and the execution
;    time of the current thread and process.
;
; Arguments:
;
;    None
;    Interrupt is disabled
;
; Return Value:
;
;    Does not return, jumps directly to KeUpdateSystemTime, which returns
;
;    Sets Irql = CLOCK2_LEVEL and dismisses the interrupt
;
;--
        ENTER_DR_ASSIST Hci_a, Hci_t

cPublicProc _Cbus2ClockInterrupt     ,0

;
; Save machine state in trap frame
;

        ENTER_INTERRUPT Hci_a, Hci_t

;
; (esp) - base of trap frame
;

ifdef MCA

;
; Special hack for MCA machines
;

        in      al, 61h
        jmp     $+2
        or      al, 80h
        out     61h, al
        jmp     $+2

endif   ; MCA


ifdef CBC_REV1
	;
	; because we can miss an interrupt due to a hardware bug in the
	; CBC rev 1 silicon, send ourselves an IPI on every clock.
	; since we don't know when we've missed one, this will ensure
	; we don't cause lock timeouts if nothing else!
	;

        stdCall _Cbus2RequestSoftwareInterrupt, <IPI_LEVEL>
endif

;
; Dismiss interrupt and raise irq level to clock2 level
;

Hci10:
        push    _CbusClockVector
        sub     esp, 4                  ; allocate space to save OldIrql
        stdCall   _HalBeginSystemInterrupt, <CLOCK2_LEVEL, _CbusClockVector, esp>

if DBG
        inc     dword ptr [CbusClockCount]
endif
	;
	; Update our software 64-bit performance counter and zero the
	; 32-bit high resolution CBC timestamp counter.  we'd like to
	; read off the real ammount of elapsed time, ie:
	;
        ;       mov     eax, dword ptr [ecx]
        ;       add     HalpPerfCounterLow, eax
	;
	; but we can't because NT holds off interrupts for long periods
	; of time, ie: like in DbgLoadImageSymbols() for over
	; 700 _milliseconds_ !!!.  so just fib like all the other HALs do,
        ; and tell NT only 10ms have gone by, even when we really know it's
        ; been more than that in some instances.  otherwise, routines like
        ; KiCheckTimerTable() will fail.
	;
        mov     ecx, dword ptr [_Cbus2TimeStamp]
        mov     dword ptr [ecx], 0

if DBG
        ;
        ; we had a problem where clock interrupts are getting
        ; held off for approximately 700 ms once per second!  here is
        ; the debug code which caught DbgLoadImageSymbols.  if you want
        ; to turn it back on, you must also enable code above which
        ; captures the Cbus2 free running 100ns timer into eax.
        ;
        ;cmp     _CbusCatchClock, 0      ; debug breakpoint desired?
        ;je      short @f

        ;cmp     eax, 2000000           ; if more than 200 milliseconds since
                                        ; the last clockintr, then trigger
                                        ; the analyzer and go into debug
        ;jb      short @f
        ;inc     dword ptr [CbusClockLate]       ; trigger analyzer
        ;int     3

        ;
        ; zero out the time stamp since we spent a bunch of time in debug
        ;
        ;mov     ecx, dword ptr [_Cbus2TimeStamp]
        ;mov     dword ptr [ecx], 0
@@:
endif

        mov     eax, HalpCurrentTimeIncrement
        add     HalpPerfCounterLow, eax
        adc     HalpPerfCounterHigh, dword ptr 0

Hci30:

;
; (esp)   = OldIrql
; (esp+4) = Vector
; (esp+8) = base of trap frame
; ebp = trap frame
; eax = time increment
;
        cmp     HalpNextMSRate, 0       ; New clock rate desired?
        jz      _KeUpdateSystemTime@0   ; No, process tick

;
; Time of clock frequency is being changed.  See if the 8254 was
; was reprogrammed for a new rate during last tick
;
        cmp     HalpPendingMSRate, 0    ; Was a new rate set durning last
        jnz     short Hci50             ; tick?  Yes, go update globals

Hci40:
; (eax) = time increment for current tick

;
; A new clock rate needs to be set.  Setting the rate here will
; cause the tick after the next tick to be at the new rate.
; (the next tick is already in progress by the 8254 and will occur
; at the same rate as this tick)
;
        mov     ebx, HalpNextMSRate
        mov     HalpPendingMSRate, ebx  ; pending rate

        mov     ecx, HalpRollOverTable[ebx*8-8].RollOver

	;
	; Set clock rate
	; (ecx) = RollOverCount
	;

        push    eax                     ; save current tick's rate

        mov     al,COMMAND_8254_COUNTER0+COMMAND_8254_RW_16BIT+COMMAND_8254_MODE2
        out     TIMER1_CONTROL_PORT0, al ;program count mode of timer 0
        IoDelay
        mov     al, cl
        out     TIMER1_DATA_PORT0, al   ; program timer 0 LSB count
        IoDelay
        mov     al,ch
        out     TIMER1_DATA_PORT0, al   ; program timer 0 MSB count

        pop     eax
        jmp     _KeUpdateSystemTime@0   ; dispatch this tick

Hci50:
;
; The next tick will occur at the rate which was programmed during the last
; tick. Update globals for new rate which starts with the next tick.
;
; (eax) = time increment for current tick
;
        mov     ebx, HalpPendingMSRate
        mov     ecx, HalpRollOverTable[ebx*8-8].RollOver
        mov     edx, HalpRollOverTable[ebx*8-8].TimeIncr

        mov     HalpCurrentRollOver, ecx
        mov     HalpCurrentTimeIncrement, edx   ; next tick rate
        mov     HalpPendingMSRate, 0    ; no longer pending, clear it

        cmp     ebx, HalpNextMSRate     ; new rate == NextRate?
        jne     short Hci40             ; no, go set new pending rate

        mov     HalpNextMSRate, 0       ; we are at this rate, clear it
        jmp     short Hci30             ; process this tick

Hci100:
        add     esp, 8                  ; spurious, no EndOfInterrupt
        SPURIOUS_INTERRUPT_EXIT         ; exit interrupt without eoi

stdENDP _Cbus2ClockInterrupt

;++
;
; ULONG
; Cbus2SetTimeIncrement (
;     IN ULONG DesiredIncrement
;     )
;
; /*++
;
; Routine Description:
;
;    This routine initializes the system time clock to generate an
;    interrupt at every DesiredIncrement interval.  No lock synchronization
;    is needed here, since it is done by the executive prior to calling us.
;
; Arguments:
;
;     DesiredIncrement - desired interval between every timer tick (in
;                        100ns unit.)
;
; Return Value:
;
;     The *REAL* time increment set.
;--
cPublicProc _Cbus2SetTimeIncrement,1

        mov     eax, [esp+4]                ; desired setting
        xor     edx, edx
        mov     ecx, 10000
        div     ecx                         ; round to MS

        cmp     eax, HalpLargestClockMS     ; MS > max?
        jc      short @f
        mov     eax, HalpLargestClockMS     ; yes, use max
@@:
        or      eax, eax                    ; MS < min?
        jnz     short @f
        inc     eax                         ; yes, use min
@@:
        mov     HalpNextMSRate, eax

        mov     eax, HalpRollOverTable[eax*8-8].TimeIncr
        stdRET  _Cbus2SetTimeIncrement

stdENDP _Cbus2SetTimeIncrement

        page ,132
        subttl  "Query Performance Counter"
;++
;
; LARGE_INTEGER
; Cbus2QueryPerformanceCounter (
;    OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
;    )
;
; Routine Description:
;
;    This routine returns current 64-bit performance counter and,
;    optionally, the Performance Frequency.
;
;    Note this routine can NOT be called at Profiling interrupt
;    service routine.  Because this routine depends on IRR0 to determine
;    the actual count.
;
;    Also note that the performace counter returned by this routine
;    is not necessary the value when this routine is just entered.
;    The value returned is actually the counter value at any point
;    between the routine is entered and is exited.
;
; Arguments:
;
;    PerformanceFrequency [TOS+4] - optionally, supplies the address
;        of a variable to receive the performance counter frequency.
;
; Return Value:
;
;    Current value of the performance counter will be returned.
;
;--


;
; Parameter definitions
;

KqpcFrequency   EQU     [esp+4]        ; User supplied Performance Frequence

cPublicProc _Cbus2QueryPerformanceCounter      ,1

	;
	; First check to see if the performance counter has been initialized
        ; yet.  Since the kernel debugger calls KeQueryPerformanceCounter to
        ; support the !timer command, we need to return something reasonable
        ; before performance counter calibration has occured.
	;
        cmp     Cbus2PerfInit, 0        ; calibration finished yet?
        jne     @f                      ; yes, we can read the perf counter

	;
	; Initialization hasn't occured yet, so just return zeroes.
	;
        mov     eax, 0
        mov     edx, 0
        jmp     ret_freq

@@:
	;
	; Merge our software 64-bit performance counter and the
	; 32-bit high resolution CBC timestamp counter into edx:eax
	;

        mov     edx, HalpPerfCounterHigh
        mov     eax, HalpPerfCounterLow
        mov     ecx, dword ptr [_Cbus2TimeStamp]
        mov     ecx, dword ptr [ecx]

        ;
        ; re-read the global counters until we see we didn't get a torn read.
        ;
        cmp     edx, HalpPerfCounterHigh
        jne     @b

        cmp     eax, HalpPerfCounterLow
        jne     @b

	;
	; since the time stamp register and our 64-bit software register
	; are already both in 100ns units, there's no need for conversion here.
	;
        add     eax, ecx
        adc     edx, dword ptr 0

ret_freq:

	;
        ; return value is in edx:eax, return the performance counter
	; frequency if the caller wants it.
	;

        or      dword ptr KqpcFrequency, 0      ; is it a NULL variable?
        jz      short @f                        ; it's NULL, so bail

        mov     ecx, KqpcFrequency              ; (ecx)-> Frequency variable

        ;
        ; Set frequency to a hardcoded clock speed of 66Mhz for now.
        ;
        mov     DWORD PTR [ecx], PERFORMANCE_FREQUENCY
        mov     DWORD PTR [ecx+4], 0

@@:
        stdRET    _Cbus2QueryPerformanceCounter

stdENDP _Cbus2QueryPerformanceCounter

_TEXT$03   ends

; CMOS_READ
;
; Description: This macro read a byte from the CMOS register specified
;        in (AL).
;
; Parameter: (AL) = address/register to read
; Return: (AL) = data
;

CMOS_READ       MACRO
        OUT     CMOS_CONTROL_PORT,al    ; ADDRESS LOCATION AND DISABLE NMI
        IODelay                         ; I/O DELAY
        IN      AL,CMOS_DATA_PORT       ; READ IN REQUESTED CMOS DATA
        IODelay                         ; I/O DELAY
ENDM

;
; CMOS_WRITE
;
; Description: This macro read a byte from the CMOS register specified
;        in (AL).
;
; Parameter: (AL) = address/register to read
;            (AH) = data to be written
;
; Return: None
;

CMOS_WRITE      MACRO
        OUT     CMOS_CONTROL_PORT,al    ; ADDRESS LOCATION AND DISABLE NMI
        IODelay                         ; I/O DELAY
        MOV     AL,AH                   ; (AL) = DATA
        OUT     CMOS_DATA_PORT,AL       ; PLACE IN REQUESTED CMOS LOCATION
        IODelay                         ; I/O DELAY
ENDM

INIT    SEGMENT DWORD PUBLIC 'CODE'     ; Start 32 bit code
        ASSUME  DS:FLAT, ES:FLAT, SS:NOTHING, FS:NOTHING, GS:NOTHING


        page ,132
        subttl  "Initialize Stall Execution Counter"
;++
;
; VOID
; Cbus2InitializeStall (
;    IN CCHAR ProcessorNumber
;    )
;
; Routine Description:
;
;    This routine initialize the per Microsecond counter for
;    KeStallExecutionProcessor.  Note that the additional processors
;    execute this loop in Phase1, so they are already getting clock
;    interrupts as well as the RTC interrupts they expect from this
;    routine.  We should disable clock interrupts during this period
;    to ensure a really accurate result, but it should be "good enough"
;    for now.
;
; Arguments:
;
;    ProcessorNumber - Processor Number
;
; Return Value:
;
;    None.
;
;--

KiseInterruptCount      equ     [ebp-12] ; local variable

cPublicProc _Cbus2InitializeStall     ,1

        push    ebp                     ; save ebp
        mov     ebp, esp                ; set up 12 bytes for local use
        sub     esp, 12

        pushfd                          ; save caller's eflag

        ;
        ; Initialize Real Time Clock to interrupt us every 125ms at
        ; IRQ 8.
        ;

        cli                             ; make sure interrupts are disabled

        ;
        ; Since RTC interrupt will come from IRQ 8, we need to save
        ; the original irq 8 descriptor and set the descriptor to point to
        ; our own handler.
        ;

        sidt    fword ptr [ebp-8]       ; get IDT address
        mov     ecx, [ebp-6]            ; (edx)->IDT

        ;
        ; the profile vector varies for each platform
        ;
        mov     eax, dword ptr [_ProfileVector]

        shl     eax, 3                  ; 8 bytes per IDT entry
        add     ecx, eax                ; now at the correct IDT RTC entry

        push    dword ptr [ecx]         ; (TOS) = original desc of IRQ 8
        push    dword ptr [ecx + 4]     ; each descriptor has 8 bytes

        ;
        ; Pushing the appropriate entry address now (instead of
        ; the IDT start address later) to make the pop at the end simpler.
        ; we actually will retrieve this value twice, but only pop it once.
        ;
        push    ecx                     ; (TOS) -> &IDT[HalProfileVector]

        ;
        ; No need to get and save current interrupt masks - only IPI
        ; and software interrupts are enabled at this point in the kernel
        ; startup.  since other processors are still in reset, the profile
        ; interrupt is the only one we will see.  we really don't need to save
        ; the old IDT[PROFILE_VECTOR] entry either, by the way - it's just
        ; going to be overwritten in HalInitSystem Phase 1 anyway.
        ;
        ; note we are not saving edx before calling this function

        stdCall _HalEnableSystemInterrupt,<dword ptr [_ProfileVector],PROFILE_LEVEL,0>

        mov     ecx, dword ptr [esp]    ; restore IDT pointer

        mov     eax, offset FLAT:RealTimeClockHandler

        mov     word ptr [ecx], ax              ; Lower half of handler addr
        mov     word ptr [ecx+2], KGDT_R0_CODE  ; set up selector
        mov     word ptr [ecx+4], D_INT032      ; 386 interrupt gate

        shr     eax, 16                 ; (ax)=higher half of handler addr
        mov     word ptr [ecx+6], ax

        mov     dword ptr KiseinterruptCount, 0 ; set no interrupt yet

        stdCall   _HalpAcquireCmosSpinLock      ; intr disabled

        mov     ax,(RegisterAInitByte SHL 8) OR 0AH ; Register A
        CMOS_WRITE                      ; Initialize it

        ;
        ; register C _MUST_ be read before register B is initialized,
        ; otherwise an interrupt which was already pending will happen
        ; immediately.
        ;
        mov     al,0CH                  ; Register C
        CMOS_READ                       ; Read to initialize
        ;
        ; Don't clobber the Daylight Savings Time bit in register B, because we
        ; stash the LastKnownGood "environment variable" there.
        ;
        mov     ax, 0bh
        CMOS_READ
        and     al, 1
        mov     ah, al
        or      ah, REGISTER_B_ENABLE_PERIODIC_INTERRUPT
        mov     al, 0bh
        CMOS_WRITE                      ; Initialize it

        mov     al,0DH                  ; Register D
        CMOS_READ                       ; Read to initialize
        mov     dword ptr [KiseInterruptCount], 0

        stdCall   _HalpReleaseCmosSpinLock

        ;
        ; Now enable the interrupt and start the counter
        ; (As a matter of fact, only IRQ8 can come through.)
        ;

        xor     eax, eax                ; (eax) = 0, initialize loopcount
if DBG
        out     090h, al                        ; tell analyzer our count
endif
        sti

        align   4
kise10:
        add     eax, 1                  ; increment the loopcount
        jnz     short kise10

        ;
        ; Counter overflowed
        ; BUGBUG shielint Should be KeBugCheck
        ;

        stdCall   _DbgBreakPoint

        ;
        ; Our RealTimeClock interrupt handler.  The control comes here through
        ; irq 8.
        ; Note: we discard the first real time clock interrupt and compute the
        ;       permicrosecond loopcount on receiving of the second real time
        ;       interrupt.  This is because the first interrupt is generated
        ;       based on the previous real time tick interval.
        ;

RealTimeClockHandler:

if DBG
        out     090h, al                        ; tell analyzer our count
endif
        inc     dword ptr KiseInterruptCount ; increment interrupt count
        cmp     dword ptr KiseInterruptCount,1 ; Is this the first interrupt?
        jnz     short kise25            ; no, its the second go process it
        pop     eax                     ; get rid of original ret addr
        push    offset FLAT:kise10      ; set new return addr


        stdCall   _HalpAcquireCmosSpinLock      ; intr disabled

        mov     ax,(RegisterAInitByte SHL 8) OR 0AH ; Register A
        CMOS_WRITE                      ; Initialize it

        ;
        ; register C _MUST_ be read before register B is initialized,
        ; otherwise an interrupt which was already pending will happen
        ; immediately.
        ;
        mov     al,0CH                  ; Register C
        CMOS_READ                       ; Read to initialize

        ;
        ; Don't clobber the Daylight Savings Time bit in register B, because we
        ; stash the LastKnownGood "environment variable" there.
        ;
        mov     ax, 0bh
        CMOS_READ
        and     al, 1
        mov     ah, al
        or      ah, REGISTER_B_ENABLE_PERIODIC_INTERRUPT
        mov     al, 0bh
        CMOS_WRITE                      ; Initialize it

        mov     al,0DH                  ; Register D
        CMOS_READ                       ; Read to initialize

	mov     eax, _ProfileVector     ; mark interrupting vec
        CBUS_EOI eax                    ; destroy eax

        xor     eax, eax                ; reset loop counter

        stdCall   _HalpReleaseCmosSpinLock

        iretd

        align   4
kise25:


if DBG
	;
	; ** temporary - check for incorrect KeStallExecutionProcessorLoopCount
	;
        cmp     eax, 0
        jnz     short @f
        stdCall   _DbgBreakPoint

        ; never return
	;
	; ** End temporary code
	;

@@:
endif
        xor     edx, edx                ; (edx:eax) = divident
        mov     ecx, PeriodInMicroSecond; (ecx) = time spent in the loop
        div     ecx                     ; (eax) = loop count per microsecond
        cmp     edx, 0                  ; Is remainder =0?
        jz      short kise40            ; yes, go kise40
        inc     eax                     ; increment loopcount by 1

        align   4
kise40:

        mov     PCR[PcStallScaleFactor], eax

        ;
        ; Reset return address to kexit
        ;

        pop     eax                     ; discard original return address
        push    offset FLAT:kexit       ; return to kexit

	;
	; Shutdown the periodic RTC interrupt
	;
        stdCall   _HalpAcquireCmosSpinLock
        mov     ax,(RegisterAInitByte SHL 8) OR 0AH ; Register A
        CMOS_WRITE                      ; Initialize it
        mov     ax, 0bh
        CMOS_READ
        and     al, 1
        mov     ah, al
        or      ah, REGISTER_B_DISABLE_PERIODIC_INTERRUPT
        mov     al, 0bh
        CMOS_WRITE                      ; Initialize it
        mov     al,0CH                  ; Register C
        CMOS_READ                       ; dismiss pending interrupt
        stdCall   _HalpReleaseCmosSpinLock

        stdCall _HalDisableSystemInterrupt,<dword ptr [_ProfileVector],PROFILE_LEVEL>
	mov     eax, _ProfileVector     ; mark interrupting vec
        CBUS_EOI eax                    ; destroy eax

        and     word ptr [esp+8], NOT 0200H ; Disable interrupt upon return
        iretd

        align   4
kexit:                                  ; Interrupts are disabled

        pop     ecx                     ; (ecx) -> &IDT[HalProfileVector]
        pop     [ecx+4]                 ; restore higher half of RTC desc
        pop     [ecx]                   ; restore lower half of RTC desc

        popfd                           ; restore caller's eflags
        mov     esp, ebp
        pop     ebp                     ; restore ebp
        stdRET    _Cbus2InitializeStall

stdENDP _Cbus2InitializeStall

INIT    ends

        end
