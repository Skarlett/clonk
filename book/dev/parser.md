# Clonk Parsing
## Compiler Doc

This documentation is intended for the use of extending clonk's parsing ability and refers to the contents of `src/parser`, excluding `src/parser/lexer`.

## Fundamentals
Clonk parses source documents by using an extended variation of (shunting yard)[https://wiki.com/p=shunting_yard].

### Shunting-Yard review
First, lets quickly review of how shunting yard works, and then start building up to how 
clonk uses shunting yard as its primary parser.

Shunting yard takes an "infix" notation, and covert it into postfix (RPN)[https://wiki.com/p=RPN].
Shunting yard is normally used to remove parentheses from a mathematical expression, 
while still preserving the correct order of operations. 
The goal of shunting yard is to make complex infix expressions 
easily parsable & computationally minimal.

The way it accomplishes this task is two-fold. 


#### Parsing
First, the parsing - uses a stack-datatype known in clonk as the `operator_stack`. When parsing, 
if the current token isn't an operator, It will be added to the output array. If the current token is an operator
then it will be added to the `operator_stack`. 

Before pushing the new operator (`operator_1`) onto the stack,
if the head of the stack (`operator_2`) has greater operator-precedence than `operator_1` or they have the same precedence 
and `operator_1` is left-associative, then pop `operator_2` out of the `operator_stack` and into the output array.

Finally, push `operator_1` onto the `operator_stack`.

Infix mathematical expressions are as follow
| infix | postifx  |
|---|---|
| `1 + 2` | `1 1 +`  |
|`a - foo`| `a foo -` |
|`(3 - 1) * 2`| `3 1 - 2 *` |

When shunting yards current token is an opening parenthesis (`(`) , it will be placed into the `operator_stack`.
From there, its operator-precedense will be set to `0`. This is so, as mentioned in the paragraph before; that when `operator_2` is checked for greater operator-precedence, it will always fail if the head of the `operator-stack` is `(`.

When the matching closing parenthesis is found (`)`), it will pop all the items out of the `operator_stack` into the output array until the matching opening parenthesis is found in the `operator-stack`. The matching opening brace (`(`) in the `operator-stack` is popped out of the stack, but not placed in the output array.

| postifx | source | `operator_stack` |
|---|---|---|
||(3 - 2) * 1||
||(3 - 2) * 1|(|
|3|(3 - 2) * 1|(|
|3|- 1) * 2|(-|
|3|1) * 2|(-|
|3 1|) * 2|(-|
|3 1 -|* 2||
|3 1 - |2|*|
|3 1 - 2||*|
|3 1 - 2 *|||



| infix | postifx  |
|---|---|
|`(3 - 1) * 2`| `3 1 - 2 *` |
|`(a - b) * 2`| `a b - 2 *` |

Clonk Extends the ability of shunting yard by bringing in the following ideas, and rules.



While searching through the source of `include/lexer.h`, you will come across various `MARKER_*` group. 












