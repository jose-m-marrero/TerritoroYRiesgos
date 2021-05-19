#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>

#define PI 3.141592653
//Si existe desbordamiento de memoria, hay que revisar el numero de elementos de entrada
#define VARCON      200  //NUM MAX CAPAS VECTORIALES EN INFILES
#define NUMBAS  3000000  //NUM MAX DE ELEMENTOS CAPA BASE
#define NUMREL  3000000  //NUM MAX DE ELEMENTOS CAPA RELA
#define CWD_MAX     200  //SIZE DIR TRABAJO
#define IN_SIZE     256  //SIZE NOMBRE IN TOTAL
#define OUT_SIZE    256  //SIZE NOMBRE OUT TOTAL

/**
* Copyright (C) 2019  Jose M. Marrero <josemarllin@gmail.com> and Hugo Yepes <hyepes@igepn.edu.ec>
* You may use, distribute and modify this code under the terms of the MIT License.
* The authors will not be held responsible for any damage or losses or for any implications 
* whatsoever resulting from downloading, copying, compiling, using or otherwise handling this 
* source code or the program compiled.
* Nombre script: Analiza_VV.c
* Version: 1.0-2021-02-16 
* Linea de compilacion: gcc Analiza_VV.c -o Analiza_VV -lm
* Linea de ejecucion:  ./Analiza_VV configfilename.cfg
* Relaciona elementos espaciales puntuales por proximidad
* Para cada relacion entre dos capas se escribe un archivo de salida de cada una
*/ 

/**
* GLOBAL VARIABLES
*/
char dirhom[30], dir_out[OUT_SIZE];                                          //workig directories y ruta de salida
char dirbas[200], dirrel[200], dirout[200];                                  //directorios intermedios
char inbas[50], inrel[50], nambas[150], namrel[150];                         //nombres de archivos de entrada
char name_incfg[IN_SIZE], name_file[IN_SIZE], name_vect[IN_SIZE];            //rutas y nombres de archivos de entrada
char outfil[50], out_vecbas[OUT_SIZE], out_vecrel[OUT_SIZE];                 //rutas y nombres de archivos de salida
int n_varfile, n_vecfile, n_base, n_rela;                                    //conteo de datos
int tipmodo, conexit, essum;                                                 //parametros generales
float distmax;                                                               //parametros generales
int hayvar, hayvec;                                                          //parametros infiles
int baspro, bascat, baslin, basdis, basord;                                  //parametros infiles capa base
int relpro, relcat, rellin, reldis, relord;                                  //parametros infiles capa rela
int txtsize, txtsizefin;                                                     //funciones auxiliares

/**
* ALMACENAMIENTO DE DATOS EN ESTRUCTURAS
*/

/*! ALMACENA DATOS INFILES CAPAS VEC-BASE */
typedef struct  
{
	int vaispro; //es calculada
	int vaiscat; //tiene categoria
	int vaislin; //es linea
	int vaisdis; //discretizado
	int vaorder; //orden de la variable
	char varinifile[150]; 
	char varcatfile[150];
	char vartipo[100]; //grupo al que pertenece
}VAR;
VAR varfiles[VARCON];

/*! ALMACENA DATOS INFILES CAPAS VEC-RELA */
typedef struct  
{
	int veispro; //es calculada
	int veiscat; //tiene categoria
	int veislin; //es linea
	int veisdis; //discretizado
	int veorder; //orden de la variable
	char verinifile[150]; 
	char vercatfile[150];
	char vertipo[100]; //grupo al que pertenece
}VEC;
VEC vecfiles[VARCON];

/*! ALMACENA DATOS CAPA VEC-BASE */
typedef struct  
{
	long int basid;     //id punto
	int      bastyp;    //tipologia
	double   basx;      //coory
	double   basy;      //coorx
	float    basmin;    //distancia minima
	long int basrelid;  //el mas cerca
	int      basrelsum; //todos los contenidos en el radio
}VLI;
VLI capabase[NUMBAS];

/*! ALMACENA DATOS CAPA VEC-RELA */
typedef struct  
{
	long int relid;     //id punto
	int      reltyp;    //tipologia
	double   relx;      //coory
	double   rely;      //coorx
	float    relmin;    //distancia minima
	long int relbasid;  //el mas cerca
	int      relbassum; //todos los contenidos en el radio
}DAT;
DAT caparela[NUMREL];


/**
* DECLARACION DE FUNCIONES
*/
int calc_size2(const char* str1, const char* str2);
int calc_size3(const char* str1, const char* str2, const char* str3);
int calc_size4(const char* str1, const char* str2, const char* str3, const char* str4);
void crea_mkdir(char *path, mode_t mode) ;
int getnamout(int numbas, int numrel);
int getnamout_rela(int numrel, int numbas);
void inicvectores(int tipo);


//**********************************************************************
//LECTURA---------------------------------------------------------------
//**********************************************************************

/*! LEYENDO ARCHIVO CONFIGURACION GENERAL */
int read_cfg(void)
{
FILE *file;
char texto[80]; 
	//LEE ARCHIVO DE CONFIGURACION INICIAL
	printf("\n***Reading cfg file, %s***\n", name_incfg);
	if ((file = fopen(name_incfg,"rt"))== NULL)
	{
		printf("-------ERROR open file--------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		exit(0);
	}
	else
	{	
		fscanf(file,"%s", texto);
		fscanf(file,"%s %i", texto, &tipmodo);
		fscanf(file,"%s %i", texto, &conexit);   //salida de bucle en busqueda por radio
		fscanf(file,"%s %f", texto, &distmax);
		fscanf(file,"%s", texto);
		fscanf(file,"%s %s", texto, dirbas);
		fscanf(file,"%s %s", texto, dirrel);
		fscanf(file,"%s %s", texto, dirout);
		fscanf(file,"%s", texto);
		fscanf(file,"%s %s", texto, inbas);
		fscanf(file,"%s %s", texto, inrel);
		
	}
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//CREA DIRECTORIO DE SALIDA SI NO EXISE
	txtsize = calc_size2(dirhom, dirout);
	char dirsal[txtsize];
	sprintf(dirsal,"%s%s", dirhom, dirout);
	if(txtsize < OUT_SIZE) sprintf(dir_out,"%s", dirsal);
	else 
	{
		printf("Se ha excedido dir_out en %i\n", txtsize);
		exit(0);
	}
	crea_mkdir(dir_out, 0777);
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	//IMPRIME DATOS LEIDOS
	printf("TIPO MODO %i\n", tipmodo);
	printf("CON EXIT %i\n", conexit);
	printf("DIR_IN BASE %s\n", dirbas);
	printf("DIR_IN RELA %s\n", dirrel);
	printf("DIR_OUTGEN %s\n", dirout);
	printf("UAE_FILE_BASE %s\n", inbas);
	printf("UAE_FILE_RELA %s\n", inrel);
	printf("RADIO BUSQUEDA %f\n", distmax);
	return 1;		
}

/*! LEYENDO CONFIG VECTOR-CSV INFILES */
int read_infiles(int tipo)
{
FILE *file;
int i;
int tcat, tlin, tdis, tord, tpros;
char texto[256], tnamvar[100],  tnamcat[100], tvartip[100];	
	i = 0;
	//LEE INFILES CON LISTADO DE VETOR BASE Y RELACIONADO
	printf("\n***Lectura archivo variables, %s***\n", name_file);
	if ((file = fopen(name_file,"rt"))== NULL)
	{
		printf("-------ERROR open file--------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		exit(0);
	}
	else
	{
		fgets(texto,256,file); 
		while (fscanf(file,"%i %i %i %i %i %s %s %s" , 
					&tpros,
					&tcat,
					&tlin,
					&tdis,
					&tord,
					tnamvar,
					tnamcat,
					tvartip
					
					) == 8)
		{
			if (tipo == 1)
			{
				varfiles[i].vaispro  = tpros;
				varfiles[i].vaiscat  = tcat;
				varfiles[i].vaislin  = tlin;
				varfiles[i].vaisdis  = tdis;
				varfiles[i].vaorder  = tord;
				sprintf(varfiles[i].varinifile,"%s", tnamvar);
				sprintf(varfiles[i].varcatfile,"%s", tnamcat);
				sprintf(varfiles[i].vartipo,"%s", tvartip);
				printf("%i %i %i %i %i %s %s %s\n", tpros, tcat, tlin, tdis, tord, tnamvar, tnamcat, tvartip);
				if (tpros == 1) hayvar++;
			}
			if (tipo == 2)
			{
				vecfiles[i].veispro  = tpros;
				vecfiles[i].veiscat  = tcat;
				vecfiles[i].veislin  = tlin;
				vecfiles[i].veisdis  = tdis;
				vecfiles[i].veorder  = tord;
				sprintf(vecfiles[i].verinifile,"%s", tnamvar);
				sprintf(vecfiles[i].vercatfile,"%s", tnamcat);
				sprintf(vecfiles[i].vertipo,"%s", tvartip);
				printf("%i %i %i %i %i %s %s %s\n", tpros, tcat, tlin, tdis, tord, tnamvar, tnamcat, tvartip);
				if (tpros == 1) hayvec++;
			}
			i++;
		}	
	}
	if (i == 0)
	{
		printf("Atencion error de lectura en variables\n");
		exit(0);
	}
	fclose(file);
	printf("Numero total de variables = %i\n", i);
	printf("end read file\n");
	if (tipo == 1)
	{
		n_varfile = i;
		printf("Numero total de variables a procesar = %i\n", hayvar);
		if (hayvar == 0)  
		{
			printf("Atencion: No se han activado variables vectoriales de base, revise configuracion\n");
			printf("Ponga el valor de la primera columna en 1 para las que quiera procesar\n");
			exit(0);
		}
	}
	if (tipo == 2)
	{
		n_vecfile = i;
		printf("Numero total de variables a procesar = %i\n", hayvec);
		if (hayvec == 0)  
		{
			printf("Atencion: No se han activado variables vectoriales de analisis, revise configuracion\n");
			printf("Ponga el valor de la primera columna en 1 para las que quiera procesar\n");
			exit(0);
		}
	}
	printf("---------------------------------\n\n");
	return 1;
}	

/*! LEYENDO CAPA VECTORIAL BASE Y RELACIONADA */	
int read_data(int tipo)
{
FILE *file;
int i, typ;
long int tid;
char texto[256];
double txcoor, tycoor;
	
	i =0;
	printf("\n***Lectura archivo vector-csv, %s tipo %i***\n", name_vect, tipo);
	if ((file = fopen(name_vect,"rt"))== NULL)
	{
		printf("-------ERROR open file--------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		exit(0);
	}
	else
	{
		fgets(texto,256,file); 
		while (fscanf(file,"%li %i %lf %lf" , 
					&tid,
					&typ,
					&txcoor,
					&tycoor         
					) == 4)
		{	
			//printf("%li %i %lf %lf\n", tid, typ, txcoor, tycoor);
			if (tipo == 1)
			{
				capabase[i].basid = tid;
				capabase[i].bastyp= typ;
				capabase[i].basx  = txcoor;
				capabase[i].basy  = tycoor;
			}
			if (tipo == 2)
			{
				caparela[i].relid  = tid;
				caparela[i].reltyp = typ;
				caparela[i].relx   = txcoor;
				caparela[i].rely   = tycoor;
			}
			i++;
		}	
	}
	if (tipo == 1)n_base = i; 
	if (tipo == 2)n_rela = i;
	fclose(file);
	printf("Numero total de elementos %i en %i\n", i, tipo);
	printf("end read file\n");
	if (i==0) 
	{
		printf("Error: No se pudieron leer datos de entrada, revise archivo\n");
		exit(0);
	}
	printf("---------------------------------\n\n");
	return 1;
}


//**********************************************************************
//AUXILIARES------------------------------------------------------------
//**********************************************************************

/*! CALCULA DIMENSION UNE 2 CADENAS */
int calc_size2(const char* str1, const char* str2)
{
int tot_size;
	tot_size = strlen(str1)+ strlen(str2)+1;
	return tot_size;	
}

/*! CALCULA DIMENSION UNE 3 CADENAS */
int calc_size3(const char* str1, const char* str2, const char* str3)
{
int tot_size;
	tot_size = strlen(str1)+ strlen(str2)+strlen(str3)+1;
	return tot_size;	
}

/*! CALCULA DIMENSION UNE 4 CADENAS */
int calc_size4(const char* str1, const char* str2, const char* str3, const char* str4)
{
int tot_size;
	tot_size = strlen(str1)+ strlen(str2)+strlen(str3)+strlen(str4)+1;
	return tot_size;	
}

/*! TIME */
int getime(void)
{
    time_t mytime = time(NULL);
    char * time_str = ctime(&mytime);
    time_str[strlen(time_str)-1] = '\0';
    printf("Current Time : %s\n", time_str);
    return 1;
}

/*! CREA DIRECTORIOS */
void crea_mkdir(char *path, mode_t mode) 
{
	if (mkdir(path, mode) == 0)
	{
	   printf("Directorio de categorias creado\n");
	}
	if (mkdir(path, mode) == -1)
	{
		printf("Atencion: Directorio de categorias no creado\n");
		if (mkdir(path, mode) && errno == EEXIST) printf("Porque el directorio Ya existe\n");
		if (mkdir(path, mode) && errno != EEXIST) 
		{
			printf("Porque la ruta esta equivocada %s, revise la configuracion\n", path);
			exit(0);
		}
	}
}

/*! CALCULA DISTANCIA */
double caldist(double x, double y, double x2, double y2)
{
double dist;
	dist = sqrt((x-x2)*(x-x2)+(y-y2)*(y-y2));
	return dist;
}

/*! LEE CAPA VECTORIAL-CSV BASE */
int selec_base(int num)
{
	//LEE VEC-BASE DESDE ESTRUCTURA INFILE
	baspro = varfiles[num].vaispro;                                     //si es procesada o no
	bascat = varfiles[num].vaiscat;                                     //si tiene categoria o no
	baslin = varfiles[num].vaislin;                                     //si es lineal o no
	basdis = varfiles[num].vaisdis;                                     //valor de discretizado
	basord = varfiles[num].vaorder;                                     //orden en el que aparece la capa
	sprintf(nambas,"%s", varfiles[num].varinifile);                     //nombre corto csv file
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	//CONSTRUYE NOMBRES
	txtsize = calc_size3(dirhom, dirbas, nambas);
	char bas_ini[txtsize];
	sprintf(bas_ini,"%s%s%s", dirhom, dirbas, nambas);
	if(txtsize < IN_SIZE) sprintf(name_vect,"%s", bas_ini);
	else 
	{
		printf("Se ha excedido name_vect en variable base en %i\n", txtsize);
		exit(0);
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	//LEE ARCHIVOS
	printf("Calucla vec-base %i -- orden %i cat %i lin %i dis %i\n\n", num, basord, bascat, baslin, basdis);
	read_data(1); 
	return 1;
}

/*! LEE CAPA VECTORIAL-CSV RELACIONAL */
int selec_rela(int num, int numuae)
{
	//LEE VEC-RELA DESDE ESTRUCTURA INFILE
	relpro = vecfiles[num].veispro;                                     //si es procesada o no
	relcat = vecfiles[num].veiscat;                                     //si tiene categoria o no
	rellin = vecfiles[num].veislin;                                     //si es lineal o no
	reldis = vecfiles[num].veisdis;                                     //valor de discretizado
	relord = vecfiles[num].veorder;                                     //orden en el que aparece la capa
	sprintf(namrel,"%s", vecfiles[num].verinifile);                     //nombre corto csv file
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	//CONSTRUYE NOMBRES
	txtsize = calc_size3(dirhom, dirbas, namrel);
	char rel_ini[txtsize];
	sprintf(rel_ini,"%s%s%s", dirhom, dirbas, namrel);
	if(txtsize < IN_SIZE) sprintf(name_vect,"%s", rel_ini);
	else 
	{
		printf("Se ha excedido name_vect en variable rela en %i\n", txtsize);
		exit(0);
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	//LEE ARCHIVOS
	printf("Calucla vec-rela %i -- orden %i cat %i lin %i dis %i\n\n", num, relord, relcat, rellin, reldis);
	read_data(2); 	
	return 1;
}

/*! CONSTRUYE NOMBRE ARCHIVO DE SALIDA VECTOR BASE*/
int getnamout(int numbas, int numrel)
{
char basname[20];
	printf("Generando nombre de archivo de salida v-base\n");
	if (tipmodo == 1)sprintf(basname, "VV_vb-sis_");
	if (tipmodo == 2)sprintf(basname, "VV_vb-usu_");
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//Captura nombres de entrada sin extension
	char *namebase = strtok(nambas, ".");
	char *namerela = strtok(namrel, ".");
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	txtsize = calc_size3(dir_out, basname, nambas);
	txtsizefin = txtsize+strlen("_Var-rel_xx");
	char basout[txtsizefin];
	sprintf(basout,"%s%s%s-%i_VAR_%s-%i.csv", dir_out, basname, namebase, basord, namerela, numrel);
	if(txtsizefin < OUT_SIZE) sprintf(out_vecbas,"%s", basout);
	else 
	{
		printf("Se ha excedido out_vecbas en %i\n", txtsizefin);
		exit(0);
	}
	return 1;
}

/*! CONSTRUYE NOMBRE ARCHIVO DE SALIDA VECTOR RELACIONADA*/
int getnamout_rela(int numrel, int numbas)
{
char basname[20];
	printf("Generando nombre de archivo de salida v-base\n");
	if (tipmodo == 1)sprintf(basname, "VV_sis-vr_");
	if (tipmodo == 2)sprintf(basname, "VV_usu-vr_");
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	char *namebase = strtok(nambas, ".");
	char *namerela = strtok(namrel, ".");
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	txtsize = calc_size3(dir_out, namerela, namebase);
	txtsizefin = txtsize+10;
	char vecrel_ini[txtsizefin];
	sprintf(vecrel_ini,"%s%s%s-%i_vb_%s-%i.csv", dir_out, basname, namerela, numrel, namebase, basord);
	if(txtsize < IN_SIZE) sprintf(out_vecrel,"%s", vecrel_ini);
	else 
	{
		printf("Se ha excedido out_vecrel en %i\n", txtsize);
		exit(0);
	}
	return 1;
}

/*! INICIALIZE STRUC CAPAS VECTOR*/
void inic_rela(void)
{
int i;	
	//INICIALIZA ESTRUCTURA PARA ALMACENAR DATOS CAPAS VECTORIALES
	for(i=0;i<n_rela;i++)  
	{ 
		caparela[i].relid     = 0;
		caparela[i].reltyp    = 0;
		caparela[i].relx      = 0.0;
		caparela[i].rely      = 0.0;
		caparela[i].relmin    = 0.0;    //distancia minima
		caparela[i].relbasid  = 0;      //el mas cerca
		caparela[i].relbassum = 0;      //todos los contenidos en el radio
	}
}

/*! INICIALIZE STRUC CAPAS VECTOR*/
void inic_base(int tipo)
{
int i;	
	//INICIALIZA ESTRUCTURA PARA ALMACENAR DATOS CAPAS VECTORIALES
	if (tipo == 1)
	{
		for(i=0;i<n_base;i++)  
		{ 
			capabase[i].basmin    = 0.0;    //distancia minima
			capabase[i].basrelid  = 0;      //el mas cerca
			capabase[i].basrelsum = 0;      //todos los contenidos en el radio
		} 
	}
	if (tipo == 2)
	{
		for(i=0;i<n_base;i++)  
		{ 
			capabase[i].basid     = 0;
			capabase[i].bastyp    = 0;
			capabase[i].basx      = 0.0;
			capabase[i].basy      = 0.0;
			capabase[i].basmin    = 0.0;    //distancia minima
			capabase[i].basrelid  = 0;      //el mas cerca
			capabase[i].basrelsum = 0;      //todos los contenidos en el radio
		} 
	}
}

//**********************************************************************
//CALCULA---------------------------------------------------------------
//**********************************************************************

/*! CALCULA PUNTO PROXIMO */
int calcprox(void)
{
int i, j, p;
int vberr, vbcon, vbsin, ok;
long int id, id2;
float distmin;
double tx, ty, tx2, ty2, discalc;
	vberr=0;
	vbcon=0;
	vbsin=0;
	p=50;
	if (distmax ==0)printf("Calculando punto mas proximo\n");
	if (distmax  >0)printf("Calculando puntos mas proximos por distancia mínima en metros %f\n", distmax);

	for(j=0;j<n_base;j++)                            
	{
		//datos vector base
		id   = capabase[j].basid;
		tx   = capabase[j].basx;
		ty   = capabase[j].basy;
		//si tiene una coordenada correcta
		if (tx != -9999)
		{
			if (distmax ==0)distmin = 100000;
			if (distmax  >0)distmin = distmax;
			ok=0;
			//para cada punto vector relacionado
			for(i=0;i<n_rela;i++)                            
			{
				//datos vector relaciona
				id2   = caparela[i].relid;
				tx2   = caparela[i].relx;
				ty2   = caparela[i].rely;
				//si tiene una coordenada correcta
				if (tx2 != -9999)
				{
					//calcula la distancia lineal entre dos puntos
					discalc = caldist(tx, ty, tx2, ty2);                
					//solo el mas cercano
					if (distmax ==0)                               
					{
						//si la distancia es menor que la minima existente, se almacena
						if (discalc < distmin)
						{
							//capturamos el mas cercano en la capa base
							capabase[j].basmin    = discalc;
							capabase[j].basrelid  = id2;
							capabase[j].basrelsum = 1;
							//escribimos tambien en la capa de relacionamiento
							caparela[i].relbasid = id;
							caparela[i].relmin   = discalc;
							//igualamos distancia minima
							distmin = discalc;
							ok=1;
						}
					}
					//todos los ubicados a una distancia dada
					if (distmax  >0)
					{
						//si la distancia es menor que la distancia maxima fijada
						if (discalc < distmax)
						{
							//Contamos el numero de puntos v-rela que hay cerca de punto v-base
							capabase[j].basrelsum +=1;
							//Contamos cuantas veces el punt v-rela esta cerca de un punto v-base
							caparela[i].relbassum += 1;
							//Capturamos el mas proximo
							if (discalc < distmin)
							{
								//capturamos el mas cercano en ambas capas
								capabase[j].basmin   = discalc;
								capabase[j].basrelid = id2;
								//escribimos tambien en la capa de relacionamiento
								caparela[i].relbasid = id;
								caparela[i].relmin   = discalc;
								//igualamos distancia minima
								distmin = discalc;
							}
							vbcon+=1;
							ok=1;
							if (conexit == 1)break;   //si encuentra 1 sale del bucle, solo para datos muy largos
						}
					}	
				}
			}
			//CONTEO RESULTADOS
			if (distmax ==0) 
			{
				if (ok == 0)vbsin+=1;
				if (ok == 1)vbcon+=1;
			}
			if (distmax  >0)
			{
				if (ok == 0)vbsin+=1;
			}
		}
		else vberr+=1;
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//imprime linea de control informativa durante el procesado
		if (p==j)
		{
			printf("%i -- Err %i Con %i Sin %i\n", j, vberr, vbcon, vbsin);
			p = p * 2;
		}
	}
	printf("Elementos v-base sin coordenadas %i\n", vberr);
	printf("Elementos v-base con valores proximos %i\n", vbcon);
	printf("Elementos v-base sin valores proximos %i\n", vbsin);
	return 1;
}	


//**********************************************************************
//ESCRIBE---------------------------------------------------------------
//**********************************************************************

/*! ESCRIBE CAPA VECTOR BASE */
int write_base(void)
{
FILE *file;
int i;

	printf("\nEscritura vector base %s\n", out_vecbas);
	if((file = fopen(out_vecbas,"wt"))== NULL)
	{
		printf("-------ERROR open file--------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		exit(0);
	}
	else
	{ 
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//CONSTRUYE CABECERA
		fprintf(file,"ID CODE X Y DISTMIN IDREL SUMREL\n");
		for(i=0;i<n_base;i++) 
		{
			//datos vec-base
			fprintf(file,"%li %i %lf %lf %f %li %i\n",
				capabase[i].basid,      //id punto
				capabase[i].bastyp,     //tipologia
				capabase[i].basx,       //coory
				capabase[i].basy,       //coorx
				capabase[i].basmin,     //distancia minima
				capabase[i].basrelid,   //el mas cerca
				capabase[i].basrelsum); //todos los contenidos en el radio
		}
	}	
	fclose(file);
	printf("Finaliza escritura archivo de salida\n");
	printf("---------------------------------\n\n");
	return 1;			
}

/*! ESCRIBE CAPA VECTOR RELACIONADA */
int write_rela(void)
{
FILE *file;
int i;

	printf("\nEscritura vector rela %s\n", out_vecrel);
	if((file = fopen(out_vecrel,"wt"))== NULL)
	{
		printf("-------ERROR open file--------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		exit(0);    }
	else
	{ 
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//CONSTRUYE CABECERA
		fprintf(file,"ID CODE X Y DISTMIN IDREL SUMREL\n");
		for(i=0;i<n_rela;i++) 
		{
			//datos uae lin
			fprintf(file,"%li %i %lf %lf %f %li %i\n",
				
				caparela[i].relid,      //id punto
				caparela[i].reltyp,     //tipologia
				caparela[i].relx,       //coory
				caparela[i].rely,       //coorx
				caparela[i].relmin,     //distancia minima
				caparela[i].relbasid,   //el mas cerca
				caparela[i].relbassum); //conteo de puntos cercanos
		}
	}	
	fclose(file);
	printf("Finaliza escritura archivo de salida\n");
	printf("---------------------------------\n\n");
	return 1;			
}

//**********************************************************************
//**********************************************************************
//MAIN------------------------------------------------------------------
//**********************************************************************
//**********************************************************************

int main(int argn, char **args)
{
int call;
int i, j;
int tcal, tcal2;	
 
	printf("Inicio del proceso de calculo\n");
	getime();
	sprintf(dirhom,"%s", getenv("HOME"));
	if (dirhom != NULL) 
	{
        printf("Homedir es %s\n", dirhom);
	}
	else
	{
		printf("Antencion, no se capturo el directorio home\n");
		exit(0);
	}
	//------
	if(args[1])sprintf(name_incfg,"%s",args[1]);                        /*!< Entra archivo config general como argumento */
	else
	{
		printf("ERROR: Por favor ingrese el archivo de configuracion\n");
		exit(0);
	}	
	call = read_cfg();                                                  //lectura archivo de configuracion
	if (call == 1)
	{
		//LEE ARCHIVOS INFILES
		sprintf(name_file, "%s", inbas);
		read_infiles(1);
		sprintf(name_file, "%s", inrel);
		read_infiles(2);
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//PARA CADA VECT BASE 
		for(j=0;j<n_varfile;j++)                                        // <----****----
		{
			tcal    = varfiles[j].vaispro;
			//SI ES PROCESABLE
			if (tcal == 1)
			{
				//LEE ARCHIVO VECT BASE
				selec_base(j);
				//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				//PARA CADA ARCHIVO VECTORIAL RELACIONAL
				for(i=0;i<n_vecfile;i++)                                // <----****----
				{
					tcal2 = vecfiles[i].veispro; 
					if (tcal2 == 1)                                     //si es procesada o no
					{
						//LEE VARIABLE >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
						selec_rela(i, j);
						//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
						//CALCULA DATOS PROXIMIDAD
						calcprox();
						//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
						//ESCRIBE Y REINICIA VEC-RELA
						getnamout_rela(relord, basord);	
						write_rela();
						inic_rela();
						//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
						//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
						//ESCRIBE VEC-BASE Y REINICIO PARCIAL
						getnamout(basord, relord);
						write_base();
						inic_base(1);
					}
				}
				//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
				//REINICIA VEC-BASE TOTAL
				inic_base(2);
			}
		}
	}
	printf("Finalizacion del proceso de calculo\n");
	getime();
}	
