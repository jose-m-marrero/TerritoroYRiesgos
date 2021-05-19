#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>

#define PI 3.141592653
//Si existe desbordamiento de memoria, hay que revisar el numero de elementos de entrada
#define NUMVPT  3000000  //NUM MAX DE ELEMENTOS VIAS-PUNTOS
#define NUMVLI   200000  //NUM MAX DE ELEMENTOS VIAS-LINEAS
#define NUMVAR  2000000  //NUM MAX DE ELEMENTOS VARIABLE-VECTORIAL
#define DISMIN        5  //DISTANCIA MINIMA EN METROS PARA PARAR LA BUSQUEDA PUNTO PROXIMO
#define CWD_MAX     256      //SIZE DIR TRABAJO
#define IN_SIZE     256      //SIZE NOMBRE IN TOTAL
#define IN_SIZE2    510      //SIZE NOMBRE IN TOTAL
#define OUT_SIZE    256      //SIZE NOMBRE OUT TOTAL

/**
* Copyright (C) 2019  Jose M. Marrero <josemarllin@gmail.com> and Hugo Yepes <hyepes@igepn.edu.ec>
* You may use, distribute and modify this code under the terms of the MIT License.
* The authors will not be held responsible for any damage or losses or for any implications 
* whatsoever resulting from downloading, copying, compiling, using or otherwise handling this 
* source code or the program compiled.
* Nombre script: Analiza_Vi-Vector.c
* Version: 1.0-2021-01-18
* Linea de compilacion: gcc Analiza_Vi-Vector.c -o Analiza_Vi-Vector -lm
* Linea de ejecucion:  ./Analiza_Vi-Vector configfilename.cfg
* Estrategia de analisis Viario-Vector
* Relaciona elementos espaciales puntuales por proximidad, centrados en el viario
* Determina puntos totales de variable vectorial en via
* Para cada relacion entre dos capas se escribe un archivo de salida de cada una
*/ 

/**
* GLOBAL VARIABLES
*/
char cwd[CWD_MAX], dirhom[30], dir_out[OUT_SIZE];                       //workig directories y ruta de salida
char diruae[180], dirvar[180], dirout[180];                             //directorios intermedios
char inviapt[50], inviali[50], invec[50];                               //nombres de archivos de entrada
char name_incfg[IN_SIZE], name_file[IN_SIZE];                           //rutas y nombres de archivos de entrada
char out_viapt[OUT_SIZE], out_vialin[OUT_SIZE], out_vec[OUT_SIZE];      //rutas y nombres de archivos de salida
int n_viapt, n_vialin, n_vec, escober;                                  //conteo de datos y parametros
int txtsize, txtsizefin;                                                //funciones auxiliares

/**
* ALMACENAMIENTO DE DATOS EN ESTRUCTURAS
*/

/*! ALMACENA DATOS CAPA VIA-PT */
typedef struct  
{
	int    vptid;      //id punto
	int    vptyp;      //id calle-linea
	double vptx;
	double vpty;
	int    vpidxl;      //indice de calle
	int    vptfil;      //filtro para descartar calles pequeñas
	int    vpnvar;      //numero de elementos de variable en punto
	int    vptram;      //todos los punto del tramo son validos si tienen valor algunos de ellos
}VPT;
VPT viapt[NUMVPT];

/*! ALMACENA DATOS CAPA VIA-LIN */
typedef struct  
{
	int vlid;      //Indice de calle
	int vltyp;     //ID calle
	int vlnum;     //numero puntos por linea
	int vlnvar;    //numero total de puntos-variables por calle
	int vltime;    //numero de puntos con valor en variable proxima
}VLI;
VLI vialin[NUMVLI];

/*! ALMACENA DATOS CAPA VEC-RELA */
typedef struct  
{
	int    varid;
	int    vartyp;
	double varlx;
	double varly;
	int    idxcal;
	int    idcall;
}VAR;
VAR variable[NUMVAR];

/**
* DECLARACION DE FUNCIONES
*/
int calc_size2(const char* str1, const char* str2);
int calc_size3(const char* str1, const char* str2, const char* str3);
int calc_size4(const char* str1, const char* str2, const char* str3, const char* str4);
void crea_mkdir(char *path, mode_t mode);


//**********************************************************************
//LECTURA---------------------------------------------------------------
//**********************************************************************

/*! LEYENDO ARCHIVO CONFIGURACION */
int read_cfg(void)
{
FILE *file;
char texto[80]; 
	//OPERACION EN MODO RASTER EN SISTEMA - VARIABLES EN SISTEMA
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
		fscanf(file,"%s %s", texto, diruae);
		fscanf(file,"%s %s", texto, dirvar);
		fscanf(file,"%s %s", texto, dirout);
		fscanf(file,"%s", texto);
		fscanf(file,"%s %s", texto, inviapt);
		fscanf(file,"%s %s", texto, inviali);
		fscanf(file,"%s %s", texto, invec);
		fscanf(file,"%s", texto);
		fscanf(file,"%s %i", texto, &escober);
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
	printf("DIR_IN UAE %s\n", diruae);
	printf("DIR_INVEC CSV %s\n", dirvar);
	printf("DIR_OUTGEN %s\n", dirout);
	printf("UAE_FILE_PT %s\n", inviapt);
	printf("UAE_FILE_LINE %s\n", inviali);
	printf("VEC_FILE %s\n", invec);
	printf("ES COBER %i\n", escober);
	return 1;		
}

/*! LEYENDO CAPA VECTORIAL VIAS-PT Y RELACIONADA */	
int read_data(int tipo)
{
FILE *file;
int i, tid, typ;
char texto[256];
double txcoor, tycoor;
	
	i =0;
	printf("\n***Lectura archivo datos-pt, %s***\n", name_file);
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
		while (fscanf(file,"%i %i %lf %lf" , 
					&tid,
					&typ,
					&txcoor,
					&tycoor         
					) == 4)
	
		{	
			//printf("%i %s %lf %lf\n", tid, txtid, txcoor, tycoor);
			if (tipo == 1)
			{
				viapt[i].vptid = tid;
				viapt[i].vptyp = typ;
				viapt[i].vptx  = txcoor;
				viapt[i].vpty  = tycoor;
			}
			if (tipo == 2)
			{
				variable[i].varid   = tid;
				variable[i].vartyp  = typ;
				variable[i].varlx   = txcoor;
				variable[i].varly   = tycoor;
				variable[i].idcall  = -9999;     
			}
			i++;
		}	
	}
	if (tipo == 1)n_viapt = i; //numero puntos via
	if (tipo == 2)n_vec   = i;   //numero puntos capa vectorial
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

/*! LEYENDO CAPA VECTORIAL VIAS-LINEAS */	
int read_vialin(void)
{
FILE *file;
int i, tid, typ, typ2, tnum;
char texto[256], txt[20];
	
	i =0;
	printf("\n***Lectura archivo ue-csv, %s***\n", name_file);
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
		//OBJECTID CAT_LOTE_I AREA_TERRE PROPIEDAD CLAVE_CATA Shape_Leng Shape_Area xcoord ycoord
		fgets(texto,256,file); 
		while (fscanf(file,"%i %i %i %i %s" , 
					&tid,
					&typ,
					&typ2,
					&tnum,
					txt         
					) == 5)
	
		{	
			vialin[i].vlid  = tid;
			vialin[i].vltyp = typ;
			vialin[i].vlnum = tnum;
			i++;
		}	
	}
	n_vialin = i;
	fclose(file);
	printf("Numero total de elementos %i\n", i);
	printf("end read file\n");
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

/*! CONSTRUYE NOMBRE ARCHIVOS DE ENTRADA */
int getnamin(char *indato)
{
	printf("Generando nombre de archivo de salida v-base\n");
	txtsize = calc_size3(dirhom, diruae, indato);
	char file_ini[txtsize];
	sprintf(file_ini,"%s%s%s", dirhom, diruae, indato);
	if(txtsize < IN_SIZE) sprintf(name_file,"%s", file_ini);
	else 
	{
		printf("Se ha excedido name_file en %i\n", txtsize);
		exit(0);
	}
	return 1;
}	

/*! CONSTRUYE NOMBRE ARCHIVOS DE SALIDA */
int getnamout(void)
{
char basname[20];
	printf("Generando nombre de archivo de salida v-base\n");
	sprintf(basname, "ViV_");
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//Captura nombres de entrada sin extension
	char *namevpt = strtok(inviapt, ".");
	char *namevli = strtok(inviali, ".");
	char *namevec = strtok(invec, ".");
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	txtsize = calc_size4(dirhom, dirout, namevpt, namevec);
	txtsizefin = txtsize+strlen(basname)+strlen("_Vec_");
	char ptout[txtsizefin];
	sprintf(ptout,"%s%s%s%s_Vec_%s.csv", dirhom, dirout, basname, namevpt, namevec);
	if(txtsizefin < OUT_SIZE) sprintf(out_viapt,"%s", ptout);
	else 
	{
		printf("Se ha excedido out_viapt pt en %i\n", txtsize);
		exit(0);
	}
	txtsize = calc_size4(dirhom, dirout, namevli, namevec);
	txtsizefin = txtsize+strlen(basname)+strlen("_Vec_");
	char liout[txtsizefin];
	sprintf(liout,"%s%s%s%s_Vec_%s.csv", dirhom, dirout, basname, namevli, namevec);
	if(txtsizefin < OUT_SIZE) sprintf(out_vialin,"%s", liout);
	else 
	{
		printf("Se ha excedido out_vialin line en %i\n", txtsize);
		exit(0);
	}
	txtsize = calc_size3(dirhom, dirout, namevec);
	txtsizefin = txtsize+strlen(basname)+strlen("_Via_");
	char veout[txtsizefin];
	sprintf(veout,"%s%s%s%s_Via_%s.csv", dirhom, dirout, basname, namevec, namevli);
	if(txtsizefin < OUT_SIZE) sprintf(out_vec,"%s", veout);
	else 
	{
		printf("Se ha excedido out_vec en %i\n", txtsize);
		exit(0);
	}
	return 1;
}

//**********************************************************************
//CALCULA---------------------------------------------------------------
//**********************************************************************

/*! DESCARTA TRAMOS DE CALLES < 10 M */
int filtsmall(void)
{
int i, j, p;
int typ, typ2, tnpt, noval, conval, hasta;
	noval=0;
	conval=0;
	p=2;
	printf("Descartando tramos de calles <10m\n");
	//PARA CADA VIA-LINEA
	for(i=0;i<n_vialin;i++)                            
	{
		//captura datos
		typ = vialin[i].vltyp;
		tnpt= vialin[i].vlnum;              //total puntos por linea
		hasta=0;
		//PARA CADA VIA-PUNTO
		for(j=0;j<n_viapt;j++)
		{
			//busca los puntos en su interior
			typ2 = viapt[j].vptyp;
			if (typ == typ2)                //si pertenecen al mismo tramo
			{
				viapt[j].vpidxl = i;        //captura id calle-linea y asigna al punto
				if (tnpt < 2)               //si la calle es small    
				{	viapt[j].vptfil = 1;    //elimina
					noval+=1;               //cuenta cuantos se eliminan
				}
				else conval+=1;             //cuantos se mantienen
				hasta +=1;
				if (hasta == tnpt)	break;  //si se llega al tope de puntos por calle sale segundo bucle
			}
		}
		if (p == i)
		{
			printf("En %i, sin %i con %i\n", i, noval, conval);
			p = p*2;
		}
	}
	printf("Total de tramos aceptados %i\n\n", conval);
	printf("Total de tramos descartados %i\n\n", noval);
	return 1;
}

/*! CALCULA PUNTO PROXIMO CALLE */
int calcprox(void)
{
int i, j, p;
int vaerr, vacon, vaok, geidfin;
int typ, distmin, tfil, tidx;
double tx, ty, tx2, ty2, discalc;
	vaerr=0;
	vacon=0;
	vaok =0;
	p=10;
	printf("Calculando punto mas proximo de calle para variable\n");
	//PARA CADA PUNTO CAPA VECTOR
	for(j=0;j<n_vec;j++)                            
	{
		//tomo datos capa vector
		tx   = variable[j].varlx;
		ty   = variable[j].varly;
		//si tiene una coordenada correcta
		if (tx != -9999)
		{
			distmin = 100;                              //el punto debe estar a menos de 100 m
			geidfin = 0;
			//PARA CADA VIA-PT
			for(i=0;i<n_viapt;i++)                            
			{
				//tid2 = viapt[i].vptid;
				typ  = viapt[i].vptyp;
				tidx = viapt[i].vpidxl;
				tx2  = viapt[i].vptx;
				ty2  = viapt[i].vpty;
				tfil = viapt[i].vptfil;
				//si tiene coordenada correta y es tramo grande
				if (tx2 != -9999 && tfil == 0)
				{
					//calcuo distancia
					discalc = caldist(tx, ty, tx2, ty2);
					if (discalc < distmin)
					{
						variable[j].idxcal = tidx;       //Asigno indice de la via donde se encuentra el punto
						variable[j].idcall = typ;        //asigno id via-linea proximo en punto
						geidfin = i;                     //capturo indice via
						distmin = discalc;               //actualizo distancia minima
						if (distmin <= 5)break;          //si se alcanzan menos de 5 metros salgo de la busqueda
					}
				}
			}
			//si encontre punto en la via
			if (geidfin > 0) 
			{
				viapt[geidfin].vpnvar += 1;              //incrementa elementos localizados en via-pt
				vaok+=1;
			}
			vacon +=1;
			
		}
		else vaerr+=1;
		//imprime linea de control informativa durante el procesado
		if (p == j)
		{
			printf("En %i, sin %i con %i prox %i\n", j, vaerr, vacon, vaok);
			p = p*2;
		}
	}
	printf("Vec-pt sin coordenadas %i\n", vaerr);
	printf("Vec-pt con coordenadas %i\n", vacon);
	printf("Vec-pt con proximidad %i\n\n", vaok);
	return 1;
}

/*! TRANSFIERE TODOS LOS VALORES DE VIA-PT A VIA-LINEA */
int from_ptolin(void)
{
int i, p;
int sin, con, tcon, tidx;
	//transfiere cuenta de elementos vectoriales en via-pt a via-linea y suma el total
	sin =0;
	con =0;
	p=2;
	printf("Transfiriendo total elementos vectoriales en via-punto a via-linea\n");
	//PARA CADA VIA-PT
	for(i=0;i<n_viapt;i++)                            
	{
		tcon = viapt[i].vpnvar;      //numero de puntos proximos
		tidx = viapt[i].vpidxl;	     //indice de via-linea
		vialin[tidx].vlnvar += tcon; //adiciona elementos vectoriales a via-lin
		
		if (tcon > 0)
		{
			//esta opcion solo tiene sentido para cobertura de elementos lineales
			vialin[tidx].vltime +=1; //cuenta el numero vias-punto que contienen valor de proximidad, por oposicion a los vacios
			con +=1;
		}
		else sin +=1;
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//imprime linea de control informativa durante el procesado
		if (p == i)
		{
			printf("En %i, sin %i con %i\n", i, sin, con);
			p = p*2;
		}
	}
	printf("vias-pt sin valor, no ocupadas %i\n", sin);
	printf("vias-pt con valor, ocupadas %i\n\n", con);
	return 1;
}

/*! DETERMINA NIVEL DE OCUPACION DE LA VIA-LIN */
int transcalle(void)
{
int i, j, p, ok;
int sin, con, tnpt, tcon, nvec, hasta, ini, datfin;
int typ, typ2;
	/*!
	* transfiere valores calle-linea a calle-punto, iguala todos donde hay actividad
	* se ignoran aquellas calles que solo tienen un punto con valor, por ser efectos de esquina.
	* solo se validan las que tienen mas de un punto de calle con valores asignados
	*/
	sin =0;
	con =0;
	p=2;
	printf("Igualando a 1 todos los puntos del tramo de via que estan cubiertos por servicio\n");
	//PARA CADA VIA-LINEA
	for(j=0;j<n_vialin;j++)
	{
		typ = vialin[j].vltyp;                  //id via-linea
		tnpt= vialin[j].vlnum;                  //total vias-puntos por via-linea
		tcon= vialin[j].vlnvar;                 //num elementos vectoriales por via-linea
		nvec= vialin[j].vltime;                 //num vias-pt con ocupacion pr via-linea
		if (escober == 0)                       //si no consideramos determinar cobertura
		{
			datfin = tcon;                      //captura total elementos vectoriales por via-linea
			ini=0;
		}
		if (escober == 1)                       //si consideramos determinar cobertura
		{
			datfin = nvec;                      //captura total puntos con proximidad por via-linea
			ini=1;
		}
		if (datfin > ini)                       //si tiene mas de un punto con valores
		{
			hasta=0;
			ok=0;
			//PARA CADA VIA-PT
			for(i=0;i<n_viapt;i++)                            
			{
				typ2 = viapt[i].vptyp;          //ID via-linea
				if (typ == typ2)                //si es la misma via
				{
					viapt[i].vptram = 1;        //todos los puntos del tramo se igualan a 1 para indicar que estan cubiertos
					ok+=1;
					hasta +=1;
					if (hasta == tnpt)	break;  //si se llega al tope de puntos por calle sale segundo bucle
				}
			}
			if (ok>0) con+=1;
		}
		else sin +=1;
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//imprime linea de control informativa durante el procesado
		if (p == j)
		{
			printf("En %i, sin %i con %i\n", j, sin, con);
			p = p*2;
		}
		
	}
	printf("vias-lin sin valor, no ocupadas %i\n", sin);
	printf("vias-lin con valor, ocupadas %i\n\n", con);
	return 1;
}

//**********************************************************************
//ESCRIBE---------------------------------------------------------------
//**********************************************************************

/*! ESCRIBE CALLES PUNTO CON TOTAL VARIABLE */
int write_viapt(void)
{
FILE *file;
int i;
	
	printf("\nEscritura via-pt %s\n", out_viapt);
	if((file = fopen(out_viapt,"wt"))== NULL)
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
		fprintf(file,"ID;CODE;X;Y;IDX;FILT;NVAR;ESVAL\n");
		for(i=0;i<n_viapt;i++) 
		{
			//datos via-pt
			fprintf(file,"%i;%i;%lf;%lf;%i;%i;%i;%i\n",
				viapt[i].vptid,
				viapt[i].vptyp,
				viapt[i].vptx,
				viapt[i].vpty,
				viapt[i].vpidxl,      //indice de calle
				viapt[i].vptfil,      //filtro para descartar calles pequeñas
				viapt[i].vpnvar,      //numero de elementos vectoriales en punto de via
				viapt[i].vptram       //todos los puntos del tramo son validos si tienen amplia cobertura
				);
		}
	}	
	fclose(file);
	printf("Finaliza escritura archivo de salida\n");
	printf("---------------------------------\n\n");
	return 1;			
}

/*! ESCRIBE CALLES LINEA CON TOTAL VARIABLE */
int write_vialin(void)
{
FILE *file;
int i;
	
	printf("\nEscritura via-linea %s\n", out_vialin);
	if((file = fopen(out_vialin,"wt"))== NULL)
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
		fprintf(file,"ID CODE NPT CONT NVEC\n");
		for(i=0;i<n_vialin;i++) 
		{
			//datos via-lin
			fprintf(file,"%i %i %i %i %i\n",
				vialin[i].vlid,
				vialin[i].vltyp,
				vialin[i].vlnum,     //numero de puntos en los que ha sido discretizada la calle
				vialin[i].vlnvar,    //numero de variables-punto totales asignadas a la calle
				vialin[i].vltime);   //numero de puntos-calle que contienen variable-punto, si la calle cuenta solo con uno se considera residual
		}
	}	
	fclose(file);
	printf("Finaliza escritura archivo de salida\n");
	printf("---------------------------------\n\n");
	return 1;			
}

/*! ESCRIBE CAPA VECTOR CONV VIA PROXIMA */
int write_var(void)
{
FILE *file;
int i;

	printf("\nEscritura capa vector relacionada %s\n", out_vec);
    if((file = fopen(out_vec,"wt"))== NULL)
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
		fprintf(file,"ID CODE X Y IDCALL\n");
		for(i=0;i<n_vec;i++) 
		{
			//datos vector rela
			fprintf(file,"%i %i %lf %lf %i\n",
				variable[i].varid,
				variable[i].vartyp,
				variable[i].varlx,
				variable[i].varly,
				variable[i].idcall);
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
		printf("%s", args[1]);
		printf("ERROR: Por favor ingrese el archivo de configuracion\n");
		exit(0);
	}	
	call = read_cfg();                                                  //lectura archivo de configuracion
	if (call == 1)
	{
		//READ FILES >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		getnamin(inviapt);                                              //genera nombre archivo de entrada
		read_data(1);
		getnamin(inviali);
		read_vialin();
		getnamin(invec);
		read_data(2);
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		filtsmall();                                                    //elimina tramos muy small de vias
 		calcprox();                                                     //calcula elemento proximo en via-pt
 		from_ptolin();                                                  //cuenta todos los valores via-pt y transfiere a via-lin
		transcalle();                                                   //transfiere numero de elementos a via-linea
		//--
		getnamout();
		write_viapt();
		write_vialin();
		write_var();
		printf("Finalizacion del proceso de calculo\n");
		getime();
	}
}
