#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
} ParametrosMeta;

void ler_arquivo_meta(const char *arquivo_meta, ParametrosMeta *meta);

int main(int argc, char **argv) {

    // arquivo-meta arquivo-capitalizacao.txt arquivo-descapitalizacao.txt saida-relatorio.txt
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

    //ARQUIVO META;

    // zerando elementos
    ParametrosMeta meta = {0};
    ler_arquivo_meta(arqMeta, &meta);

    // Teste: imprimir o conteúdo lido do arquivo-meta
    printf("\n=== Dados lidos do arquivo-meta ===\n");
    printf("Idade Formatura: %d\n", meta.idadeFormatura);
    printf("Idade Fim Capitalização: %d\n", meta.idadeFimCapitalizacao);
    printf("Salário Médio Líquido: %.2f\n", meta.salarioMedioLiquido);
    printf("Percentual de Investimento: %.2f\n", meta.percentualInvestimento);
    printf("Patrimônio Acumulado: %.2f\n", meta.patrimonioAcumulado);
    printf("Idade Fim Retirada: %d\n", meta.idadeFimRetirada);

    //ler_arquivoCap();
    //ler_arquivoDescap();
    //gerar_relario();

    return 0;

  }

void ler_arquivo_meta(const char *arquivo_meta, ParametrosMeta *meta) {

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
