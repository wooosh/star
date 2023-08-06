; involution16.fasm - fasm definitions for the involution16 ISA

element register
element r0? : register + 0
element r1? : register + 1
element r2? : register + 2
element r3? : register + 3
element r4? : register + 4
element r5? : register + 5
element r6? : register + 6
element r7? : register + 7
element r8? : register + 8
element r9? : register + 9
element rA? : register + 10
element rB? : register + 11
element rC? : register + 12
element rD? : register + 13
element rE? : register + 14
element rF? : register + 15

macro macro_insns_rrr?
  macro ?! name, op
    if `name = "macro_insns_rrr_end"
      purge ?
    else
      macro name? regx, regy, regz
        db (op shl 4) + regx metadata 1 - register, ((regy metadata 1 - register) shl 4) + regz metadata 1 - register
      end macro
    end if
  end macro
end macro

macro_insns_rrr
  add, 0h
  sub, 1h
  ror, 2h
  rol, 3h
  shr, 4h
  shl, 5h
  and, 6h
  ora, 7h
  mul, 8h
  div, 9h
  cmp, 0xA
  jeq, 0xB
macro_insns_rrr_end

macro xri? regx, imm
  db 0xC0 + regx metadata 1 - register, imm
end macro

element srrCode
element p.CDAB? : srrCode + 0
element p.DCBA? : srrCode + 1
element p.BACD? : srrCode + 2
element p.BADC? : srrCode + 3
element p.ACBD? : srrCode + 4
element p.DBCA? : srrCode + 5
element p.ADCB? : srrCode + 6
element p.CBAD? : srrCode + 7

macro srr? regx, regy, code
  db 0xD0 + regx metadata 1 - register, ((regy metadata 1 - register) shl 4) + code metadata 1 - srrCode
end macro

macro srm? regx, regy
  db 0xE0 + regx metadata 1 - register, (regy metadata 1 - register) shl 4
end macro

macro brk?
  dw 0xFFFF
end macro

macro dbg? x
  db 0xF0 + x, 0xF0 + x
end macro