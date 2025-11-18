#include "Lexer_Test.h"
#include "Demo.h"

int main()
{
    //Lexer_Test();
    Demo();
}

// <program>       :: = <stmt_list>
// 
// <stmt_list>     :: = <stmt> <stmt_list>
//                    | <stmt>
// 
// <stmt>          :: = <let_stmt>
//                    | <print_stmt>
//                    | <expr_stmt>
// 
// <let_stmt>      :: = "let" < identifier > "=" < expr > ";"
// 
// < print_stmt >  :: = "print" < expr > ";"
// 
// < expr_stmt >   :: = < expr> ";"
// 
// < expr > :: = <term>{ ("+" | "-") < term > }
// 
// <term>          :: = <factor>{ ("*" | "/") < factor > }
// 
// <factor>        :: = <number>
//                    | <identifier>
//                    | "(" < expr > ")"
// 
// < identifier >  :: = letter{ letter | digit }
// 
// <number>        :: = digit{ digit }
