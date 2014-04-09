HCalc
=====

Hex Calc support Bitwise Operator &amp; Arithmetic
***
Usage: hcalc [ EXPRESSION ]

\>>> No Floating-Point Support <<<

Binding Key:

    q, Q, <CTRL-D>  - Quit
      <ESC>         - Clear current line

Support number:

    [bB]XXX         - Binary number
    [oO]XXX         - Octal number
    [dD]XXX         - Decimal number
    [hH]XXX         - Hexadecimal number
    [0x]XXX         - Hexadecimal number
    X[A-F]|[a-f]X   - Hexadecimal number
    0XXX            - Binary or Octal if there is number in X more than 1
    OTHERS          - Decimal if it is a number

Support Symbol:

    +, add, ADD     - arithmetic Addition     [priority lowest]
    -, sub, SUB     - arithmetic Subtraction  [priority lowest]
    *, mul, MUL     - arithmetic Multiply     [priority normal]
    /, div, DIV     - arithmetic Division     [priority normal]
    %, mod, MOD     - arithmetic remainder    [priority normal]
    &, and, AND     - bitwise logical AND     [priority high]
    |, or, OR       - bitwise logical OR      [priority high]
    ^, xor, XOR     - bitwise logical XOR     [priority high]
       nor, NOR     - bitwise logical NOR     [priority high]
    ~, not, NOT     - bitwise Inversion       [priority high]
       >>           - logical shift Right     [priority high]
       <<           - logical shift Left      [priority high]
       ( )          - brackets                [priority highest]
