#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct colum colum ;
struct colum{
    char *contenido;
    colum* next;
};

typedef struct nodoColumnas nodoColumnas;

struct nodoColumnas{
    colum* col;
    int numeroColumnas;
};

char* obtenerFecha()
{
    time_t timer;
    char *buffer=malloc(sizeof(char));
    struct tm* tm_info;

    timer = time(NULL);
    tm_info = localtime(&timer);

    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    //puts(buffer);
    return buffer;
}

void imprimirErroresFilas(FILE* fp_errores, int numRenglon)
{
    char error[100], renglon[10];
    strcpy(error,obtenerFecha());
    strcat(error," ");
    strcat(error,"La fila [");
    sprintf(renglon, "%d",numRenglon+1);
    strcat(error,renglon);
    strcat(error,"] esta mal formada\n");
    fputs(error,fp_errores);
    printf("%s",error);
    //memset(&error[0], 0, sizeof(error));
    //memset(&renglon[0], 0, sizeof(renglon));
}

int obtenerLongitudColumna(char* columna)
{
    if(columna==NULL)
        return 0;
    if(columna[0]=='\0')
        return 0;
    int i=0;
    while(columna[i]!='\0')
    {
        i++;
    }
    return i+1;
}

colum* agregarColumna(nodoColumnas *nodo,char * columna, colum* p)
{
    int columnaLongitud = obtenerLongitudColumna(columna);
    //printf("AGREGAR COLUMNA");
    {
        if(nodo->col!=NULL)
        {   //se añade la primera vez, cuando no hay datos
            if(!nodo->col->contenido)
            {
                nodo->col->contenido=(char*)malloc(columnaLongitud);
                strcpy(nodo->col->contenido,columna);
                nodo->col->next=malloc(sizeof(colum));
                p=nodo->col->next;
                ////printf("%s\n",nodo->col->contenido);
            }
            else//cuando la lista tiene por lo menos un dato(nodo) con información
            {   
                if(!p)
                    p=(colum*)malloc(sizeof(colum));
                p->contenido=(char*)malloc(columnaLongitud);
                strcpy(p->contenido,columna);
                ////printf("%s\n",p->contenido);
                p->next=malloc(sizeof(colum));
                p=p->next;
            }
        }
    //memset(&columna[0], 0, sizeof(columna));
    return p;    
    }   
}

//Para el caso 1, solo se permiten comillas dobles cuando se abre y termina el contenido de un campo
//no se permiten comilla doble intermedia
nodoColumnas* obtenerNumeroColumnas(FILE* fp_errores,FILE* fp, int numRenglon)
{
    nodoColumnas* nodo=(nodoColumnas*)malloc(sizeof(nodoColumnas));
    nodo->col=(colum*)malloc(sizeof(colum));
    colum *p;

    char buff[512]; memset(&buff[0], 0, sizeof(buff));
    char *m=fgets(buff,512,fp), columna[256];
    int i=0,contador=0, inicioColumna=0, terminoColumna=0,columnaContador=0; 
    memset(&columna[0], 0, sizeof(columna));
    while(buff[i]!='\0')
    {
        //printf("[%c]",buff[i]);
        //caso 1 estan encerrados entre doble comilla
        if(buff[i]=='\"')
        {   
            if(inicioColumna==0)
            {
                inicioColumna=1;
                terminoColumna=0;
            }
            else 
                terminoColumna=1;
        }
        if(inicioColumna==1){
            if(buff[i]!='\"' && buff[i]!='\"')
                columna[columnaContador++]=buff[i];
            if(terminoColumna==1){//Se formo toda la columna del csv
                printf("\n[%s]",columna);
                contador++;
                p=agregarColumna(nodo,columna,p);
            }
        }

        //caso 2 no estan entre comilla doble
        if(inicioColumna==0)    
        {
            if(buff[i]!=','&&buff[i]!='\n')
                columna[columnaContador++]=buff[i];
            if(i>0 && buff[i]==',' &&buff[i-1]==',')
                columna[columnaContador++]=(char)20;
            if(buff[i]==','|| buff[i]=='\n'|| buff[i]=='\000')
            {   
                if(columna[0]!='\0')
                {
                    printf("[%s]",columna);//se formo toda la columna del csv
                    contador++;
                    p=agregarColumna(nodo,columna,p);
                }
                memset(&columna[0], 0, sizeof(columna));
                columnaContador=0;
            }
        }
        
        //se abrieron comillas dobles pero no se cerraron
        if(inicioColumna==1&&terminoColumna==0&& (buff[i]=='\n'||buff[i]=='\0'))
        {
            imprimirErroresFilas(fp_errores, numRenglon);
        }
        i++;
        //caso1
        if(terminoColumna==1)
        {
            //printf("inicio y termino de columna correctmente");
            inicioColumna=0;terminoColumna=0; columnaContador=0;
            memset(&columna[0], 0, sizeof(columna));
        }
    }
    printf("\nNumero de columnas %d\n",contador);
    nodo->numeroColumnas=contador;
    //free(p);
    //memset(&p[0], 0, sizeof(p));
    return nodo;
}

int obtenerNumeroRenglones(FILE *fp_errores, char* nombreArchivo)
{
    FILE *pfCommand;
    int longitudComando=obtenerLongitudColumna(nombreArchivo)+40;
    char command[longitudComando], data[512], comando[longitudComando];
    strcpy(comando, "wc -l "); strcat(comando,nombreArchivo);
    strcat(comando," | awk '{printf $1}'");
    // Obtengo el numero de renglones
    //wc -l ./prueba.csv | awk '{printf $1}'
    sprintf(command, comando); 

    // Setup our pipe for reading and execute our command.
    pfCommand = popen(command,"r"); 

    // Get the data from the process execution
    char* m=fgets(data, longitudComando , pfCommand);

    // the data is now in 'data'
    //se convierte el dato a entero
    int numRenglones=atoi(data);
    
    if (pclose(pfCommand) != 0){
        fprintf(stderr," Error: Failed to close command stream \n");
        fputs(obtenerFecha(),fp_errores);
        fputs(" Error: Failed to close command stream \n",fp_errores);
    }
    memset(&command[0], 0, sizeof(command));
    memset(&data[0], 0, sizeof(data));
    memset(&comando[0], 0, sizeof(comando));
    return numRenglones;
}

void imprimirErroresColumnas(int i, FILE* fp_errores)
{
    char error[100], numRenglon1Error[10];
    strcpy(error,obtenerFecha());
    strcat(error," El numero de columnas del renglon [");
    sprintf(numRenglon1Error,"%d",i+1);
    strcat(error,numRenglon1Error);
    strcat(error,"] y [1] no coincide\n");
    printf("%s",error);
    fputs(error,fp_errores);
    memset(&error[0], 0, sizeof(error));
    memset(&numRenglon1Error[0], 0, sizeof(numRenglon1Error));
}

void leerCsv()
{
    char *archivoEntrada="prueba.csv", nombreArchivo[100];
    strcpy(nombreArchivo,"./");
    strcat(nombreArchivo,archivoEntrada);
    FILE *fp, *fp_output, *fp_errores;
    fp = fopen(nombreArchivo, "r");
    //fp_output=fopen("./salida.sql","w+");
    fp_errores=fopen("./errores.log","w+");
    int numRenglones=obtenerNumeroRenglones(fp_errores, nombreArchivo), numColumnas[numRenglones];
    nodoColumnas *columnasPorRenglon;
    for (int i=0;i<numRenglones;i++)
    {
        printf("RENGLON:%d",i+1);
        columnasPorRenglon=obtenerNumeroColumnas(fp_errores,fp, i);
        numColumnas[i]=columnasPorRenglon->numeroColumnas;
        if(i>0)
            if(numColumnas[i]!=numColumnas[0])
            {
                imprimirErroresColumnas(i,fp_errores);
            }
        printf("\n");
        memset(&columnasPorRenglon[0], 0, sizeof(columnasPorRenglon));
    }   
    fclose(fp_errores);
    //fclose(fp_output);
    fclose(fp);
}

void main()
{
    leerCsv();
}