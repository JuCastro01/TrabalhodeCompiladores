#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// Tokens
typedef enum {
    ERRO, IDENTIFICADOR, INTCONST, CHARCONST,
    CHAR, ELSE, IF, INT, MAIN, READINT, VOID, WHILE, WRITEINT,
    ABRE_PAR, FECHA_PAR, ABRE_CHAVES, FECHA_CHAVES, IGUAL, PONTO_VIRGULA,
    VIRGULA, MAIS, MENOS, MULT, DIV,
    REL_MENOR, REL_MENORIG, REL_MAIOR, REL_MAIORIG, REL_IGUAL, REL_DIF,
    OR, AND,
    COMENTARIO,
    EOS
} TAtomo;

// Estrutura para armazenar informações de um token
typedef struct {
    TAtomo atomo;
    int linha;
    char lexema[32];
    int atributo_decimal;
    char atributo_caractere;
} TInfoAtomo;

// Palavras reservadas
typedef struct {
    const char *palavra;
    TAtomo atomo;
} TPalavraReservada;

// Tabela de palavras reservadas
TPalavraReservada palavras[] = {
    {"char", CHAR}, {"else", ELSE}, {"if", IF}, {"int", INT},
    {"main", MAIN}, {"readint", READINT}, {"void", VOID},
    {"while", WHILE}, {"writeint", WRITEINT}
};

// Variáveis globais 
char *entrada = NULL;
char *cursor = NULL;
int contaLinha = 1;

// Nomes dos tokens para printar
const char *strAtomo[] = {
    "erro", "id", "intconst", "charconst",
    "char", "else", "if", "int", "main", "readint", "void", "while", "writeint",
    "abre_par", "fecha_par", "abre_chaves", "fecha_chaves", "igual", "ponto_virgula",
    "virgula", "mais", "menos", "mult", "div",
    "rel_menor", "rel_menorig", "rel_maior", "rel_maiorig", "rel_igual", "rel_dif",
    "or", "and",
    "comentario",
    "eos"
};

// Lexico
TInfoAtomo obter_atomo();
TInfoAtomo reconhece_id_ou_palavra();
TInfoAtomo reconhece_intconst();
TInfoAtomo reconhece_charconst();
void reconhece_comentario();

TInfoAtomo obter_atomo() {
    TInfoAtomo info = {ERRO, contaLinha, "", 0, '\0'};
    
    // Pula espaços em branco
    while (*cursor == ' ' || *cursor == '\t' || *cursor == '\n' || *cursor == '\r') {
        if (*cursor == '\n') contaLinha++;
        cursor++;
    }

    info.linha = contaLinha;

    // Fim do arquivo
    if (*cursor == '\0') {
        info.atomo = EOS;
        return info;
    }

    // Comentário 
    if (*cursor == '/' && *(cursor+1) == '/') {
        reconhece_comentario();
        printf("# %d:comentario\n", info.linha);
        return obter_atomo(); // ignora comentário
    }
    if (*cursor == '/' && *(cursor+1) == '*') {
        reconhece_comentario();
        printf("# %d:comentario\n", info.linha);
        return obter_atomo();
    }

    // Identificadores e palavras reservadas
    if (isalpha(*cursor) || *cursor == '_') return reconhece_id_ou_palavra();
    // Constantes hexadecimais
    if (*cursor == '0' && *(cursor+1) == 'x') return reconhece_intconst();
    // Constantes de caractere
    if (*cursor == '\'') return reconhece_charconst();

    // Operadores, parenteses, chaves
    switch (*cursor) {
        case '(': cursor++; info.atomo = ABRE_PAR; break;
        case ')': cursor++; info.atomo = FECHA_PAR; break;
        case '{': cursor++; info.atomo = ABRE_CHAVES; break;
        case '}': cursor++; info.atomo = FECHA_CHAVES; break;
        case '=':
            if (*(cursor+1) == '=') { cursor += 2; info.atomo = REL_IGUAL; }
            else { cursor++; info.atomo = IGUAL; }
            break;
        case ';': cursor++; info.atomo = PONTO_VIRGULA; break;
        case ',': cursor++; info.atomo = VIRGULA; break;
        case '+': cursor++; info.atomo = MAIS; break;
        case '-': cursor++; info.atomo = MENOS; break;
        case '*': cursor++; info.atomo = MULT; break;
        case '/': cursor++; info.atomo = DIV; break;
        case '<':
            if (*(cursor+1) == '=') { cursor += 2; info.atomo = REL_MENORIG; }
            else { cursor++; info.atomo = REL_MENOR; }
            break;
        case '>':
            if (*(cursor+1) == '=') { cursor += 2; info.atomo = REL_MAIORIG; }
            else { cursor++; info.atomo = REL_MAIOR; }
            break;
        case '!':
            if (*(cursor+1) == '=') { cursor += 2; info.atomo = REL_DIF; }
            else { cursor++; info.atomo = ERRO; }
            break;
        case '&':
            if (*(cursor+1) == '&') { cursor += 2; info.atomo = AND; }
            else { cursor++; info.atomo = ERRO; }
            break;
        case '|':
            if (*(cursor+1) == '|') { cursor += 2; info.atomo = OR; }
            else { cursor++; info.atomo = ERRO; }
            break;
        default: cursor++; info.atomo = ERRO; break;
    }

    // Imprime o token reconhecido
    printf("# %d:%s", info.linha, strAtomo[info.atomo]);
    if (info.atomo == IDENTIFICADOR)
        printf(" | %s", info.lexema);
    else if (info.atomo == INTCONST)
        printf(" | %d", info.atributo_decimal);
    else if (info.atomo == CHARCONST)
        printf(" | %c", info.atributo_caractere);
    printf("\n");

    return info;
}

// Reconhece identificadores e palavras reservadas
TInfoAtomo reconhece_id_ou_palavra() {
    TInfoAtomo info = {IDENTIFICADOR, contaLinha, "", 0, '\0'};
    int len = 0;
    
    while (isalnum(*cursor) || *cursor == '_') {
        if (len < 31) info.lexema[len++] = *cursor;
        cursor++;
    }
    info.lexema[len] = '\0';
    
    // Verifica tamanho máximo do identificador (para arrumar o warning)
    if (len > 15) { info.atomo = ERRO; return info; }

    // Verifica se é palavra reservada
    for (int i = 0; i < (int)(sizeof(palavras)/sizeof(palavras[0])); i++) {
        if (strcmp(info.lexema, palavras[i].palavra) == 0) {
            info.atomo = palavras[i].atomo;
            break;
        }
    }
    
    printf("# %d:%s", info.linha, strAtomo[info.atomo]);
    if (info.atomo == IDENTIFICADOR)
        printf(" | %s", info.lexema);
    printf("\n");
    return info;
}

// Reconhece constantes hexadecimais
TInfoAtomo reconhece_intconst() {
    TInfoAtomo info = {INTCONST, contaLinha, "", 0, '\0'};
    cursor += 2; // pula "0x"
    int val = 0;
    
    // Converte dígitos hexadecimais
    while (isxdigit(*cursor)) {
        val = val * 16 + (isdigit(*cursor) ? *cursor - '0' : toupper(*cursor) - 'A' + 10);
        cursor++;
    }
    info.atributo_decimal = val;
    return info;
}

// Reconhece constantes de caractere
TInfoAtomo reconhece_charconst() {
    TInfoAtomo info = {CHARCONST, contaLinha, "", 0, '\0'};
    cursor++; // pula primeira '
    
    // Verifica formato 'c'
    if (*cursor != '\0' && *(cursor+1) == '\'') {
        info.atributo_caractere = *cursor;
        cursor += 2;
    } else {
        info.atomo = ERRO;
    }
    return info;
}

// Reconhece comentários
void reconhece_comentario() {
    // Comentário de linha
    if (*cursor == '/' && *(cursor+1) == '/') {
        cursor += 2;
        while (*cursor != '\n' && *cursor != '\0') cursor++;
    } 
    // Comentário de bloco
    else if (*cursor == '/' && *(cursor+1) == '*') {
        cursor += 2;
        while (*cursor && !(*cursor == '*' && *(cursor+1) == '/')) {
            if (*cursor == '\n') contaLinha++;
            cursor++;
        }
        if (*cursor == '*') cursor += 2;
    }
}

// Sintatico

TInfoAtomo lookahead; // Token atual

// funções
void consome(TAtomo t);
void program(), compound_stmt(), var_decl(), var_decl_list(), stmt();
void expr(), conjunction(), comparison(), sum(), term(), factor();

// Consome um token esperado
void consome(TAtomo t) {
    if (lookahead.atomo == t)
        lookahead = obter_atomo();
    else {
        printf("Erro sintático na linha %d: esperado [%s], encontrado [%s]\n",
               lookahead.linha, strAtomo[t], strAtomo[lookahead.atomo]);
        exit(1);
    }
}


int main(int argc, char *argv[]) {
    // Verifica argumentos
    if (argc < 2) {
        printf("Uso: %s <arquivo_tinyc>\n", argv[0]);
        return 1;
    }

    // Abre arquivo
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    // Lê arquivo completo
    fseek(f, 0, SEEK_END);
    long tamanho = ftell(f);
    rewind(f);
    entrada = (char *) malloc(tamanho + 1);
    if (!entrada) {
        fprintf(stderr, "Erro de alocação\n");
        fclose(f);
        return 1;
    }

    fread(entrada, 1, tamanho, f);
    entrada[tamanho] = '\0';
    fclose(f);

    // Inicia análise
    cursor = entrada;
    lookahead = obter_atomo();
    program();
    
    // Verifica fim do programa
    if (lookahead.atomo != EOS) {
        printf("Erro: código após fim do programa.\n");
        return 1;
    }

    printf("%d linhas analisadas, programa sintaticamente correto\n", contaLinha);
    free(entrada);
    return 0;
}

void program() {
    consome(VOID); consome(MAIN); consome(ABRE_PAR); consome(VOID); consome(FECHA_PAR);
    compound_stmt();
}


void compound_stmt() {
    consome(ABRE_CHAVES);
    var_decl();
    while (lookahead.atomo != FECHA_CHAVES)
        stmt();
    consome(FECHA_CHAVES);
}

void var_decl() {
    while (lookahead.atomo == INT || lookahead.atomo == CHAR) {
        consome(lookahead.atomo);
        var_decl_list();
        consome(PONTO_VIRGULA);
    }
}

void var_decl_list() {
    consome(IDENTIFICADOR);
    if (lookahead.atomo == IGUAL) {
        consome(IGUAL);
        expr();
    }
    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        consome(IDENTIFICADOR);
        if (lookahead.atomo == IGUAL) {
            consome(IGUAL);
            expr();
        }
    }
}

void stmt() {
    if (lookahead.atomo == ABRE_CHAVES) compound_stmt();
    else if (lookahead.atomo == IDENTIFICADOR) {
        // Atribuição: id = expr;
        consome(IDENTIFICADOR); consome(IGUAL); expr(); consome(PONTO_VIRGULA);
    } else if (lookahead.atomo == IF) {
        // if (expr) stmt else stmt
        consome(IF); consome(ABRE_PAR); expr(); consome(FECHA_PAR); stmt();
        if (lookahead.atomo == ELSE) { consome(ELSE); stmt(); }
    } else if (lookahead.atomo == WHILE) {
        // while (expr) stmt
        consome(WHILE); consome(ABRE_PAR); expr(); consome(FECHA_PAR); stmt();
    } else if (lookahead.atomo == READINT) {
        // readint(id);
        consome(READINT); consome(ABRE_PAR); consome(IDENTIFICADOR); consome(FECHA_PAR); consome(PONTO_VIRGULA);
    } else if (lookahead.atomo == WRITEINT) {
        // writeint(expr);
        consome(WRITEINT); consome(ABRE_PAR); expr(); consome(FECHA_PAR); consome(PONTO_VIRGULA);
    } else {
        printf("Erro sintático na linha %d: comando inválido\n", lookahead.linha);
        exit(1);
    }
}

//OR
void expr() {
    conjunction();
    while (lookahead.atomo == OR) {
        consome(OR);
        conjunction();
    }
}

//AND
void conjunction() {
    comparison();
    while (lookahead.atomo == AND) {
        consome(AND);
        comparison();
    }
}

// comparar
void comparison() {
    sum();
    if (lookahead.atomo >= REL_MENOR && lookahead.atomo <= REL_DIF) {
        consome(lookahead.atomo);
        sum();
    }
}

// Soma e subtração
void sum() {
    term();
    while (lookahead.atomo == MAIS || lookahead.atomo == MENOS) {
        consome(lookahead.atomo);
        term();
    }
}
void term() {
// Multiplicação e divisão
    factor();
    while (lookahead.atomo == MULT || lookahead.atomo == DIV) {
        consome(lookahead.atomo);
        factor();
    }
}

void factor() {
    if (lookahead.atomo == INTCONST || lookahead.atomo == CHARCONST || lookahead.atomo == IDENTIFICADOR)
        consome(lookahead.atomo);
    else if (lookahead.atomo == ABRE_PAR) {
        consome(ABRE_PAR);
        expr();
        consome(FECHA_PAR);
    } else {
        printf("Erro sintático na linha %d\n", lookahead.linha);
        exit(1);
    }
}
