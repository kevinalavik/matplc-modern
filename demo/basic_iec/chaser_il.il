

FUNCTION SCROLL_LEFT: INT
  (* Interface *)
  VAR_INPUT
    bits: INT;
    top_bit: INT;
  END_VAR

  LD bits
  MUL 2
  ST bits
  ST SCROLL_LEFT

  LT top_bit
  RETC

  LD bits
  ADD 1
  SUB top_bit
  ST SCROLL_LEFT
END_FUNCTION




FUNCTION SCROLL_RIGHT: INT
  VAR_INPUT
    bits: INT;
    top_bit: INT;
  END_VAR

  LD bits
  MOD 2
  MUL top_bit
  ADD bits
  DIV 2
  ST bits
  ST SCROLL_RIGHT
END_FUNCTION



PROGRAM SCROLL
  VAR_EXTERNAL
    left_bt  : BOOL;
    right_bt : BOOL;
    bits : INT;
  END_VAR
  VAR
    (* FALSE is right, TRUE is left , we do not yet support enums *)
    direction : BOOL;
  END_VAR

  LD left_bt
  R  direction
  LD right_bt
  S  direction

  LD direction
  JMPC sleft

  sright: LD bits
          SCROLL_RIGHT 16
          ST bits
          RET

  sleft: LD bits
         SCROLL_LEFT 16
	 ST bits
         RET
END_PROGRAM




CONFIGURATION config1
  VAR_GLOBAL
    bits AT %L : INT := 1;
    left_bt  AT %left  : BOOL;
    right_bt AT %right : BOOL;
   END_VAR

  PROGRAM main: SCROLL(left_bt := left_bt, right_bt := right_bt, bits := bits);
END_CONFIGURATION



