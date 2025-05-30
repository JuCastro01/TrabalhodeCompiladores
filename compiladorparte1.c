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
    OR, AND, COMENTARIO, EOS
} TAtomo;

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

// estrutura p/ tabela de símbolos
typedef struct _TNo {
    char ID[16];
    int endereco;
    struct _TNo *prox;
} TNo;

// Tabela de palavras reservadas
TPalavraReservada palavras[] = {
    {"char", CHAR}, {"else", ELSE}, {"if", IF}, {"int", INT},
    {"main", MAIN}, {"readint", READINT}, {"void", VOID},
    {"while", WHILE}, {"writeint", WRITEINT}
};

// Variáveis globais
char *entrada = NULL;       
char *cursor = NULL;        // Posição atual no código
int contaLinha = 1;         // Contador de linhas
TInfoAtomo lookahead;       // Token atual
TInfoAtomo infoAtual;       // Token anterior
TNo *tabela = NULL;         // Tabela de símbolos
int prox_endereco = 0;      // Próximo endereço de memória
int rotulo = 0;             // Contador de rotulos

// Nomes dos tokens para printar
const char *strAtomo[] = {
    "erro", "id", "intconst", "charconst",
    "char", "else", "if", "int", "main", "readint", "void", "while", "writeint",
    "abre_par", "fecha_par", "abre_chaves", "fecha_chaves", "igual", "ponto_virgula",
    "virgula", "mais", "menos", "mult", "div",
    "rel_menor", "rel_menorig", "rel_maior", "rel_maiorig", "rel_igual", "rel_dif",
    "or", "and", "comentario", "eos"
};

// Insere identificador na tabela de símbolos
void insere_tabela_simbolos(const char *id) {
    // Verifica se já existe ou n
    TNo *n = tabela;
    while (n) {
        if (strcmp(n->ID, id) == 0) {
            printf("Erro semântico na linha %d: identificador [%s] já declarado\n", contaLinha, id);
            exit(1);
        }
        n = n->prox;
    }
    TNo *novo = malloc(sizeof(TNo));
    strcpy(novo->ID, id);
    novo->endereco = prox_endereco++;
    novo->prox = tabela;
    tabela = novo;
}

// Busca identificador na tabela de símbolos
int busca_tabela_simbolos(const char *id) {
    TNo *n = tabela;
    while (n) {
        if (strcmp(n->ID, id) == 0) return n->endereco;
        n = n->prox;
    }
    printf("Erro semântico na linha %d: identificador [%s] não declarado\n", contaLinha, id);
    exit(1);
}

// Gera prox rotulo
int proximo_rotulo() {
    return rotulo++;
}

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

    // Fim do codigo
    if (*cursor == '\0') {
        info.atomo = EOS;
        return info;
    }

    // Comentarios
    if (*cursor == '/' && *(cursor+1) == '/') {
        reconhece_comentario();
        return obter_atomo();
    }

    if (*cursor == '/' && *(cursor+1) == '*') {
        reconhece_comentario();
        return obter_atomo();
    }

    // Identificadores e palavras reservadas
    if (isalpha(*cursor) || *cursor == '_') return reconhece_id_ou_palavra();
    // Constantes hexadecimais
    if (*cursor == '0' && *(cursor+1) == 'x') return reconhece_intconst();
    // Constantes de letras
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

    return info;
}

// Reconhece identificadores ou palavras reservadas
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
    return info;
}

// Reconhece constantes
TInfoAtomo reconhece_intconst() {
    TInfoAtomo info = {INTCONST, contaLinha, "", 0, '\0'};
    cursor += 2; // Pula "0x"
    int val = 0;
    
    // Converte os hexadecimais
    while (isxdigit(*cursor)) {
        val = val * 16 + (isdigit(*cursor) ? *cursor - '0' : toupper(*cursor) - 'A' + 10);
        cursor++;
    }
    info.atributo_decimal = val;
    return info;
}

// Reconhece constantes de palavras
TInfoAtomo reconhece_charconst() {
    TInfoAtomo info = {CHARCONST, contaLinha, "", 0, '\0'};
    cursor++; 
    
    // Verifica formato válido 'c'
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
    if (*cursor == '/' && *(cursor+1) == '/') {
        // Comentário de linha
        cursor += 2;
        while (*cursor != '\n' && *cursor != '\0') cursor++;
    } else if (*cursor == '/' && *(cursor+1) == '*') {
        // Comentário de bloco
        cursor += 2;
        while (*cursor && !(*cursor == '*' && *(cursor+1) == '/')) {
            if (*cursor == '\n') contaLinha++;
            cursor++;
        }
        if (*cursor == '*') cursor += 2;
    }
}

// Função para consumir token esperado
void consome(TAtomo t) {
    if (lookahead.atomo == t) {
        infoAtual = lookahead;
        lookahead = obter_atomo();
    } else {
        printf("Erro sintático na linha %d: esperado [%s], encontrado [%s]\n",
               lookahead.linha, strAtomo[t], strAtomo[lookahead.atomo]);
        exit(1);
    }
}

//funções
void program(), compound_stmt(), var_decl(), var_decl_list(), stmt();
void expr(), conjunction(), comparison(), sum(), term(), factor();

//função inicial: void main(void) { ... }
void program() {
    consome(VOID); consome(MAIN); consome(ABRE_PAR); consome(VOID); consome(FECHA_PAR);
    compound_stmt();
}


void compound_stmt() {
    consome(ABRE_CHAVES);
    var_decl();         // Declarações de variáveis
    while (lookahead.atomo != FECHA_CHAVES)
        stmt();         // Comandos
    consome(FECHA_CHAVES);
}

// Declaração 
void var_decl() {
    while (lookahead.atomo == INT || lookahead.atomo == CHAR) {
        consome(lookahead.atomo);
        var_decl_list();
        consome(PONTO_VIRGULA);
    }
}

void var_decl_list() {
    insere_tabela_simbolos(lookahead.lexema);
    printf("\tAMEM \n");      // Aloca memória
    consome(IDENTIFICADOR);
    while (lookahead.atomo == VIRGULA) {
        consome(VIRGULA);
        insere_tabela_simbolos(lookahead.lexema);
        printf("\tAMEM\n");
        consome(IDENTIFICADOR);
    }
}

// Comandos da linguagem
void stmt() {
    if (lookahead.atomo == ABRE_CHAVES) {
        compound_stmt();
    } else if (lookahead.atomo == IDENTIFICADOR) {
        // id
        int endereco = busca_tabela_simbolos(lookahead.lexema);
        consome(IDENTIFICADOR); consome(IGUAL); expr();
        printf("\tARMZ %d\n", endereco);  // Armazena valor
        consome(PONTO_VIRGULA);
    } else if (lookahead.atomo == IF) {
        // if
        int L1 = proximo_rotulo();
        int L2 = proximo_rotulo();
        consome(IF); consome(ABRE_PAR); expr(); consome(FECHA_PAR);
        printf("\tDSVF L%d\n", L1);      // Desvia se falso
        stmt();
        printf("\tDSVS L%d\n", L2);      // Desvia sempre
        printf("L%d:\tNADA\n", L1);
        //else
        if (lookahead.atomo == ELSE) {
            consome(ELSE); stmt();
        }
        printf("L%d:\tNADA\n", L2);
    } else if (lookahead.atomo == WHILE) {
        //while
        int L1 = proximo_rotulo();
        int L2 = proximo_rotulo();
        printf("L%d:\tNADA\n", L1);      // Início do loop
        consome(WHILE); consome(ABRE_PAR); expr(); consome(FECHA_PAR);
        printf("\tDSVF L%d\n", L2);      // Sai do loop se falso
        stmt();
        printf("\tDSVS L%d\n", L1);      // Volta ao início
        printf("L%d:\tNADA\n", L2);      // Fim do loop
    } else if (lookahead.atomo == READINT) {
        //read (recebe entrada)
        consome(READINT); consome(ABRE_PAR);
        int endereco = busca_tabela_simbolos(lookahead.lexema);
        consome(IDENTIFICADOR); consome(FECHA_PAR); consome(PONTO_VIRGULA);
        printf("\tLEIT\n\tARMZ %d\n", endereco);
    } else if (lookahead.atomo == WRITEINT) {
        // Escrita: writeint(expr);
        consome(WRITEINT); consome(ABRE_PAR); expr(); consome(FECHA_PAR); consome(PONTO_VIRGULA);
        printf("\tIMPR\n");
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
        printf("\tDISJ\n");
    }
}

//AND
void conjunction() {
    comparison();
    while (lookahead.atomo == AND) {
        consome(AND);
        comparison();
        printf("\tCONJ\n");
    }
}

// maior ou menor
void comparison() {
    sum();
    if (lookahead.atomo >= REL_MENOR && lookahead.atomo <= REL_DIF) {
        TAtomo op = lookahead.atomo;
        consome(op);
        sum();
        switch (op) {
            case REL_MENOR: printf("\tCMME\n"); break;     // Menor
            case REL_MENORIG: printf("\tCMEG\n"); break;   // Menor ou igual
            case REL_MAIOR: printf("\tCMMA\n"); break;     // Maior
            case REL_MAIORIG: printf("\tCMAG\n"); break;   // Maior ou igual
            case REL_IGUAL: printf("\tCMIG\n"); break;     // Igual
            case REL_DIF: printf("\tCMDG\n"); break;       // Diferente
            default: break;
        }
    }
}

// Soma e subtração
void sum() {
    term();
    while (lookahead.atomo == MAIS || lookahead.atomo == MENOS) {
        TAtomo op = lookahead.atomo;
        consome(op);
        term();
        if (op == MAIS) printf("\tSOMA\n");
        else printf("\tSUBT\n");
    }
}

// Multiplicação e divisão
void term() {
    factor();
    while (lookahead.atomo == MULT || lookahead.atomo == DIV) {
        TAtomo op = lookahead.atomo;
        consome(op);
        factor();
        if (op == MULT) printf("\tMULT\n");
        else printf("\tDIVI\n");
    }
}


void factor() {
    if (lookahead.atomo == IDENTIFICADOR) {
        int endereco = busca_tabela_simbolos(lookahead.lexema);
        printf("\tCRVL %d\n", endereco);  // Carrega valor
        consome(IDENTIFICADOR);
    } else if (lookahead.atomo == INTCONST) {
        printf("\tCRCT %d\n", lookahead.atributo_decimal);  // Carrega constante
        consome(INTCONST);
    } else if (lookahead.atomo == ABRE_PAR) {
        consome(ABRE_PAR);
        expr();
        consome(FECHA_PAR);
    } else {
        printf("Erro sintático na linha %d: fator inválido\n", lookahead.linha);
        exit(1);
    }
}

// Imprime tabela de símbolos
void imprime_tabela_simbolos() {
    printf("\nTabela de Símbolos:\n");
    TNo *n = tabela;
    while (n) {
        printf("ID: %-16s | Endereço: %d\n", n->ID, n->endereco);
        n = n->prox;
    }
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Uso: %s <arquivo_tinyc>\n", argv[0]);
        return 1;
    }

    // Le arquivo de entrada
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror("Erro ao abrir arquivo");
        return 1;
    }

    //guardar o arquivo (correção de warning)
    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    rewind(f);
    entrada = malloc(tam + 1);
    fread(entrada, 1, tam, f);
    entrada[tam] = '\0';
    fclose(f);

    cursor = entrada;
    lookahead = obter_atomo();
    program();
    
    if (lookahead.atomo != EOS) {
        printf("Erro: código após fim do programa.\n");
        return 1;
    }
    
    imprime_tabela_simbolos();
    free(entrada);
    return 0;
}
