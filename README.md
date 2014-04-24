HCalc
=====

Hex Calc support Bitwise Operator &amp; Arithmetic
***
Usage: hcalc [-i] [ EXPRESSION ]

\>>> No Floating-Point Support <<<

Option:

       -i           - Interactive

Binding Key:

    q, Q, <CTRL-D>  - Quit
       <ESC>        - Clear current line
    =, <Enter>      - Do calculate
       ?            - Show this help message

Support number:

    [bB]XXX         - Binary number
    [oO]XXX         - Octal number
    [dD]XXX         - Decimal number
    [hH]XXX         - Hexadecimal number
    [0x]XXX         - Hexadecimal number
    X[A-F]|[a-f]X   - Hexadecimal number
                      Add prefix ``0x'' to the Hexadecimal number which starting with 
                      the `[bB]' or `[dD]'
    0XXX            - Binary, or Octal if there is digitin X more than 1
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
