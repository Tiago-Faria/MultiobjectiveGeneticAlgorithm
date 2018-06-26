#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define VALOR_MAXIMO_DE_X 6.0
#define TAMANHO_POPULACAO 500
#define TAMANHO_POP_EXTERNA 20
#define MUTACOES_A_CADA_100 10
#define NUMERO_DE_GERACOES 10


typedef struct individuo{
    int id;
    int fronteira;
    double fitness[2];
    double cromossomo[5];
    int strength;
    int rawFitness;
    double aptidao;
}Individuo;

typedef struct populacao{
    Individuo *individuo[TAMANHO_POPULACAO];
    int size;
}Populacao;

///Variaveis Globais
int idCounter = 0;
Individuo individuoSendoCalculado;

double funcao_fitness_1(Individuo i);
double funcao_fitness_2(Individuo i);
void cria_populacao(Individuo **populacao, int quantidade);
int isDominated(Individuo ind1,Individuo ind2);
int calculaStength(Individuo ind, Populacao popP, Populacao popE);
void calculaRawFitness(Individuo *ind, Populacao popP, Populacao popE);
void calculaAptidaoPopulacao(Populacao popP, Populacao popE);
void calculaAptidao(Individuo *ind, Individuo **individuos, int tamanho);
int comparaIndividuo(const void *ind1, const void *ind2);
double distanciaEntreIndividuos(Individuo ind1, Individuo ind2);
void imprimeIndividuo(Individuo ind);
void imprimePopulacao(Populacao pop);
int nthVizinhoMaisProximo(Populacao pop, int individuo, int n);
int vizinhoMaisProximo(Populacao pop, int individuo, int firstViable);
void truncamento(Populacao *pop);
int vizinhoDeMelhorAptidao(Populacao pop);
void selecaoPorTorneio(Populacao pop, int *ind1, int *ind2);
void crossover(Individuo pai1, Individuo pai2, Individuo *popNova, int posicao);
Individuo copiarIndividuo(Individuo ind);

int main()
{
    srand((unsigned)time(NULL));
    Individuo *pop;
    Individuo popNova[TAMANHO_POPULACAO];
    Individuo *popAux;
    int i,j,flagDominated,contadorGeracao;
    int melhor;
    int pai1,pai2;
    cria_populacao(&(pop),TAMANHO_POPULACAO);

    Populacao P,E;
    for(contadorGeracao = 0;contadorGeracao < NUMERO_DE_GERACOES;contadorGeracao++){
        printf("\n\niniciando geracao %d",contadorGeracao);
        P.size = 0;
        E.size = 0;
        for(i=0;i<TAMANHO_POPULACAO;i++){
            P.individuo[i] = -1;
            E.individuo[i] = -1;
        }

        for(i=0;i<TAMANHO_POPULACAO;i++){
            flagDominated = 0;
            for(j=0;j < TAMANHO_POPULACAO && flagDominated == 0;j++){
                if(isDominated(pop[i],pop[j]) == 1) flagDominated = 1;
            }
            if(flagDominated){
                P.individuo[P.size] = &(pop[i]);
                P.size++;
            } else{
                E.individuo[E.size] = &(pop[i]);
                E.size++;
            }
        }

        ///Calcula strenght das duas populações
        printf("\ncalculando strenght das populacoes...");
        for(i=0;i<P.size;i++){
            if(P.individuo[i] != -1){
                P.individuo[i]->strength =  calculaStength(*(P.individuo[i]),P,E);
                //printf("\nstr: %d",P.individuo[i]->strength);
            }
        }
        for(i=0;i<E.size;i++){
            if(E.individuo[i] != -1){
                E.individuo[i]->strength =  calculaStength(*(E.individuo[i]),P,E);
            }
        }
        ///Calcula o raw fitness da populaçao P
        printf("\ncalculando raw fitness...");
        for(i=0;i<P.size;i++){
            if(P.individuo[i] != -1){
                calculaRawFitness(P.individuo[i],P,E);
            }
        }

        ///Calcula Aptidao das populações
        printf("\ncalculando aptidao...");
        calculaAptidaoPopulacao(P,E);

        ///Corrige o tamanho da população externa
        printf("\ncorrigindo tamanho da populacao externa...");
        if(E.size > TAMANHO_POP_EXTERNA){
            truncamento(&E);
        } else if(E.size < TAMANHO_POP_EXTERNA){
            do{
                melhor = vizinhoDeMelhorAptidao(P);
                E.individuo[E.size] = P.individuo[melhor];
                E.size++;
                P.individuo[melhor] = P.individuo[P.size - 1];
                P.size--;
            }while(E.size < TAMANHO_POP_EXTERNA);
        }

        ///Avalia se há condição de termino
        if(contadorGeracao == NUMERO_DE_GERACOES - 1){
            break;
        }

        ///Cria População nova
        printf("\ncriando populacao nova...");
        for(i=0;i<TAMANHO_POP_EXTERNA;i++){
            popNova[i] = copiarIndividuo(*(E.individuo[i]));
        }
        for(i=TAMANHO_POP_EXTERNA;i<TAMANHO_POPULACAO;i+=2){
            selecaoPorTorneio(E,&pai1,&pai2);
            crossover(*(E.individuo[pai1]),*(E.individuo[pai2]),popNova,i);
        }

        for(i=0;i<TAMANHO_POPULACAO;i++){
            pop[i] = copiarIndividuo(popNova[i]);
        }
    }
    FILE *arquivo = fopen("output.txt","w");
    for(i=0;i<E.size;i++){
        fprintf(arquivo,"%lf %lf\n",E.individuo[i]->fitness[0],E.individuo[i]->fitness[1]);
    }
    for(i=0;i<P.size;i++){
        fprintf(arquivo,"%lf %lf\n",P.individuo[i]->fitness[0],P.individuo[i]->fitness[1]);
    }
    fclose(arquivo);
    double menorDistancia,somaDasDistancias = 0;
    double menorFitness[2] = {1000.0,1000.0};
    double maiorFitness[2] = {0.0,0.0};
    int kAux, errorRate = 0;
    for(i=0;i<E.size;i++){
        kAux = vizinhoMaisProximo(E,i,0);
        menorDistancia = distanciaEntreIndividuos(*(E.individuo[i]),*(E.individuo[kAux]));
        somaDasDistancias += menorDistancia;
        if(((E.individuo[i]->cromossomo[0] >= 1 && E.individuo[i]->cromossomo[0] <= 1.5)  ||
            (E.individuo[i]->cromossomo[0] >= 3 && E.individuo[i]->cromossomo[0] <= 3.5)  ||
            (E.individuo[i]->cromossomo[0] >= 5 && E.individuo[i]->cromossomo[0] <= 5.5)) &&
            (E.individuo[i]->cromossomo[1] >= 1 && E.individuo[i]->cromossomo[1] <= 1.5)  ||
            (E.individuo[i]->cromossomo[1] >= 3 && E.individuo[i]->cromossomo[1] <= 3.5)  ||
            (E.individuo[i]->cromossomo[1] >= 5 && E.individuo[i]->cromossomo[1] <= 5.5)){}
        else errorRate++;
        for(int cont = 0;cont<2;cont++){
            if(E.individuo[i]->fitness[cont] > maiorFitness[cont]){
                maiorFitness[cont] = E.individuo[i]->fitness[cont];
            }
            if(E.individuo[i]->fitness[cont] < menorFitness[cont]){
                menorFitness[cont] = E.individuo[i]->fitness[cont];
            }
        }
    }
    for(i=0;i<P.size;i++){
        kAux = vizinhoMaisProximo(P,i,0);
        menorDistancia = distanciaEntreIndividuos(*(P.individuo[i]),*(P.individuo[kAux]));
        somaDasDistancias += menorDistancia;
        if(((P.individuo[i]->cromossomo[0] >= 1 && P.individuo[i]->cromossomo[0] <= 1.5)  ||
            (P.individuo[i]->cromossomo[0] >= 3 && P.individuo[i]->cromossomo[0] <= 3.5)  ||
            (P.individuo[i]->cromossomo[0] >= 5 && P.individuo[i]->cromossomo[0] <= 5.5)) &&
           ((P.individuo[i]->cromossomo[1] >= 1 && P.individuo[i]->cromossomo[1] <= 1.5)  ||
            (P.individuo[i]->cromossomo[1] >= 3 && P.individuo[i]->cromossomo[1] <= 3.5)  ||
            (P.individuo[i]->cromossomo[1] >= 5 && P.individuo[i]->cromossomo[1] <= 5.5))){}
        else errorRate++;
        for(int cont = 0;cont<2;cont++){
            if(P.individuo[i]->fitness[cont] > maiorFitness[cont]){
                maiorFitness[cont] = P.individuo[i]->fitness[cont];
            }
            if(P.individuo[i]->fitness[cont] < menorFitness[cont]){
                menorFitness[cont] = P.individuo[i]->fitness[cont];
            }
        }
    }
    double maxSpread = 0.0;
    for(i=0;i<2;i++){
        maxSpread += (maiorFitness[i] - menorFitness[i])*(maiorFitness[i] - menorFitness[i]);
    }
    maxSpread = sqrt(maxSpread);
    printf("\nA generational distance é %lf",somaDasDistancias/TAMANHO_POPULACAO);
    printf("\nO error rate é de %lf",(double)errorRate/TAMANHO_POPULACAO);
    printf("\nO pareto subset é %d",TAMANHO_POPULACAO - errorRate);
    printf("\nO Max Spread é %lf",maxSpread);
    //imprimePopulacao(P);
    //imprimePopulacao(E);
    return 0;
}

int isDominated(Individuo ind1,Individuo ind2){
    int i;
    int isBetterInSomething = 0;
    for(i = 0;i < 2;i++){
        if(ind2.fitness[i] > ind1.fitness[i])return 0;
        else if(ind2.fitness[i] < ind1.fitness[i])isBetterInSomething = 1;
    }
    return isBetterInSomething;
}

double funcao_fitness_1(Individuo i){
    int j ;
    double sum = 0;
    for(j = 0;j < 5;j++){
        sum +=  sin((double)i.cromossomo[j] * M_PI);
    }
    return sum;
}
double funcao_fitness_2(Individuo i){
    int j ;
    double sum = 0;
    for(j = 0;j < 5;j++){
        sum +=  cos(i.cromossomo[j] * M_PI);
    }
    return sum;
}
void cria_populacao(Individuo **populacao, int quantidade){
    int i,j;
    Individuo *indi;
    *populacao = malloc(sizeof(Individuo)*quantidade);

    for(i = 0;i < quantidade; i++){
        indi = &((*populacao)[i]);
        for(j = 0;j < 5;j++){
            (*indi).cromossomo[j] = ((double)rand()/(double)RAND_MAX) * VALOR_MAXIMO_DE_X;
        }
        (*indi).fitness[0] = funcao_fitness_1(*indi);
        (*indi).fitness[1] = funcao_fitness_2(*indi);
        (*indi).rawFitness = 0;
        (*indi).aptidao = 0;
        (*indi).id = idCounter ++;
     }
}
int calculaStength(Individuo ind, Populacao popP, Populacao popE){
    int i,strength = 0;
    for(i=0;i<popP.size;i++){
        if(popP.individuo[i] != -1){
            if(isDominated(*(popP.individuo[i]),ind)){
                strength ++;
            }
        }
    }
    for(i=0;i<popE.size;i++){
        if(popE.individuo[i] != -1){
            if(isDominated(*(popE.individuo[i]),ind)){
                strength++;
            }
        }
    }
    return strength;
}

void calculaRawFitness(Individuo *ind, Populacao popP, Populacao popE){
    int i;
    ind->rawFitness = 0;
    for(i=0;i<popP.size;i++){
        if(popP.individuo[i] != -1){
            if(isDominated(*ind,*(popP.individuo[i]))){
                ind->rawFitness += popP.individuo[i]->strength;
            }
        }
    }
    for(i=0;i<popE.size;i++){
        if(popE.individuo[i] != -1){
            if(isDominated(*ind,*(popE.individuo[i]))){
                ind->rawFitness += popE.individuo[i]->strength;
            }
        }
    }
}
void calculaAptidaoPopulacao(Populacao popP, Populacao popE){
    int TamanhoPopulacoes,i;
    Individuo *individuos[TAMANHO_POPULACAO + TAMANHO_POP_EXTERNA];
    Individuo *copiaIndividuos[TAMANHO_POPULACAO + TAMANHO_POP_EXTERNA];
    TamanhoPopulacoes = popE.size + popP.size;


    for(i = 0;i < popP.size;i++){
        individuos[i] = popP.individuo[i];
        copiaIndividuos[i] = popP.individuo[i];
    }
    for(i = 0;i < popE.size;i++){
        individuos[popP.size + i] = popE.individuo[i];
        copiaIndividuos[popP.size + i] = popE.individuo[i];
    }
    for(i = 0;i < TamanhoPopulacoes;i++){
        calculaAptidao(individuos[i], copiaIndividuos, TamanhoPopulacoes);
        //imprimeIndividuo(*individuos[i]);
    }
}
void calculaAptidao(Individuo *ind, Individuo **individuos, int tamanho){
    double d;
    int k;
    k = sqrt(tamanho);
    individuoSendoCalculado = *ind;
    qsort(individuos,tamanho,sizeof(Individuo*),comparaIndividuo);
    d = 1/(distanciaEntreIndividuos(individuoSendoCalculado,*individuos[k]) + 2);
    ind->aptidao = ind->rawFitness + d;
    //imprimeIndividuo(*ind);
}
int comparaIndividuo(const void *ind1,const void *ind2){
    //imprimeIndividuo(*((Individuo*)ind1));
    if( distanciaEntreIndividuos(individuoSendoCalculado,**(Individuo **)ind1) > distanciaEntreIndividuos(individuoSendoCalculado,**(Individuo**)ind2))return 1;
    else if( distanciaEntreIndividuos(individuoSendoCalculado,**((Individuo**)ind1)) < distanciaEntreIndividuos(individuoSendoCalculado,**((Individuo**)ind2)))return -1;
    else return 0;
}
double distanciaEntreIndividuos(Individuo ind1, Individuo ind2){
    double soma = 0;
    int i;
    //imprimeIndividuo(ind1);
    //imprimeIndividuo(ind2);
    for(i = 0;i < 2;i++){
        soma += (ind1.fitness[i] - ind2.fitness[i]) * (ind1.fitness[i] - ind2.fitness[i]);
    }
    return sqrt(soma);
}
void imprimeIndividuo(Individuo ind){
    int i;
    printf("\n--- individuo %d ---",ind.id);
    for(i=0;i<2;i++){
        printf("\nfitness %d: %f",i,ind.fitness[i]);
    }
    printf("\nrawfit:%d, str:%d, aptidao:%lf",ind.rawFitness,ind.strength,ind.aptidao);
    printf("\ncromossomos:[");
    for(i=0;i<5;i++){
        printf("%f, ",ind.cromossomo[i]);
    }
    printf("]");
}
void imprimePopulacao(Populacao pop){
    int i;
    for(i=0;i<pop.size;i++){
        if(pop.individuo != -1)imprimeIndividuo(*(pop.individuo[i]));
    }
    printf("\n --------------------------");
}

int nthVizinhoMaisProximo(Populacao pop, int individuo, int n){
    int i , menor;
    Individuo* aux;
    for(i = 0;i < n - 1;i++){
        menor = vizinhoMaisProximo(pop,individuo,i);
        aux = pop.individuo[menor];
        pop.individuo[menor] = pop.individuo[i];
        pop.individuo[i] = aux;
    }
    return vizinhoMaisProximo(pop,individuo,n-1);
}
int vizinhoMaisProximo(Populacao pop, int individuo, int firstViable){
    int i,MaisProximo = -1;
    double distancia, MenorDistancia  = 100;
    for(i=firstViable;i<pop.size;i++){
        if(i != individuo){
            distancia = distanciaEntreIndividuos(*pop.individuo[i],*pop.individuo[individuo]);
            if(distancia < MenorDistancia){
                MenorDistancia = distancia;
                MaisProximo = i;
            }
        }
    }
    return MaisProximo;
}
void truncamento(Populacao *pop){
    int i,j,individuoComMenorDistanciaA,vizinho,individuoComMenorDistanciaB;
    double menorDistancia = 100,distancia;
    double menorDistanciaDoA, menorDistanciaDoB;
    int individuoRetirado;
    while(pop->size > TAMANHO_POP_EXTERNA){
        menorDistancia = 100;

        for(j=0;j<pop->size;j++){
            vizinho = vizinhoMaisProximo(*pop,j,0);
            distancia = distanciaEntreIndividuos(*(pop->individuo[j]),*(pop->individuo[vizinho]));
            if( distancia < menorDistancia){
                menorDistancia = distancia;
                individuoComMenorDistanciaA = j;
                individuoComMenorDistanciaB = vizinho;
            }
        }
        j = 2;
        menorDistanciaDoA = menorDistanciaDoB = menorDistancia;
        while(menorDistanciaDoA == menorDistanciaDoB && j < pop->size){
            menorDistanciaDoA = distanciaEntreIndividuos(*(pop->individuo[individuoComMenorDistanciaA]),*(pop->individuo[nthVizinhoMaisProximo(*pop,individuoComMenorDistanciaA,j)]));
            menorDistanciaDoB = distanciaEntreIndividuos(*(pop->individuo[individuoComMenorDistanciaB]),*(pop->individuo[nthVizinhoMaisProximo(*pop,individuoComMenorDistanciaB,j)]));
            j++;
        }
        if(menorDistanciaDoA < menorDistanciaDoB){
            individuoRetirado = individuoComMenorDistanciaA;
        } else{
            individuoRetirado = individuoComMenorDistanciaB;
        }
        pop->individuo[individuoRetirado] = pop->individuo[pop->size -1];
        pop->individuo[pop->size -1] = -1;
        pop->size--;
    }
}

int vizinhoDeMelhorAptidao(Populacao pop){
    int i,melhor;
    double melhorAptidao;
    melhor = 0;
    melhorAptidao = pop.individuo[0]->aptidao;
    for(i=1;i<pop.size;i++){
        if(pop.individuo[i]->aptidao < melhorAptidao){
            melhor = i;
            melhorAptidao = pop.individuo[i]->aptidao;
        }
    }
    return melhor;
}

void selecaoPorTorneio(Populacao pop, int *ind1, int *ind2){
    int i,j;
    i = rand()%pop.size;
    do{
        j = rand()%pop.size;
    } while(i == j);
    if(pop.individuo[i]->aptidao < pop.individuo[j]->aptidao){
        *ind1 = i;
    } else{
        *ind1 = j;
    }
    do{
        i = rand()%pop.size;
        do{
            j = rand()%pop.size;
        } while(i == j);
        if(pop.individuo[i]->aptidao < pop.individuo[j]->aptidao){
            *ind2 = i;
        } else{
            *ind2 = j;
        }
    } while (*ind1 == *ind2);
}

void crossover(Individuo pai1, Individuo pai2, Individuo *popNova, int posicao){
    Individuo filho1,filho2;
    int corte = rand()%5;
    int i;
    for(i=0;i<corte;i++){
        filho1.cromossomo[i] = pai1.cromossomo[i];
        filho2.cromossomo[i] = pai2.cromossomo[i];
    }
    for(i=corte;i<5;i++){
        filho1.cromossomo[i] = pai2.cromossomo[i];
        filho2.cromossomo[i] = pai1.cromossomo[i];
    }
    if(rand()%100 < MUTACOES_A_CADA_100){
        filho1.cromossomo[rand()%5] = ((double)rand()/(double)RAND_MAX) * VALOR_MAXIMO_DE_X;
    }
    if(rand()%100 < MUTACOES_A_CADA_100){
        filho2.cromossomo[rand()%5] = ((double)rand()/(double)RAND_MAX) * VALOR_MAXIMO_DE_X;
    }
    filho1.fitness[0] = funcao_fitness_1(filho1);
    filho1.fitness[1] = funcao_fitness_2(filho1);
    filho1.rawFitness = 0;
    filho1.aptidao = 0;
    filho1.id = idCounter ++;

    filho2.fitness[0] = funcao_fitness_1(filho2);
    filho2.fitness[1] = funcao_fitness_2(filho2);
    filho2.rawFitness = 0;
    filho2.aptidao = 0;
    filho2.id = idCounter ++;

    popNova[posicao] = filho1;
    popNova[posicao+1] = filho2;
}
Individuo copiarIndividuo(Individuo ind){
    Individuo copia;
    int i;
    for(i=0;i<5;i++) copia.cromossomo[i] = ind.cromossomo[i];
    for(i=0;i<2;i++) copia.fitness[i] = ind.fitness[i];
    copia.id = ind.id;
    copia.aptidao = ind.aptidao;
    copia.fronteira = ind.fronteira;
    copia.rawFitness = ind.rawFitness;
    copia.strength = ind.strength;
    return copia;
}
