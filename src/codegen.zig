// Codegen version 1.00, proof of concept, will be replaced by a robust C Impl

const Parser = @cImport({
        @cInclude("parser.h");
        @cInclude("token.h");
        @cInclude("types.h");
});

const std = @import("std");

fn GenerateAddressForExpr(expr: ?*Parser.EXPRESSION) void
{
        if (expr == null)
                return;
        const e = expr.?;
        
        if (e.Type == Parser.EXPR_TYPE_VAR)
        {
                std.debug.print("\tlea esi, [{s}]\n", .{e.as.variable.Name});
                std.debug.print("\tpush esi\n", .{});
        }
        else if (e.Type == Parser.EXPR_TYPE_ACCESS)
        {
                GenerateForExpr(e.as.access.Expr);
                GenerateForExpr(e.as.access.Index);
                std.debug.print("\tpop ebx\n", .{});
                std.debug.print("\tpop eax\n", .{});
                std.debug.print("\tlea eax, [eax + ebx*4]\n", .{});
                std.debug.print("\tpush eax\n", .{});
        }
        else
        {
                std.debug.print("Error: Cannot assign to this expression type\n", .{});
        }
}

pub export fn GenerateForExpr(expr: ?*Parser.EXPRESSION) void
{
    if (expr == null)
    {
        return;
    }
    
    const e = expr.?;
    
    switch (e.Type)
    {
        Parser.EXPR_TYPE_NONE =>
        {},

        Parser.EXPR_TYPE_BINARY_OP =>
        {
                GenerateForExpr(e.as.binary.Lhs);
                GenerateForExpr(e.as.binary.Rhs);
                std.debug.print("\tpop ebx\n", .{});
                std.debug.print("\tpop eax\n", .{});
                switch (e.as.binary.Operator)
                {
                        Parser.TOKEN_EXPR_ADD =>
                        {
                                std.debug.print("\tadd eax, ebx\n", .{});
                        },

                        Parser.TOKEN_EXPR_SUB =>
                        {
                                std.debug.print("\tsub eax, ebx\n", .{});
                        },

                        Parser.TOKEN_EXPR_MUL =>
                        {
                                std.debug.print("\timul eax, ebx\n", .{});
                        },

                        Parser.TOKEN_EXPR_DIV =>
                        {
                                std.debug.print("\tcdq\n", .{});
                                std.debug.print("\tidiv ebx\n", .{});
                        },

                        Parser.TOKEN_EXPR_EQ =>
                        {
                                std.debug.print("\tcmp eax, ebx\n", .{});
                                std.debug.print("\tsete al\n", .{});
                                std.debug.print("\tmovzx eax, al\n", .{});
                        },

                        //Parser.TOKEN_EXPR_NE =>
                        //{
                        //    std.debug.print("\tcmp eax, ebx\n", .{});
                        //    std.debug.print("\tsetne al\n", .{});
                        //    std.debug.print("\tmovzx eax, al\n", .{});
                        //},
        //
                        //Parser.TOKEN_EXPR_LT =>
                        //{
                        //    std.debug.print("\tcmp eax, ebx\n", .{});
                        //    std.debug.print("\tsetl al\n", .{});
                        //    std.debug.print("\tmovzx eax, al\n", .{});
                        //},
        //
                        //Parser.TOKEN_EXPR_LE =>
                        //{
                        //    std.debug.print("\tcmp eax, ebx\n", .{});
                        //    std.debug.print("\tsetle al\n", .{});
                        //    std.debug.print("\tmovzx eax, al\n", .{});
                        //},
        //
                        //Parser.TOKEN_EXPR_GT =>
                        //{
                        //    std.debug.print("\tcmp eax, ebx\n", .{});
                        //    std.debug.print("\tsetg al\n", .{});
                        //    std.debug.print("\tmovzx eax, al\n", .{});
                        //},
        //
                        //Parser.TOKEN_EXPR_GE =>
                        //{
                        //    std.debug.print("\tcmp eax, ebx\n", .{});
                        //    std.debug.print("\tsetge al\n", .{});
                        //    std.debug.print("\tmovzx eax, al\n", .{});
                        //},

                        Parser.TOKEN_EXPR_AND =>
                        {
                                std.debug.print("\tand eax, ebx\n", .{});
                        },

                        Parser.TOKEN_EXPR_OR =>
                        {
                                std.debug.print("\tor eax, ebx\n", .{});
                        },

                        Parser.TOKEN_EXPR_XOR =>
                        {
                                std.debug.print("\txor eax, ebx\n", .{});
                        },

                        //Parser.TOKEN_EXPR_SHL =>
                        //{
                        //    std.debug.print("\txchg eax, ebx\n", .{});
                        //    std.debug.print("\tshl eax, cl\n", .{});
                        //},
        //
                        //Parser.TOKEN_EXPR_SHR =>
                        //{
                        //    std.debug.print("\txchg eax, ebx\n", .{});
                        //    std.debug.print("\tshr eax, cl\n", .{});
                        //},

                        else =>
                        {
                                std.debug.print("\t??? eax, ebx ; Unknown operator\n", .{});
                        },
                }
                std.debug.print("\tpush eax\n", .{});
        },

        Parser.EXPR_TYPE_UNARY_OP =>
        {
                GenerateForExpr(e.as.unary.Operand);
                std.debug.print("\tpop eax\n", .{});
                switch (e.as.unary.Operator)
                {
                        Parser.TOKEN_EXPR_SUB =>
                        {
                                std.debug.print("\tneg eax\n", .{});
                        },

                        Parser.TOKEN_EXPR_NOT =>
                        {
                                std.debug.print("\tnot eax\n", .{});
                        },

                        Parser.TOKEN_EXPR_COMPLEMENT =>
                        {
                                std.debug.print("\tnot eax\n", .{});
                        },

                        Parser.TOKEN_EXPR_MUL =>
                        {
                                std.debug.print("\tmov eax, [eax]\n", .{});
                        },

                        else =>
                        {
                                std.debug.print("\t??? eax ; Unknown unary operator\n", .{});
                        },
                }
                std.debug.print("\tpush eax\n", .{});
        },

        Parser.EXPR_TYPE_FUNCTION =>
        {
                std.debug.print("{s}:\n", .{e.as.fun.Name});
                std.debug.print("\tpush ebp\n", .{});
                std.debug.print("\tmov ebp, esp\n", .{});
                
                var param: ?*Parser.EXPRESSION = e.as.fun.Params;
                var offset: i32 = 8;
                
                while (param != null)
                {
                        const p = param.?;

                        if (p.*.Type == Parser.EXPR_TYPE_DECLARATION)
                        {
                                std.debug.print("\tmov [{s}], [ebp+{}]\n", .{p.*.as.declaration.Name, offset});
                        }
                        else
                        {
                                std.debug.print("\t; Warning: parameter is not a declaration\n", .{});
                        }
                        offset += 4;
                        param = p.*.Next;
                }
                
                GenerateForExpr(e.as.fun.Body);
                std.debug.print(".ex:\n", .{});
                std.debug.print("\tmov esp, ebp\n", .{});
                std.debug.print("\tpop ebp\n", .{});
                std.debug.print("\tret\n", .{});
        },

        Parser.EXPR_TYPE_VAR =>
        {
                std.debug.print("\tpush [{s}]\n", .{e.as.variable.Name});
        },

        Parser.EXPR_TYPE_LITERAL_NUM =>
        {
                std.debug.print("\tpush {}\n", .{e.as.integer_literal.Value});
        },

        Parser.EXPR_TYPE_LITERAL_REAL =>
        {
                std.debug.print("\tsub esp, 8\n", .{});
                std.debug.print("\tfld qword ptr {}\n", .{e.as.real_literal.Value});
                std.debug.print("\tfstp qword ptr [esp]\n", .{});
        },

        Parser.EXPR_TYPE_LITERAL_STR =>
        {
                std.debug.print("\tpush offset string_{}\n", .{e.as.string_literal.Value});
        },

        Parser.EXPR_TYPE_RETURN =>
        {
                GenerateForExpr(e.as.return_statement);
                std.debug.print("\tpop eax\n", .{});
                std.debug.print("\tjmp .ex\n", .{});
        },

        Parser.EXPR_TYPE_ASSIGNMENT =>
        {
                GenerateAddressForExpr(e.as.assignment.Lhs);
                GenerateForExpr(e.as.assignment.Rhs);
                std.debug.print("\tpop ebx\n", .{}); // value
                std.debug.print("\tpop eax\n", .{}); // address
                std.debug.print("\tmov [eax], ebx\n", .{});
        },

        Parser.EXPR_TYPE_CALL =>
        {
                var arg: ?*Parser.EXPRESSION = e.as.call.Args;
                var arg_count: usize = 0;
                
                while (arg != null)
                {
                        const a = arg.?;
                        arg_count += 1;
                        arg = a.Next;
                }
                
                arg = e.as.call.Args;
                var i: usize = 0;
                while (arg) |a| : (i += 1)
                {
                        GenerateForExpr(a);
                        arg = a.Next;
                }
                
                GenerateForExpr(e.as.call.Callee);
                std.debug.print("\tpop eax\n", .{});
                std.debug.print("\tcall eax\n", .{});
                
                if (arg_count > 0)
                {
                        std.debug.print("\tadd esp, {}\n", .{arg_count * 4});
                }
                
                std.debug.print("\tpush eax\n", .{});
        },

        Parser.EXPR_TYPE_DECLARATION =>
        {
                if (e.as.declaration.Init) |init|
                {
                        GenerateForExpr(init);
                        std.debug.print("\tpop eax\n", .{});
                        std.debug.print("\tmov [{s}], eax\n", .{e.as.declaration.Name});
                }
                else
                {
                        std.debug.print("\tmov dword ptr [{s}], 0\n", .{e.as.declaration.Name});
                }
        },

        Parser.EXPR_TYPE_IFELSE =>
        {
                const else_label = ".Lelse";
                const end_label = ".Lendif";
                
                GenerateForExpr(e.as.ifelse.Conditional);
                std.debug.print("\tpop eax\n", .{});
                std.debug.print("\ttest eax, eax\n", .{});
                std.debug.print("\tjz {s}\n", .{else_label});
                
                GenerateForExpr(e.as.ifelse.Body);
                std.debug.print("\tjmp {s}\n", .{end_label});
                
                std.debug.print("{s}:\n", .{else_label});
                if (e.as.ifelse.ElseBody) |else_body|
                {
                        GenerateForExpr(else_body);
                }
                
                std.debug.print("{s}:\n", .{end_label});
        },

        Parser.EXPR_TYPE_ACCESS =>
        {
                GenerateForExpr(e.as.access.Expr);
                GenerateForExpr(e.as.access.Index);
                std.debug.print("\tpop ebx\n", .{}); // index
                std.debug.print("\tpop eax\n", .{}); // base address
                std.debug.print("\tmov eax, [eax + ebx*4]\n", .{}); // assuming 4-byte elements
                std.debug.print("\tpush eax\n", .{});
        },

        Parser.EXPR_TYPE_STRUCTURE =>
        {
                std.debug.print("; Structure definition: {s}\n", .{e.as.structure.Name});
                // For structure definitions, we might just need to emit type information
                // or allocate space for the structure
                GenerateForExpr(e.as.structure.Body);
        },

        else =>
        {
                std.debug.print("Unexpected Expression type {}\n", .{e.Type});
        },
    }
    
    GenerateForExpr(if (e.Next != null) e.Next else null);
}

pub export fn GenerateStringLiterals(expr: ?*Parser.EXPRESSION) void {
    if (expr == null) return;
    
    const e = expr.?;
    
    switch (e.Type) {
        Parser.EXPR_TYPE_LITERAL_STR => {
            std.debug.print("string_{}: db \"{s}\", 0\n", .{e.as.string_literal.Value, e.as.string_literal.Data});
        },
        else => {},
    }
    
    GenerateStringLiterals(if (e.Next != null) e.Next else null);
}