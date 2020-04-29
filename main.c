#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
    char error[100];
    char renglon[10];
    strcpy(error,obtenerFecha());
    strcat(error," ");
    strcat(error,"La fila [");sprintf(renglon, "%d",numRenglon+1);
    strcat(error,renglon);strcat(error,"] esta mal formada\n");
    fputs(error,fp_errores);
    printf("%s",error);
}

int obtenerNumeroColumnas(FILE* fp_errores,FILE* fp, int numRenglon)
{
    char buff[255], *m=fgets(buff,255,fp), columna[255];
    int i=0,contador=0, inicioColumna=0, terminoColumna=0,columnaContador=0; 
    memset(&columna[0], 0, sizeof(columna));
    while(buff[i]!='\0')
    {
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
                printf("[%s]",columna);
                contador++;
            }
        }

        //caso 2 no estan entre comilla doble
        if(inicioColumna==0)    
        {
            if(buff[i]!=','&&buff[i]!='\n')
                columna[columnaContador++]=buff[i];
            if(buff[i]==','|| buff[i]=='\n'|| buff[i]=='\000')
            {   
                if(columna[0]!='\0')
                {
                    printf("[%s]",columna);//se formo toda la columna del csv
                    contador++;
                }
                memset(&columna[0], 0, sizeof(columna));
                columnaContador=0;
            }
        }

        if(inicioColumna==1&&terminoColumna==0&& (buff[i]=='\n'||buff[i]=='\0'))//se abrieron comillas dobles pero no se cerraron
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
    return contador;
}

int obtenerNumeroRenglones(FILE *fp_errores, char* nombreArchivo)
{
    FILE *pfCommand;
    char command[40];
    char data[512];

    char comando[100];
    strcpy(comando, "wc -l "); strcat(comando,nombreArchivo);
    strcat(comando," | awk '{printf $1}'");
    // Obtengo el numero de renglones
    //wc -l ./prueba.csv | awk '{printf $1}'
    sprintf(command, comando); 

    // Setup our pipe for reading and execute our command.
    pfCommand = popen(command,"r"); 

    // Get the data from the process execution
    char* m=fgets(data, 512 , pfCommand);

    // the data is now in 'data'
    //se convierte el dato a entero
    int numRenglones=atoi(data);
    
    if (pclose(pfCommand) != 0){
        fprintf(stderr," Error: Failed to close command stream \n");
        fputs(obtenerFecha(),fp_errores);
        fputs(" Error: Failed to close command stream \n",fp_errores);
    }
    return numRenglones+1;
}

void imprimirErroresColumnas(int i, FILE* fp_errores)
{
    char error[100];
    char numRenglon1Error[10];
    strcpy(error,obtenerFecha());
    strcat(error," El numero de columnas del renglon [");
    sprintf(numRenglon1Error,"%d",i+1);
    strcat(error,numRenglon1Error);
    strcat(error,"] y [1] no coincide\n");
    printf("%s",error);
    fputs(error,fp_errores);
}

void leerCsv()
{
    char *archivoEntrada="prueba.csv";
    char nombreArchivo[100];
    strcpy(nombreArchivo,"./");
    strcat(nombreArchivo,archivoEntrada);
    FILE *fp, *fp_output, *fp_errores;
    fp = fopen(nombreArchivo, "r");
    //fp_output=fopen("./salida.sql","w+");
    fp_errores=fopen("./errores.log","w+");
    int numRenglones=obtenerNumeroRenglones(fp_errores, nombreArchivo);
    int numColumnas[numRenglones];
    
    for (int i=0;i<numRenglones;i++)
    {
        numColumnas[i]=obtenerNumeroColumnas(fp_errores,fp, i);
        if(i>0)
            if(numColumnas[i]!=numColumnas[0])
            {
                imprimirErroresColumnas(i,fp_errores);
            }
        printf("\n");
    }   
    fclose(fp_errores);
    //fclose(fp_output);
    fclose(fp);
}

void main()
{
    leerCsv();
}