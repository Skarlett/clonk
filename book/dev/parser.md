# xA0 Clonk Parsing
## Compiler Doc

This documentation is intended for the use of extending clonk's parsing ability and refers to the contents of `src/parser`, excluding `src/parser/lexer`. primarily describing `onk_parse`

# 0x00 Abstract
Clonk parses source documents by first tokenizing the source text into a stream of tokens. This stream of tokens is placed into `onk_parse`, this documentation concerns this function and its related implementation.

`onk_parse` will then return the stream of input tokens, but sorted in postfix notation. The stream of tokens returned will include "parser-generated" tokens, which are cannot be found direcly in the source document. "Parser-generated" tokens are later referred to as "logical operators" inside of this documentation. When this new stream of tokens is evaluated, it constructs the AST of the source document. This stage only deals with the source code conversion to postfix. 

Clonk accomplishes this task by using a custom variation of shunting yard, where the derivations are documented below.

## 0x21 Definitions 
| Words      | Definitions                                                                                                                                                                                      |
|------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| token      | a symbol/piece resembling a segment of the source code while denoting its type (word/int/operator symbol)                                                                                        |
| keywords   | a reserved WORD token that expressions an operation in the language                                                                                                                              |
| operator   | a symbol which manipulates the runtime-value of another symbol                                                                                                                                   |
| parsed unit | a token where its type is denoted as the following `ONK_WORD_TOKEN`, `ONK_STRING_LITERAL_TOKEN`, `ONK_INTEGER_TOKEN`, `ONK_TRUE_TOKEN`, `ONK_FALSE_TOKEN`, `ONK_NULL_TOKEN`                      |
| expression | a combination of units and operations as a collective segment                                                                                                                                    |
| terminator | a token which represents the end of a expression or block                                                                                                                                        |
| parse group | a group is the declaration of a literal collection-datatype (list/tuple/hashmap) where each item in the collection is followed by the appriotate terminator & begins with an opening brace (`{`/`(`/`[`/`${`) |
| codeblock  | a group but explicitly beginning with `{`                                                         |

## 0x10 Shunting Yard Review
First, lets quickly review of how shunting yard works, and then start building up to how 
clonk uses shunting yard as its primary parser.

Shunting yard takes an "infix" notation, and covert it into postfix (RPN)[https://wiki.com/p=RPN].
| infix         | postifx     |
|---------------|-------------|
| `(3 - 1) * 2` | `3 1 - 2 *` |
| `(a - b) * 2` | `a b - 2 *` |

Shunting yard is normally used to remove parentheses from a mathematical expression and preserving the correct order of operations to occur in the expression. 


## 0x11 Shunting-Yard Parsing
shunting yard algorithm uses a stack-datatype known in *Clonk* as the `operator_stack`. When processing input, if the current token isn't an operator, It will be added to the output array. If the current token is an operator then it will be added to the `operator_stack`. 

Before pushing the new operator (`operator_1`) onto the stack, it must have a lower precedence than the head of the `operator_stack` (`operator_2`).
Pop the head of the stack until the precedence of `operator_1` is less than `operator_2` or has the same precedence and is left assciotative. 

Infix mathematical expressions are as follow
| infix | postifx  |
|---|---|
| `1 + 2` | `1 1 +`  |
|`a - foo`| `a foo -` |
|`(3 - 1) * 2`| `3 1 - 2 *` |

While processing the token stream, if the current token is an opening parenthesis (`(`) , it will be placed into the `operator_stack` with its precendence set to `0`. negating the previously mentioned "lower-precedence" check from suceeding.

When the matching closing parenthesis is found (`)`), it will pop all the items out of the `operator_stack` into the output array until the matching opening parenthesis is found in the `operator-stack`. The matching opening brace (`(`) in the `operator-stack` is popped out of the stack, but not placed in the output array.


| postifx   | source             | `operator_stack` |
|-----------|--------------------|------------------|
|           |                    |                  |
|           | (3 - 2) * 1        |                  |
|           | <u>(</u>3 - 2) * 1 | (                |
| 3         | <u>3</u> - 2) * 1  | (                |
| 3         | <u>-</u> 1) * 2    | (-               |
| 3         | <u>1</u>) * 2      | (-               |
| 3 1       | <u>)</u> * 2       | <u>(-<u>         |
| 3 1 -     | <u> * </u> 2       |                  |
| 3 1 -     | <u>2</u>           | *                |
| 3 1 - 2   |                    | <u> * </u>       |
| 3 1 - 2 * |                    |                  |


## 0x12 Postfix Evaluation

Once the postfix expression is created, they're relatively easy to evaluate. This will use another stack. When the current token is an operator, take N operands from the stack, and compute their sum. Once completed, we'll take that new sum, and place it at the top of the stack.
N will always be equal to `2` in our example, since all our operators take `2` arguments.


| source    | stack |
|-----------|-------|
| 3 1 - 2 * |       |
| 1 - 2 *   | 3     |
| - 2 *     | 3 1   |
| 2 *       | 2     |
| *         | 2 2   |
|           | 4     |


## 0x20 Clonk Parsing

Clonk defines two definitions inside the parser, which will be referred to later on.

Clonk Extends the ability of shunting yard by bringing in the following ideas, and rules.

### 0x22 parse-units
**Where integers were added to the output array immediately, clonk does this to all parsed units.**

### 0x23 operations
We first start by introducing the operator-precedence table.

operations are treated the same as the shunting yard parsing algorithm.
The opening braces are all labeled as `0` precedence, effectively creating a new scope for operators to occur in.

precendense table:
| token                | precedense | assciotation     |
|----------------------|------------|------------------|
| `) } }`              | 126        | Non-assciotative |
| `.`                  | 13         | Left             |
| `^`                  | 12         | Right            |
| `/ * %`              | 11         | Left             |
| `+ -`                | 10         | Left             |
| `<< >>`              | 9          | Left             |
| `> < >= <=`          | 8          | Left             |
| `== !=`              | 7          | Left             |
| `&`                  | 6          | Left             |
| `\|`                 | 5          | Left             |
| `&&`                 | 4          | Left             |
| `\|\|`               | 3          | Left             |
| `== &= ~= += -= \|=` | 2          | Left             |
| `{ [ ( ${`           | 0          | Non-assciotitve  |
|                      |            |                  |

One of the first ideas you'll notice, if you've never written a language before (like me.) Is that `.` is an operator.
I have decided to call this operation `onk_ast_op_access` ("Access").


During this stage, we only validate grammer rules, so you can imagine that something like `a = (b+c) = d` will easily pass, but make no reasonable sense. This is a problem for a later stage.

## 0x24 Clonk Expressive grouping

First, we define the **Grouping mechanism**, where we can represent a *collection* of units in postfix (RPN) notation.
Take the following example.
```
Fig.1

    |-- Group/Boundary beginning
    v
a = [1, 2, 3];
      ^
      |-- Terminator
```
Infix notation represents grouping in a natural way we all understand, with easily identifiable boundaries, and termination.

*Clonk* chooses to represent this idea in the postfix output as parser-generated tokens. 

*Clonk* adds a token to the output array describing the kind of collection, and the amount of elements it pops off of the stack for evaluation.

Using the previous (`Fig.1`) expression, it'd be represented as the following in *Clonk's* postfix notation (`Fig.2`).

```
Fig.2

a  1 2 3 ListGroup(3)  =
a (1 2 3 ListGroup(3)) =


a (1 2 3 ListGroup(3)) =
^  ^----------------^  ^
|  |                |  |
|--+----------------+--| 
a= [ 1     2      3 ]

```

When the example postfix expression is evaluated, it will first place all *4* (`a`, `1`, `2`, `3`) values onto the stack.
**Every group** takes *N* items from the stack. `ListGroup(n=3)` Will pop `3 2 1`

The items popped when evaluating will then be used to help construct the `ListGroup` type, which will then be placed on the stack, replacing the 3 items & `ListGroup` with its self. This process is handled in the next stage. 

Parser-generated tokens that represent groups are placed onto the output directly the matching closing brace is found. The "parser-generated" tokens for groups are **never** placed in the `operator_stack`. (Their respective logical operators are though. I hope this doesn't cause too much confusion).

With the introduction of grouped expressions, a way is needed to determine the beginning and end of expressions. 

### Expression Termination
**Terminators** (based on the group start) flush the parser's `operator_stack` onto the output until the head of of the `operator_stack` is the group's opening brace. 


If a terminator is the matching closing brace, it will preceed as it normally would, mentioned previously.

| group start | terminator  | example               |
|-------------|-------------|-----------------------|
| `${`        | `:` `,` `}` | `${x: y, z: d}`       |
| `{`         | `;` `}`     | `{ foo(); }`          |
| `[`         | `:` `,` `]` | `[1, 2, 3, 4][1:2:3]` |
| `(`         | `,` `)`     | `(1, 2)`              |

## 0x25 Clonk Logical-Operators

Additionally, "parser-generated" tokens were added to describe logistic expressions. Logical expressions include function calls, if/else, and others listed below. 

**Logical operators are placed on the** `operator_stack` before the group's opening brace. If a group's matching end-brace is found the head of the `operator_stack` is checked to determine if a logical operator is present. If so, its popped from the operator stack, and placed into the output. This behavior can see in the table below on step 10.

|step| `operator_stack` | source          | postfix                           |
|----|------------------|-----------------|-----------------------------------|
| 1  |                  | `foo(a + 1, b)` |                                   |
| 2  |                  | `(a + 1, b)`    | `foo`                             |
| 3  | `Apply` `(`      | `a + 1, b)`     | `foo`                             |
| 4  | `Apply` `(`      | `+ 1, b)`       | `foo` `a`                         |
| 5  | `Apply` `(` `+`  | `1, b)`         | `foo` `a`                         |
| 6  | `Apply` `(` `+`  | `, b)`          | `foo` `a` `1`                     |
| 7  | `Apply` `(`      | `b)`            | `foo` `a` `1` `+`                 |
| 8  |                  | `)`             | `foo` `a` `1` `+` `b`             |
| 9  | `Apply`          |                 | `foo` `a` `1` `+` `b` `TupleGroup(2)`|
| 10 |                  |                 | `foo` `a` `1` `+` `b` `TupleGroup(2)` `Apply` |


The logical operators `Apply` (`foo(arg)`), `IndexAccess` (`foo[idx]`) and struct initalization `Type {x=y}` are determined when processing the opening brace if the previous token is `ONK_WORD_TOKEN`, or another closing brace. (`foo(x)(y)` or `foo[idx](arg)`)




### If logical-operator


| step | `operator_stack`      | source         | postfix                          |
|------|-----------------------|----------------|----------------------------------|
| 1    |                       | `if(a) { .. }` |                                  |
| 2    | `ifBody` `ifCond`     | `(a)`          |                                  |
| 3    | `ifBody` `ifCond` `(` | `a) { .. }`    |                                  |
| 4    | `ifBody` `ifCond` `(` | `) { .. }`     | `a`                              |
| 5    | `ifBody`              | `{ .. }`       | `a` `ifCond`                     |
| 6    | `ifBody` `{`          | ` .. }`        | `a` `ifCond`                     |
| 7    | `ifBody` `{`          | `}`            | `a` `ifCond` `..`                |
| 8    |                       |                | `a` `ifCond` `..` `CodeBlock(N)` |


