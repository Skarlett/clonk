# Clonk Parsing
## Compiler Doc

This documentation is intended for the use of extending clonk's parsing ability and refers to the contents of `src/parser`, excluding `src/parser/lexer`.

# 0x00 Introduction
Clonk parses source documents by using an extended variation of (shunting yard)[https://wiki.com/p=shunting_yard].
`onk_parse` will return the token stream, but in postfix notation.


## 0x10 Shunting-Yard review
First, lets quickly review of how shunting yard works, and then start building up to how 
clonk uses shunting yard as its primary parser.

Shunting yard takes an "infix" notation, and covert it into postfix (RPN)[https://wiki.com/p=RPN].
Shunting yard is normally used to remove parentheses from a mathematical expression, 
while still preserving the correct order of operations. 
The goal of shunting yard is to make complex infix expressions 
easily parsable & computationally minimal.

| infix | postifx  |
|---|---|
|`(3 - 1) * 2`| `3 1 - 2 *` |
|`(a - b) * 2`| `a b - 2 *` |


The way it accomplishes this task is two-fold. 


## 0x11 Shunting-Yard Parsing
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
From there, its operator-precedense will be set to `0`. negating the previously mentioned `operator_2` check from suceeding.


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

## 0x24 Parsing primitive group

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

Clonk chooses to represent this idea in the postfix output as parser-generated tokens. 
Each token is a different group with the number of entries attached as meta-data, based on the enclosing braces used.
Taken the previous expression, it'd be represented as the following.

```
Fig.2

a  1 2 3 ListGroup(3)  =
a (1 2 3 ListGroup(3)) =
```

When the postfix expression is evaluated, it will first place all *4* values onto the stack.
**Every group** takes *N* items from the stack. `ListGroup(n=3)` Will pop `3 2 1`
before wrapping them inside of its self, and placing `onk_expr_list_t` on the stack. Finally binding `a` 
to the `onk_expr_list_t` value. (See Fig.3)


```
Fig.3

a (1 2 3 ListGroup(3)) =
^  ^----------------^  ^
|  |                |  |
|--+----------------+--| 
a= [ 1     2      3 ]
```

with a little bit of deeper intutition, you also find that expressions can work inside of groups `[1 + 2, 3]`, 
inside the parser, you will find that it flushes the `operator_stack` whenever a terminator is reached `,` and `}` in this case; 


## 0x25 Clonk Group Operator

The second adjustment to the shunting yard is the occurance of group operators. 

Group operators are placed into the operator stack, 
and once the group has gotten its terminating brace, 
the group-operator is then applied to the output array.

group operators are used when certain patterns are used, such as function calls `any_word(x, y)` (`Apply`), group access `foo[idx]` (`Indexaccess`), and structure initalization `Type { x=[1, 2] }` (`StructInit`).


### Example input
```
foo(x, y)
```

### Example output
```
        
foo x y TupleGroup(2) Apply
~~~ ~~~               ~~~~~
 |  ^ arg expr        ^ Group Operator
 |
 +-- function name 
```



The second big outline to 

While searching through the source of `include/lexer.h`, you will come across various `MARKER_*` group. 













