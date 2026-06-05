#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_LEN 1024
#define MAX_STACK_SIZE 1024
#define MAX_TOKENS_COUNT 1024
#define MAX_TERMS_COUNT 512
#define MAX_VAR_LEN 64

const int PRIORITY_ADDITIVE = 2;
const int PRIORITY_MULTIPLICATIVE = 3;
const int PRIORITY_POWER = 4;
const int PRIORITY_UNARY = 5;

const double DOUBLE_EPSILON = 1e-9;

typedef enum {
    TOKEN_END,
    TOKEN_NUMBER,
    TOKEN_VARIABLE,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_POWER,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_UNARY_MINUS
} ExprTokenType;

typedef struct {
    ExprTokenType type;
    double number;
    char variable[MAX_VAR_LEN];
} Token;

typedef enum {
    NODE_NUMBER,
    NODE_VARIABLE,
    NODE_BINARY_OP,
    NODE_UNARY_MINUS
} NodeType;

typedef struct ExprNode {
    NodeType type;
    double number;
    char variable[MAX_VAR_LEN];
    char op;
    struct ExprNode *left;
    struct ExprNode *right;
} ExprNode;

typedef struct {
    double coefficient;
    char variable[MAX_VAR_LEN];
    bool is_simple;
    const ExprNode *complex_node;
} SumTerm;


static bool IsNearlyZero(double value) {
    return fabs(value) < DOUBLE_EPSILON;
}

static bool IsNearlyEqual(double a, double b) {
    return fabs(a - b) < DOUBLE_EPSILON;
}

static void PrintDouble(double value) {
    if (IsNearlyEqual(value, (int)value)) {
        printf("%d", (int)value);
    } else {
        printf("%g", value);
    }
}

static int GetTokenPriority(ExprTokenType type) {
    if (type == TOKEN_PLUS || type == TOKEN_MINUS) {
        return PRIORITY_ADDITIVE;
    }
    if (type == TOKEN_MULTIPLY || type == TOKEN_DIVIDE) {
        return PRIORITY_MULTIPLICATIVE;
    }
    if (type == TOKEN_POWER) {
        return PRIORITY_POWER;
    }
    if (type == TOKEN_UNARY_MINUS) {
        return PRIORITY_UNARY;
    }
    return 0;
}

static char ExprTokenTypeToChar(ExprTokenType type) {
    if (type == TOKEN_PLUS) {
        return '+';
    }
    if (type == TOKEN_MINUS) {
        return '-';
    }
    if (type == TOKEN_MULTIPLY) {
        return '*';
    }
    if (type == TOKEN_DIVIDE) {
        return '/';
    }
    if (type == TOKEN_POWER) {
        return '^';
    }
    if (type == TOKEN_UNARY_MINUS) {
        return '~';
    }
    return '?';
}

static bool IsOperandToken(ExprTokenType type) {
    return type == TOKEN_NUMBER || type == TOKEN_VARIABLE;
}

static bool IsOperatorToken(ExprTokenType type) {
    return type == TOKEN_PLUS || type == TOKEN_MINUS || type == TOKEN_MULTIPLY
           || type == TOKEN_DIVIDE || type == TOKEN_POWER || type == TOKEN_UNARY_MINUS;
}

static bool IsRightAssociative(ExprTokenType type) {
    return type == TOKEN_POWER || type == TOKEN_UNARY_MINUS;
}

static int Tokenize(const char *input, Token *tokens, int max_tokens) {
    int token_count = 0;
    int i = 0;
    bool prev_was_operand = false;

    while (input[i] != '\0') {
        if (isspace((unsigned char)input[i])) {
            ++i;
            continue;
        }

        if (token_count >= max_tokens - 1) {
            printf("Ошибка. Слишком много токенов.\n");
            return -1;
        }

        if (isdigit((unsigned char)input[i]) || (input[i] == '.' && isdigit((unsigned char)input[i + 1]))) {
            char *end_ptr = NULL;
            tokens[token_count].type = TOKEN_NUMBER;
            tokens[token_count].number = strtod(&input[i], &end_ptr);
            tokens[token_count].variable[0] = '\0';
            i += (int)(end_ptr - &input[i]);
            ++token_count;
            prev_was_operand = true;
            continue;
        }

        if (isalpha((unsigned char)input[i]) || input[i] == '_') {
            tokens[token_count].type = TOKEN_VARIABLE;
            tokens[token_count].number = 0.0;
            int j = 0;
            while ((isalnum((unsigned char)input[i]) || input[i] == '_') && j < MAX_VAR_LEN - 1) {
                tokens[token_count].variable[j] = input[i];
                ++j;
                ++i;
            }
            tokens[token_count].variable[j] = '\0';
            ++token_count;
            prev_was_operand = true;
            continue;
        }

        if (input[i] == '(') {
            tokens[token_count].type = TOKEN_LPAREN;
            tokens[token_count].number = 0.0;
            tokens[token_count].variable[0] = '\0';
            ++token_count;
            prev_was_operand = false;
            ++i;
            continue;
        }

        if (input[i] == ')') {
            tokens[token_count].type = TOKEN_RPAREN;
            tokens[token_count].number = 0.0;
            tokens[token_count].variable[0] = '\0';
            ++token_count;
            prev_was_operand = true;
            ++i;
            continue;
        }

        if (input[i] == '-' && !prev_was_operand) {
            tokens[token_count].type = TOKEN_UNARY_MINUS;
            tokens[token_count].number = 0.0;
            tokens[token_count].variable[0] = '\0';
            ++token_count;
            prev_was_operand = false;
            ++i;
            continue;
        }

        ExprTokenType op_type = TOKEN_END;
        if (input[i] == '+') {
            op_type = TOKEN_PLUS;
        } else if (input[i] == '-') {
            op_type = TOKEN_MINUS;
        } else if (input[i] == '*') {
            op_type = TOKEN_MULTIPLY;
        } else if (input[i] == '/') {
            op_type = TOKEN_DIVIDE;
        } else if (input[i] == '^') {
            op_type = TOKEN_POWER;
        } else {
            printf("Ошибка. Неизвестный символ '%c'.\n", input[i]);
            return -1;
        }

        tokens[token_count].type = op_type;
        tokens[token_count].number = 0.0;
        tokens[token_count].variable[0] = '\0';
        ++token_count;
        prev_was_operand = false;
        ++i;
    }

    tokens[token_count].type = TOKEN_END;
    tokens[token_count].number = 0.0;
    tokens[token_count].variable[0] = '\0';
    return token_count;
}

static int ShuntingYard(const Token *tokens, int token_count, Token *output, int max_output) {
    Token op_stack[MAX_STACK_SIZE];
    int stack_top = 0;
    int output_count = 0;

    for (int i = 0; i < token_count; ++i) {
        ExprTokenType type = tokens[i].type;

        if (IsOperandToken(type)) {
            if (output_count >= max_output) {
                printf("Ошибка. Переполнение выходного буфера.\n");
                return -1;
            }
            output[output_count] = tokens[i];
            ++output_count;
        } else if (type == TOKEN_LPAREN) {
            if (stack_top >= MAX_STACK_SIZE) {
                printf("Ошибка. Переполнение стека.\n");
                return -1;
            }
            op_stack[stack_top] = tokens[i];
            ++stack_top;
        } else if (type == TOKEN_RPAREN) {
            while (stack_top > 0 && op_stack[stack_top - 1].type != TOKEN_LPAREN) {
                if (output_count >= max_output) {
                    printf("Ошибка. Переполнение выходного буфера.\n");
                    return -1;
                }
                output[output_count] = op_stack[stack_top - 1];
                ++output_count;
                --stack_top;
            }
            if (stack_top == 0) {
                printf("Ошибка. Несовпадение скобок.\n");
                return -1;
            }
            --stack_top;
        } else if (IsOperatorToken(type)) {
            int cur_priority = GetTokenPriority(type);
            bool right_assoc = IsRightAssociative(type);
            while (stack_top > 0 && IsOperatorToken(op_stack[stack_top - 1].type)) {
                int top_priority = GetTokenPriority(op_stack[stack_top - 1].type);
                bool should_pop = right_assoc ? (top_priority > cur_priority) : (top_priority >= cur_priority);
                if (!should_pop) {
                    break;
                }
                if (output_count >= max_output) {
                    printf("Ошибка. Переполнение выходного буфера.\n");
                    return -1;
                }
                output[output_count] = op_stack[stack_top - 1];
                ++output_count;
                --stack_top;
            }
            if (stack_top >= MAX_STACK_SIZE) {
                printf("Ошибка. Переполнение стека.\n");
                return -1;
            }
            op_stack[stack_top] = tokens[i];
            ++stack_top;
        }
    }

    while (stack_top > 0) {
        if (op_stack[stack_top - 1].type == TOKEN_LPAREN) {
            printf("Ошибка. Несовпадение скобок.\n");
            return -1;
        }
        if (output_count >= max_output) {
            printf("Ошибка. Переполнение выходного буфера.\n");
            return -1;
        }
        output[output_count] = op_stack[stack_top - 1];
        ++output_count;
        --stack_top;
    }

    return output_count;
}

static void PrintPostfix(const Token *tokens, int count) {
    printf("ПОЛИЗ: ");
    for (int i = 0; i < count; ++i) {
        if (i > 0) {
            printf(" ");
        }
        if (tokens[i].type == TOKEN_NUMBER) {
            PrintDouble(tokens[i].number);
        } else if (tokens[i].type == TOKEN_VARIABLE) {
            printf("%s", tokens[i].variable);
        } else if (tokens[i].type == TOKEN_UNARY_MINUS) {
            printf("(~)");
        } else {
            printf("%c", ExprTokenTypeToChar(tokens[i].type));
        }
    }
    printf("\n");
}


static ExprNode *CreateNumberNode(double value) {
    ExprNode *node = (ExprNode *)malloc(sizeof(ExprNode));
    if (node == NULL) {
        fprintf(stderr, "Ошибка. Не удалось выделить память.\n");
        return NULL;
    }
    node->type = NODE_NUMBER;
    node->number = value;
    node->variable[0] = '\0';
    node->op = '\0';
    node->left = NULL;
    node->right = NULL;
    return node;
}

static ExprNode *CreateVariableNode(const char *name) {
    ExprNode *node = (ExprNode *)malloc(sizeof(ExprNode));
    if (node == NULL) {
        fprintf(stderr, "Ошибка. Не удалось выделить память.\n");
        return NULL;
    }
    node->type = NODE_VARIABLE;
    node->number = 0.0;
    strncpy(node->variable, name, MAX_VAR_LEN - 1);
    node->variable[MAX_VAR_LEN - 1] = '\0';
    node->op = '\0';
    node->left = NULL;
    node->right = NULL;
    return node;
}

static ExprNode *CreateBinaryOpNode(char op, ExprNode *left, ExprNode *right) {
    ExprNode *node = (ExprNode *)malloc(sizeof(ExprNode));
    if (node == NULL) {
        fprintf(stderr, "Ошибка. Не удалось выделить память.\n");
        return NULL;
    }
    node->type = NODE_BINARY_OP;
    node->number = 0.0;
    node->variable[0] = '\0';
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

static ExprNode *CreateUnaryMinusNode(ExprNode *operand) {
    ExprNode *node = (ExprNode *)malloc(sizeof(ExprNode));
    if (node == NULL) {
        fprintf(stderr, "Ошибка. Не удалось выделить память.\n");
        return NULL;
    }
    node->type = NODE_UNARY_MINUS;
    node->number = 0.0;
    node->variable[0] = '\0';
    node->op = '~';
    node->left = operand;
    node->right = NULL;
    return node;
}

static void FreeTree(ExprNode *node) {
    if (node == NULL) {
        return;
    }
    FreeTree(node->left);
    FreeTree(node->right);
    free(node);
}

static ExprNode *CopyTree(const ExprNode *node) {
    if (node == NULL) {
        return NULL;
    }
    ExprNode *copy = (ExprNode *)malloc(sizeof(ExprNode));
    if (copy == NULL) {
        fprintf(stderr, "Ошибка. Не удалось выделить память.\n");
        return NULL;
    }
    *copy = *node;
    copy->left = CopyTree(node->left);
    copy->right = CopyTree(node->right);
    return copy;
}

static ExprNode *BuildTreeFromPostfix(const Token *postfix, int count) {
    ExprNode *node_stack[MAX_STACK_SIZE];
    int stack_top = 0;

    for (int i = 0; i < count; ++i) {
        ExprTokenType type = postfix[i].type;

        if (type == TOKEN_NUMBER) {
            if (stack_top >= MAX_STACK_SIZE) {
                printf("Ошибка. Переполнение стека при построении дерева.\n");
                return NULL;
            }
            node_stack[stack_top] = CreateNumberNode(postfix[i].number);
            ++stack_top;
        } else if (type == TOKEN_VARIABLE) {
            if (stack_top >= MAX_STACK_SIZE) {
                printf("Ошибка. Переполнение стека при построении дерева.\n");
                return NULL;
            }
            node_stack[stack_top] = CreateVariableNode(postfix[i].variable);
            ++stack_top;
        } else if (type == TOKEN_UNARY_MINUS) {
            if (stack_top < 1) {
                printf("Ошибка. Недостаточно операндов для унарного минуса.\n");
                return NULL;
            }
            ExprNode *operand = node_stack[stack_top - 1];
            --stack_top;
            if (stack_top >= MAX_STACK_SIZE) {
                FreeTree(operand);
                printf("Ошибка. Переполнение стека при построении дерева.\n");
                return NULL;
            }
            node_stack[stack_top] = CreateUnaryMinusNode(operand);
            ++stack_top;
        } else if (IsOperatorToken(type)) {
            if (stack_top < 2) {
                printf("Ошибка. Недостаточно операндов для оператора.\n");
                return NULL;
            }
            ExprNode *right = node_stack[stack_top - 1];
            ExprNode *left = node_stack[stack_top - 2];
            stack_top -= 2;
            char op = ExprTokenTypeToChar(type);
            if (stack_top >= MAX_STACK_SIZE) {
                FreeTree(left);
                FreeTree(right);
                printf("Ошибка. Переполнение стека при построении дерева.\n");
                return NULL;
            }
            node_stack[stack_top] = CreateBinaryOpNode(op, left, right);
            ++stack_top;
        }
    }

    if (stack_top != 1) {
        printf("Ошибка. Некорректное выражение.\n");
        while (stack_top > 0) {
            --stack_top;
            FreeTree(node_stack[stack_top]);
        }
        return NULL;
    }

    return node_stack[0];
}


static bool IsConstantTree(const ExprNode *node) {
    if (node == NULL) {
        return true;
    }
    if (node->type == NODE_NUMBER) {
        return true;
    }
    if (node->type == NODE_VARIABLE) {
        return false;
    }
    if (node->type == NODE_UNARY_MINUS) {
        return IsConstantTree(node->left);
    }
    return IsConstantTree(node->left) && IsConstantTree(node->right);
}

static double EvaluateConstant(const ExprNode *node) {
    if (node->type == NODE_NUMBER) {
        return node->number;
    }
    if (node->type == NODE_UNARY_MINUS) {
        return -EvaluateConstant(node->left);
    }
    double left_val = EvaluateConstant(node->left);
    double right_val = EvaluateConstant(node->right);
    if (node->op == '+') {
        return left_val + right_val;
    }
    if (node->op == '-') {
        return left_val - right_val;
    }
    if (node->op == '*') {
        return left_val * right_val;
    }
    if (node->op == '/') {
        return left_val / right_val;
    }
    if (node->op == '^') {
        return pow(left_val, right_val);
    }
    return 0.0;
}

static void AnalyzeTerm(const ExprNode *node, SumTerm *term) {
    term->coefficient = 1.0;
    term->variable[0] = '\0';
    term->is_simple = false;
    term->complex_node = node;

    if (node == NULL) {
        return;
    }

    if (node->type == NODE_NUMBER) {
        term->coefficient = node->number;
        term->is_simple = true;
        term->complex_node = NULL;
        return;
    }

    if (node->type == NODE_VARIABLE) {
        term->coefficient = 1.0;
        strncpy(term->variable, node->variable, MAX_VAR_LEN - 1);
        term->variable[MAX_VAR_LEN - 1] = '\0';
        term->is_simple = true;
        term->complex_node = NULL;
        return;
    }

    if (node->type == NODE_UNARY_MINUS) {
        SumTerm inner;
        AnalyzeTerm(node->left, &inner);
        if (inner.is_simple) {
            term->coefficient = -inner.coefficient;
            strncpy(term->variable, inner.variable, MAX_VAR_LEN - 1);
            term->variable[MAX_VAR_LEN - 1] = '\0';
            term->is_simple = true;
            term->complex_node = NULL;
        }
        return;
    }

    if (node->type == NODE_BINARY_OP && node->op == '*') {
        bool left_is_num = node->left->type == NODE_NUMBER;
        bool right_is_num = node->right->type == NODE_NUMBER;
        bool left_is_var = node->left->type == NODE_VARIABLE;
        bool right_is_var = node->right->type == NODE_VARIABLE;

        if (left_is_num && right_is_var) {
            term->coefficient = node->left->number;
            strncpy(term->variable, node->right->variable, MAX_VAR_LEN - 1);
            term->variable[MAX_VAR_LEN - 1] = '\0';
            term->is_simple = true;
            term->complex_node = NULL;
            return;
        }

        if (right_is_num && left_is_var) {
            term->coefficient = node->right->number;
            strncpy(term->variable, node->left->variable, MAX_VAR_LEN - 1);
            term->variable[MAX_VAR_LEN - 1] = '\0';
            term->is_simple = true;
            term->complex_node = NULL;
            return;
        }
    }
}

static void ExtractSumTerms(const ExprNode *node, double sign, SumTerm *terms, int *term_count) {
    if (node == NULL || *term_count >= MAX_TERMS_COUNT) {
        return;
    }

    if (node->type == NODE_BINARY_OP && (node->op == '+' || node->op == '-')) {
        ExtractSumTerms(node->left, sign, terms, term_count);
        double right_sign = (node->op == '-') ? -sign : sign;
        ExtractSumTerms(node->right, right_sign, terms, term_count);
        return;
    }

    SumTerm term;
    AnalyzeTerm(node, &term);

    if (term.is_simple) {
        term.coefficient *= sign;
    } else {
        term.complex_node = node;
    }

    if (!term.is_simple && sign < 0.0) {
        SumTerm negated_term;
        negated_term.is_simple = false;
        negated_term.coefficient = sign;
        negated_term.variable[0] = '\0';
        negated_term.complex_node = node;
        terms[*term_count] = negated_term;
    } else {
        terms[*term_count] = term;
    }
    ++(*term_count);
}

static ExprNode *BuildAbsTermNode(double abs_coeff, const char *var_name) {
    if (var_name[0] == '\0') {
        return CreateNumberNode(abs_coeff);
    }
    if (IsNearlyEqual(abs_coeff, 1.0)) {
        return CreateVariableNode(var_name);
    }
    ExprNode *coeff_node = CreateNumberNode(abs_coeff);
    ExprNode *var_node = CreateVariableNode(var_name);
    return CreateBinaryOpNode('*', coeff_node, var_node);
}

static ExprNode *AddTermToSum(ExprNode *current_sum, ExprNode *term_node, bool is_negative) {
    if (current_sum == NULL) {
        if (is_negative) {
            return CreateUnaryMinusNode(term_node);
        }
        return term_node;
    }
    if (is_negative) {
        return CreateBinaryOpNode('-', current_sum, term_node);
    }
    return CreateBinaryOpNode('+', current_sum, term_node);
}

static ExprNode *BuildSumFromTerms(SumTerm *terms, int term_count) {
    char unique_vars[MAX_TERMS_COUNT][MAX_VAR_LEN];
    int unique_var_count = 0;
    bool has_complex = false;

    for (int i = 0; i < term_count; ++i) {
        if (!terms[i].is_simple) {
            has_complex = true;
            continue;
        }
        bool found = false;
        for (int j = 0; j < unique_var_count; ++j) {
            if (strcmp(unique_vars[j], terms[i].variable) == 0) {
                found = true;
                break;
            }
        }
        if (!found && unique_var_count < MAX_TERMS_COUNT) {
            strncpy(unique_vars[unique_var_count], terms[i].variable, MAX_VAR_LEN - 1);
            unique_vars[unique_var_count][MAX_VAR_LEN - 1] = '\0';
            ++unique_var_count;
        }
    }

    ExprNode *result = NULL;

    for (int v = 0; v < unique_var_count; ++v) {
        if (unique_vars[v][0] == '\0') {
            continue;
        }
        double combined = 0.0;
        for (int i = 0; i < term_count; ++i) {
            if (terms[i].is_simple && strcmp(terms[i].variable, unique_vars[v]) == 0) {
                combined += terms[i].coefficient;
            }
        }
        if (IsNearlyZero(combined)) {
            continue;
        }
        double abs_coeff = fabs(combined);
        bool is_negative = combined < 0.0;
        ExprNode *term_node = BuildAbsTermNode(abs_coeff, unique_vars[v]);
        result = AddTermToSum(result, term_node, is_negative);
    }

    double constant_sum = 0.0;
    bool has_constant = false;
    for (int i = 0; i < term_count; ++i) {
        if (terms[i].is_simple && terms[i].variable[0] == '\0') {
            constant_sum += terms[i].coefficient;
            has_constant = true;
        }
    }
    if (has_constant && !IsNearlyZero(constant_sum)) {
        double abs_val = fabs(constant_sum);
        bool is_negative = constant_sum < 0.0;
        ExprNode *const_node = CreateNumberNode(abs_val);
        result = AddTermToSum(result, const_node, is_negative);
    }

    if (has_complex) {
        for (int i = 0; i < term_count; ++i) {
            if (terms[i].is_simple) {
                continue;
            }
            ExprNode *copy = CopyTree(terms[i].complex_node);
            bool is_negative = terms[i].coefficient < 0.0;
            if (is_negative && copy->type != NODE_UNARY_MINUS) {
                copy = CreateUnaryMinusNode(copy);
                is_negative = false;
            }
            result = AddTermToSum(result, copy, is_negative);
        }
    }

    if (result == NULL) {
        result = CreateNumberNode(0.0);
    }

    return result;
}

static ExprNode *SimplifyNode(ExprNode *node) {
    if (node == NULL) {
        return NULL;
    }

    if (node->type == NODE_NUMBER || node->type == NODE_VARIABLE) {
        return node;
    }

    if (node->type == NODE_UNARY_MINUS) {
        node->left = SimplifyNode(node->left);
        if (node->left != NULL && IsConstantTree(node->left)) {
            double val = -EvaluateConstant(node->left);
            FreeTree(node->left);
            node->left = NULL;
            node->type = NODE_NUMBER;
            node->number = val;
            node->op = '\0';
            return node;
        }
        return node;
    }

    node->left = SimplifyNode(node->left);
    node->right = SimplifyNode(node->right);

    if (IsConstantTree(node)) {
        double val = EvaluateConstant(node);
        FreeTree(node->left);
        FreeTree(node->right);
        node->left = NULL;
        node->right = NULL;
        node->type = NODE_NUMBER;
        node->number = val;
        node->op = '\0';
        return node;
    }

    if (node->op == '+' || node->op == '-') {
        SumTerm terms[MAX_TERMS_COUNT];
        int term_count = 0;
        ExtractSumTerms(node, 1.0, terms, &term_count);
        ExprNode *simplified = BuildSumFromTerms(terms, term_count);
        FreeTree(node);
        return simplified;
    }

    return node;
}


static int GetOpPrecedence(char op) {
    if (op == '+' || op == '-') {
        return PRIORITY_ADDITIVE;
    }
    if (op == '*' || op == '/') {
        return PRIORITY_MULTIPLICATIVE;
    }
    if (op == '^') {
        return PRIORITY_POWER;
    }
    return 0;
}

static void PrintExpressionNode(const ExprNode *node, int parent_prec, bool is_right_child) {
    if (node == NULL) {
        return;
    }

    if (node->type == NODE_NUMBER) {
        bool needs_parens = node->number < 0.0 && parent_prec > 0;
        if (needs_parens) {
            printf("(");
        }
        PrintDouble(node->number);
        if (needs_parens) {
            printf(")");
        }
        return;
    }

    if (node->type == NODE_VARIABLE) {
        printf("%s", node->variable);
        return;
    }

    if (node->type == NODE_UNARY_MINUS) {
        printf("-(");
        PrintExpressionNode(node->left, 0, false);
        printf(")");
        return;
    }

    int cur_prec = GetOpPrecedence(node->op);
    bool needs_parens = cur_prec < parent_prec
                        || (cur_prec == parent_prec && is_right_child && (node->op == '-' || node->op == '/'));
    if (needs_parens) {
        printf("(");
    }
    PrintExpressionNode(node->left, cur_prec, false);
    printf(" %c ", node->op);
    PrintExpressionNode(node->right, cur_prec, true);
    if (needs_parens) {
        printf(")");
    }
}

static void PrintExpression(const ExprNode *node) {
    PrintExpressionNode(node, 0, false);
    printf("\n");
}

static void PrintTreeVisual(const ExprNode *node, int depth) {
    if (node == NULL) {
        return;
    }

    PrintTreeVisual(node->right, depth + 1);

    for (int i = 0; i < depth; ++i) {
        printf("|");
    }

    if (node->type == NODE_NUMBER) {
        PrintDouble(node->number);
        printf("\n");
    } else if (node->type == NODE_VARIABLE) {
        printf("%s\n", node->variable);
    } else if (node->type == NODE_UNARY_MINUS) {
        printf("(-)\n");
    } else {
        printf("%c\n", node->op);
    }

    PrintTreeVisual(node->left, depth + 1);
}

static ExprNode *ParseExpression(const char *input) {
    Token tokens[MAX_TOKENS_COUNT];
    int token_count = Tokenize(input, tokens, MAX_TOKENS_COUNT);
    if (token_count < 0) {
        return NULL;
    }
    if (token_count == 0) {
        printf("Ошибка. Пустое выражение.\n");
        return NULL;
    }

    Token postfix[MAX_TOKENS_COUNT];
    int postfix_count = ShuntingYard(tokens, token_count, postfix, MAX_TOKENS_COUNT);
    if (postfix_count < 0) {
        return NULL;
    }

    PrintPostfix(postfix, postfix_count);

    ExprNode *tree = BuildTreeFromPostfix(postfix, postfix_count);
    return tree;
}

int main() {

    char input[MAX_INPUT_LEN];
    printf("Введите выражение:\n");
    if (fgets(input, MAX_INPUT_LEN, stdin) == NULL) {
        printf("Ошибка. Не удалось прочитать ввод.\n");
        return 1;
    }
    int input_len = (int)strlen(input);
    if (input_len > 0 && input[input_len - 1] == '\n') {
        input[input_len - 1] = '\0';
    }

    ExprNode *tree = ParseExpression(input);
    if (tree == NULL) {
        return 1;
    }

    printf("Исходное выражение: ");
    PrintExpression(tree);
    printf("Дерево исходного выражения:\n");
    PrintTreeVisual(tree, 0);

    ExprNode *simplified = SimplifyNode(tree);

    printf("Упрощённое выражение: ");
    PrintExpression(simplified);
    printf("Дерево упрощённого выражения:\n");
    PrintTreeVisual(simplified, 0);

    FreeTree(simplified);
    return 0;
}