#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define MAX_ITER 1000
#define EPSILON 0.0001
#define TOLER 0.001
#define X0 0.05
// leitura do arquivo Meta
typedef struct {
    // Capitalização
    int idadeFormatura;
    int idadeFimCapitalizacao;
    double salarioMedioLiquido;
    double percentualInvestimento;
    double patrimonioAcumulado;

    // Descapitalização
    int idadeFimRetirada;
} Meta;

// Leitura dos arquivos de aplicações financeiras
typedef struct {
    char nomeAtivo[100];
    char categoria[100];
    float taxaRetorno;
    int risco;
} Aplicacoes;

// lista encadeada
typedef struct NodeAplicacoes {
    Aplicacoes inv;
    struct NodeAplicacoes *next;
} NodeAplicacoes;


//melhor opcao investimento -> capitalizacao
typedef struct {
    int periodo; //em meses
    double pagamento; //valor investido -> mensalmente
    double capitalizado; // total
    double juros; // taxa de juros
    char nomeAtivo[100];
} CapSelecionado;

typedef struct {
    char nomeAtivo[100];
    double taxa;
    double retirada; //PMTret
} OpcoesDescap;

typedef struct NodeOpcoesDescap {
    OpcoesDescap opcoesDescap;
    struct NodeOpcoesDescap *next;
} NodeOpcoesDescap;

//Leitura de arquivos
void ler_arquivo_meta(const char *arquivo_meta, Meta *meta);
NodeAplicacoes* ler_arquivoCap(const char *arquivoCap);
NodeAplicacoes* ler_arquivoDescap(const char *arquivoDescap);

//Funcoes de capitalizacao
double derivadaCapit(double i, double PMT, int n, double FV);
double fCapitalizacao(double i, double PMT, int n, double FV);
double newtonCapitalizao(double i, double PMT, int n, double FV);

//Funcoes de descapitalizacao
double fDescapitalizacao(double pmt_ret,int n, double pv, double taxa);
double derivadaDesc(double pmt_ret, int n, double pv, double taxa);
double newtonDescapitalizacao(double pmt_ret, int n, double pv, double taxa);
CapSelecionado melhorInvestimento(double taxaEncontrada, NodeAplicacoes *listaCap,Meta meta);

//Funcoes de relatorio de saida
//Retorna 1 se deve continuar para a descapitalizacao ou 0 se o relatorio
//ja encerrou na etapa de capitalizacao.
int escrever_relatorio_capitalizacao(FILE *fp,Meta meta,double taxa_calculada,CapSelecionado capSel);
void escrever_relatorio_descapitalizacao(FILE *fp,Meta meta,CapSelecionado capSel,NodeOpcoesDescap *listaDescap);


void bubbleSort(NodeAplicacoes **inicioLista);

int main(int argc, char **argv) {

    // arquivo-meta arquivo-capitalizacao arquivo-descapitalizacao saida-relatorio
    char arqMeta[128], arqCapitalizao[128], arqDescapitalizao[128], arqRelatorio[128];

    printf("<arquivo-meta> <arquivo-capitalizacao> <arquivo-descapitalizacao> <saida-relatorio>\n");

    if (scanf("%127s %127s %127s %127s", arqMeta, arqCapitalizao, arqDescapitalizao, arqRelatorio) != 4) {
        printf("\nErro de Sintaxe\n");
        exit(1);
    }

    if (strlen(arqMeta) == 0 || strlen(arqCapitalizao) == 0 || strlen(arqDescapitalizao) == 0 || strlen(arqRelatorio) == 0) {
        printf("\nErro: entre com o nome de todos os arquivos (meta - capitalizacao - descanpitalizacao - relatorio).\n");
        exit(1);
    }


    //  LEITURA DOS ARQUIVOS

    Meta meta = {0};
    ler_arquivo_meta(arqMeta, &meta);

    // Teste: imprimir o conteúdo lido do arquivo-meta
    printf("\n=== Dados lidos do arquivo-meta ===\n");
    printf("Idade Formatura: %d\n", meta.idadeFormatura);
    printf("Idade Fim Capitalização: %d\n", meta.idadeFimCapitalizacao);
    printf("Salário Médio Líquido: %.2f\n", meta.salarioMedioLiquido);
    printf("Percentual de Investimento: %.2f\n", meta.percentualInvestimento);
    printf("Patrimônio Acumulado: %.2f\n", meta.patrimonioAcumulado);
    printf("Idade Fim Retirada: %d\n", meta.idadeFimRetirada);

    NodeAplicacoes *listaCap = ler_arquivoCap(arqCapitalizao);
    NodeAplicacoes *listaDescap = ler_arquivoDescap(arqDescapitalizao);

    // Teste: imprimir o conteúdo lido dos arquivos
    printf("\n=== Investimentos para Capitalização ===\n");
    NodeAplicacoes *p1 = listaCap;
    while (p1 != NULL) {
        printf("Ativo: %s | Taxa: %.4f | Risco: %d\n", p1->inv.nomeAtivo, p1->inv.taxaRetorno, p1->inv.risco);
        p1 = p1->next;
    }

    printf("\n=== Investimentos para Descapitalização ===\n");
    NodeAplicacoes *p2 = listaDescap;
    while (p2 != NULL) {
        printf("Ativo: %s | Taxa: %.4f | Risco: %d\n", p2->inv.nomeAtivo, p2->inv.taxaRetorno, p2->inv.risco);
        p2 = p2->next;
    }

    //Capitalizacao
    CapSelecionado capSelecionado = {0};

    double taxaEncontrada = newtonCapitalizao(X0,meta.percentualInvestimento*meta.salarioMedioLiquido,(meta.idadeFimCapitalizacao-meta.idadeFormatura)*12,meta.patrimonioAcumulado );


    if (taxaEncontrada == 0) {
        printf("\nSaida de relatorio: Nao existe aplicacao...\n");
    }
    else {
        //ordenar listaCap
        bubbleSort(&listaCap);
        capSelecionado = melhorInvestimento(taxaEncontrada, listaCap, meta);

        if (capSelecionado.capitalizado == 0 ) {
            printf("\nSaida de relatorio: Nao existe aplicacao...\n");
        }
    }

    //Descapitalizacao
    p2 = listaDescap;
    NodeOpcoesDescap *listaDescapRelatorio = NULL;
    NodeOpcoesDescap *ultimoDescapRelatorio = NULL;

    while (p2 != NULL) {
        NodeOpcoesDescap *novo = malloc(sizeof(NodeOpcoesDescap));

        novo->next = NULL;

        novo->opcoesDescap.retirada = newtonDescapitalizacao(meta.salarioMedioLiquido,(meta.idadeFimRetirada - meta.idadeFimCapitalizacao)*12, capSelecionado.capitalizado, p2->inv.taxaRetorno);
        novo->opcoesDescap.taxa = p2->inv.taxaRetorno;
        strncpy(novo->opcoesDescap.nomeAtivo, p2->inv.nomeAtivo, sizeof(novo->opcoesDescap.nomeAtivo) - 1);
        //Garante que a string termina com um caractere nulo
        novo->opcoesDescap.nomeAtivo[sizeof(novo->opcoesDescap.nomeAtivo) - 1] = '\0';


        if (listaDescapRelatorio == NULL) {
            // lista vazia: novo nó é a cabeça
            listaDescapRelatorio = novo;
            ultimoDescapRelatorio = novo;
        } else {
            // lista não vazia: adiciona no final
            ultimoDescapRelatorio->next = novo;
            ultimoDescapRelatorio = novo;

        }

        p2 = p2->next;

    }

    FILE *fp = fopen(arqRelatorio, "w");
    if (!fp)
    {
        perror("Erro ao criar relatório");
        exit(1);
    }

    // 1) secao de capitalizacao
    int viavel = escrever_relatorio_capitalizacao(fp, meta, taxaEncontrada, capSelecionado);

    // 2) somente se for viavel prosseguir para descapitalizacao
    if (viavel) {
        escrever_relatorio_descapitalizacao(fp, meta, capSelecionado, listaDescapRelatorio);
    }

    fclose(fp);

    return 0;
  }

//melhor investimento -> taxa maior ou igual a taxaEncontrada e menor risco
CapSelecionado melhorInvestimento(double taxaEncontrada, NodeAplicacoes *listaCap, Meta meta) {

    CapSelecionado melhorOpcao = {0};

    melhorOpcao.periodo = (meta.idadeFimCapitalizacao - meta.idadeFormatura)*12;
    melhorOpcao.pagamento = meta.percentualInvestimento * meta.salarioMedioLiquido;

    //funcao q ordena
    int menorRisco = 6;
    double melhorTaxa = taxaEncontrada;


    NodeAplicacoes *p = listaCap;
    while (p  != NULL) {
        if (p->inv.taxaRetorno >= melhorTaxa) {
            if (p->inv.risco <= menorRisco) {
                strncpy( melhorOpcao.nomeAtivo, p->inv.nomeAtivo, sizeof(melhorOpcao.nomeAtivo) - 1);
                melhorOpcao.nomeAtivo[sizeof(melhorOpcao.nomeAtivo) - 1] = '\0';
                melhorOpcao.juros = p->inv.taxaRetorno;

                // total capitalizado
                //melhorOpcao.capitalizado
                melhorOpcao.capitalizado = melhorOpcao.pagamento * ( pow((1+melhorOpcao.juros), melhorOpcao.periodo) - 1)/melhorOpcao.juros;

                melhorTaxa = p->inv.taxaRetorno;
                menorRisco = p->inv.risco;
            }
        }
        p = p->next;
    }

    return melhorOpcao;
}

// LEITURA DOS ARQUIVOS
void ler_arquivo_meta(const char *arquivo_meta, Meta *meta) {

    FILE *file = fopen(arquivo_meta, "r");
    if (!file) {
        perror("Erro ao abrir o arquivo-meta");
        exit(1);
    }


    char linha[256];
    while (fgets(linha, sizeof(linha), file) != NULL) {
        // # = comentario
        // \n = linha em branco
        if (linha[0] == '#' || linha[0] == '\n') {
            continue;
        }

        char tipo; // c = capitalização  d = descapitalização
        char nome[64]; // nome da variável
        double valor; // valor da variável


        if (sscanf(linha, "%c %63s %lf", &tipo, nome, &valor) == 3) {

            if (strcmp(nome, "IDADEFORMATURA") == 0)
                meta->idadeFormatura = (int)valor;

            else if (strcmp(nome, "IDADEFIMCAP") == 0)
                meta->idadeFimCapitalizacao = (int)valor;

            else if (strcmp(nome, "SALARIOMEDIOLIQ") == 0)
                meta->salarioMedioLiquido = valor;

            else if (strcmp(nome, "PERCINVESTIMENTO") == 0)
                meta->percentualInvestimento = valor;

            else if (strcmp(nome, "PATRIMONIOACC") == 0)
                meta->patrimonioAcumulado = valor;

            else if (strcmp(nome, "IDADEFIMRETIRADA") == 0)
                meta->idadeFimRetirada = (int)valor;
        }

    }

    fclose(file);

}
NodeAplicacoes *ler_arquivoCap(const char *arquivoCap) {

    FILE *file = fopen(arquivoCap, "r");
    if (!file) {
        perror("Erro ao abrir o arquivo-capitalizacao");
        exit(1);
    }

    char linha[256];
    NodeAplicacoes *lista = NULL;    // início da lista
    NodeAplicacoes *ultimo = NULL;   // último nó para inserção


    while (fgets(linha, sizeof(linha), file)!= NULL) {
        if (linha[0] == '#' || linha[0] == '\n') {
            continue;
        }
        if (linha[0] == 'f') {
            break;
        }

        char tipo; // i = investimento

        // Aloca o novo nó antes de preencher os dados
        NodeAplicacoes *novo = malloc(sizeof(NodeAplicacoes));

        if (!novo) {
            fprintf(stderr, "Erro de alocação de memória\n");
            exit(1);
        }

        if (sscanf(linha, "%c %99s %99s %f %d",
                   &tipo, novo->inv.nomeAtivo, novo->inv.categoria, &novo->inv.taxaRetorno, &novo->inv.risco) == 5) {

            if (tipo != 'i') {
                free(novo); // descarta se o tipo não for 'i'
                continue;
            }

            novo->next = NULL; // sempre NULL pois será o último nó

            if (lista == NULL) {
                // lista vazia: novo nó é a cabeça
                lista = novo;
                ultimo = novo;
            } else {
                // lista não vazia: adiciona no final
                ultimo->next = novo;
                ultimo = novo;
            }

                   } else {
                       free(novo);
                   }
    }

    fclose(file);
    return lista;

}
NodeAplicacoes* ler_arquivoDescap(const char *arquivoDescap) {
    FILE *file = fopen(arquivoDescap, "r");
    if (!file) {
        perror("Erro ao abrir o arquivo de descapitalização");
        exit(1);
    }

    char linha[256];
    NodeAplicacoes *lista = NULL;    // início da lista
    NodeAplicacoes *ultimo = NULL;   // último nó para inserção

    while (fgets(linha, sizeof(linha), file)!= NULL) {
        if (linha[0] == '#' || linha[0] == '\n') {
            continue;
        }
        if (linha[0] == 'f') {
            break;
        }

        char tipo;

        // Aloca o novo nó
        NodeAplicacoes *novo = malloc(sizeof(NodeAplicacoes));
        if (!novo) {
            fprintf(stderr, "Erro de alocação de memória\n");
            exit(1);
        }

        if (sscanf(linha, "%c %99s %99s %f %d",
                   &tipo, novo->inv.nomeAtivo, novo->inv.categoria, &novo->inv.taxaRetorno, &novo->inv.risco) == 5) {


            if (tipo != 'i' || novo->inv.risco > 2) {
                free(novo); // ignora se não for 'i' ou se risco for maior que 2
                continue;
            }

            // Insere no início da lista
            novo->next = lista;
            lista = novo;

                   } else {
                       free(novo);
                   }
    }

    fclose(file);
    return lista;
}

//funcao de descapitalizacao
double fDescapitalizacao(double pmt_ret,int n, double pv, double taxa) {
    double potencia = pow((1+taxa),(-n));
    return pmt_ret*(1 - potencia)/taxa - pv;

}

//derivada finita central
double derivadaDesc(double pmt_ret, int n, double pv, double taxa) { //     f ′(x) ≈ (f(x+ h)− f(x− h)) / 2h
    double fmais, fmenos;
    double derivada;


    fmais = fDescapitalizacao(pmt_ret+EPSILON,n,pv,taxa);  // f(i + h) a função depende dos três valores para realizar a conta
    fmenos = fDescapitalizacao(pmt_ret-EPSILON,n,pv,taxa); // f(i - h)
    derivada = (fmais - fmenos) / (2 * EPSILON);

    return derivada;
}

double newtonDescapitalizacao(double pmt_ret, int n, double pv, double taxa) {
    int j=0;

    double x = pmt_ret;
    double Fx= fDescapitalizacao(pmt_ret,n,pv,taxa);
    double Dx= derivadaDesc(pmt_ret, n,pv,taxa);
    double deltax;

    while (Dx!=0.0 && j< MAX_ITER) {
        deltax = -Fx/Dx;
        x= x+deltax;
        Fx= fDescapitalizacao(x,n,pv,taxa);
        Dx= derivadaDesc(x, n,pv,taxa);
        j++;

        if(fabs((deltax)<=TOLER) && fabs(Fx<=TOLER)){
            return (x);
        }
    }

    return 0;
}

double newtonCapitalizao(double i, double PMT, int n, double FV) {
    int j=0;

    double x = i;
    double Fx= fCapitalizacao(i, PMT, n, FV);
    double Dx= derivadaCapit(i, PMT, n, FV);
    double deltax;



    while (Dx!=0.0 && j< MAX_ITER) {

        deltax = -Fx/Dx;
        x= x+deltax;
        Fx= fCapitalizacao(x, PMT, n, FV);
        Dx= derivadaCapit(x, PMT, n, FV);
        j++;

        if(fabs((deltax)<=TOLER) && fabs(Fx)<=TOLER){
            return (x);
        }
    }

    return 0;
}

double fCapitalizacao(double i, double PMT, int n, double FV) {
    double base;
    double potencia;
    double numerador;
    double divisao;
    double resultado;
    double f;

    base = 1 + i;
    potencia = pow(base, n);      //  f(i) = PMT · ((1 + i)^n − 1) / i − FV
    numerador = potencia - 1;
    divisao = numerador / i;
    resultado = PMT * divisao;
    f = resultado - FV;

    return f;
}

double derivadaCapit(double i, double PMT, int n, double FV) { //     f ′(x) ≈ (f(x+ h)− f(x− h)) / 2h

    double fmais, fmenos;
    double derivada;


    fmais = fCapitalizacao(i + EPSILON, PMT, n, FV);  // f(i + h) a função depende dos três valores para realizar a conta
    fmenos = fCapitalizacao(i - EPSILON, PMT, n, FV); // f(i - h)
    derivada = (fmais - fmenos) / (2 * EPSILON);

    return derivada;
}

// proximoNo= o proximo no da lista
//  noAtual = Aponta para o nó que está sendo analisado no momento
// aux = to usando para fazer a troca de valores entre dois nós

void bubbleSort(NodeAplicacoes **inicioLista) {
    if (*inicioLista == NULL) return;

    int houveTroca; // Um indicador lógico (flag)
    NodeAplicacoes *limiteOrdenado = NULL;

    do {
        houveTroca = 0;
        NodeAplicacoes *noAtual = *inicioLista;

        while (noAtual->next != limiteOrdenado) {
            if (noAtual->inv.risco > noAtual->next->inv.risco) {
                Aplicacoes temp = noAtual->inv; // Usa uma struct temporária
                noAtual->inv = noAtual->next->inv;
                noAtual->next->inv = temp;
                houveTroca = 1;
            }
            noAtual = noAtual->next;
        }
        limiteOrdenado = noAtual;
    } while (houveTroca);
}

//Funcoes do relatorio de saida
int escrever_relatorio_capitalizacao(FILE *fp,Meta meta,double taxa_calculada,CapSelecionado capSel) {
    fprintf(fp, "ETAPA: CAPITALIZAÇÃO\n");

    // nao ha aplicacao suficiente
    if (capSel.capitalizado <= 0.0 || strlen(capSel.nomeAtivo) == 0) {
        fprintf(fp,
            "Não existe aplicação de capitalização capaz de obter o valor de '%.2f' "
            "com a taxa '%.5f'.\n",
            meta.patrimonioAcumulado,
            taxa_calculada);
        return 0;  // encerra o relatorio aqui
    }

    // Caso viavel, imprime resumo final
    fprintf(fp,
        "O valor total obtido ao final de '%d' meses, com pagamentos de '%.2f' reais "
        "aplicados em '%s' a uma taxa de juros de '%.5f' será de '%.2f' reais:\n",
        capSel.periodo,
        capSel.pagamento,
        capSel.nomeAtivo,
        capSel.juros,
        capSel.capitalizado);

    // Cabecalho da tabela
    fprintf(fp, "Periodo Pagamento Capitalizado Juros\n");
    fprintf(fp, "====== ========= =========== =====\n");

    // Detalha periodo a periodo
    for (int k = 1; k <= capSel.periodo; ++k) {
        double pag_acum    = capSel.pagamento * k;
        double fv          = capSel.pagamento * (pow(1 + capSel.juros, k) - 1) / capSel.juros;
        double juros_acum  = fv - pag_acum;
        fprintf(fp, "%6d %9.2f %10.2f %6.2f\n",
                k, pag_acum, fv, juros_acum);
    }

    fprintf(fp, "\n");  // linha em branco antes da proxima secao
    return 1;  // continuar para descapitalizacao
}

void escrever_relatorio_descapitalizacao(FILE *fp,Meta meta,CapSelecionado capSel,NodeOpcoesDescap *listaDescap) {
    int num_periodos = (meta.idadeFimRetirada - meta.idadeFimCapitalizacao) * 12;

    fprintf(fp, "ETAPA: DESCAPITALIZAÇÃO\n");
    fprintf(fp,
        "O patrimônio acumulado de '%.2f' será consumido ao longo de '%d' meses.\n"
        "As retiradas mensais possíveis são:\n",
        capSel.capitalizado,
        num_periodos);

    fprintf(fp, "No.Aplicacao       Taxa    Retirada\n");
    fprintf(fp, "== ==============  ======= ==========\n");

    int idx = 1;
    for (NodeOpcoesDescap *p = listaDescap; p != NULL; p = p->next, ++idx) {
        fprintf(fp,
            "%2d %-12s %6.4f %10.2f\n",
            idx,
            p->opcoesDescap.nomeAtivo,
            p->opcoesDescap.taxa,
            p->opcoesDescap.retirada);
    }

    fprintf(fp, "\n");  // fim do relatorio
}