#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define PI 3.141592653
//Si existe desbordamiento de memoria, hay que revisar el numero de elementos de entrada
#define UAECON       40      //NUM MAX DE UNIDADES ESPACIALES
#define VARCON      100      //NUM MAX VARIABLES CSV A ANALIZAR
#define NUMUAE   150000      //NUM MAX DE UNIDADES ESPACIALES
#define NUMVAR  4000000      //NUM MAX DE PUNTOS POR VARIABLE CSV
#define NUMCAT     1000      //NUM MAX DE CATEGORIAS POR VARIABLE CSV
#define NUMLOT   550000      //NUM MAX DE LOTES 426
#define NUMEDI   600000      //NUM MAX DE EDIFICIOS 499632
#define NUMCON  3000000      //NUM MAX DE CONSTRUCCIONES 2128713
//--
#define VALM        250      //VALOR DE METRO CUADRADO A CONSIDERAR 
#define VALME       450      //VALOR DE METRO CUADRADO A CONSIDERAR 
#define POBL      10000      //VALORES EXPRESADOS EN n HABITANTES 
#define POBL2     10000      //VALORES EXPRESADOS EN n HABITANTES 
#define POBL3      1000      //VALORES EXPRESADOS EN n HABITANTES 
#define POBL4      1000      //VALORES EXPRESADOS EN n HABITANTES 
#define LUZFA         5      //COBERTURA LUMINICA DE UNA FAROLA EN M
#define MOVIL      2500      //COBERTURA ANTENA TELEFONIA MOVIL EN M
#define CWD_MAX     256      //SIZE DIR TRABAJO
#define IN_SIZE     256      //SIZE NOMBRE IN TOTAL
#define OUT_SIZE    510      //SIZE NOMBRE OUT TOTAL

/**
* Copyright (C) 2019-2021  Jose M. Marrero <josemarllin@gmail.com> and Hugo Yepes <hyepes@igepn.edu.ec>
* You may use, distribute and modify this code under the terms of the MIT License.
* The authors will not be held responsible for any damage or losses or for any implications 
* whatsoever resulting from downloading, copying, compiling, using or otherwise handling this 
* source code or the program compiled.
* Nombre script: Analiza_RV_usu
* Version: 1.0-2021-05-12 
* Linea de compilacion: gcc Analiza_RV_user.c -o Analiza_RV_user -lm
* Linea de ejecucion:  ./Analiza_RV_user configfilename.cfg
* Genera indicadores a partir de la superposicion de capas vectoriales en UAE
* Cuenta elementos vectoriales en cada UAE
* Genera un unico archivo UAE-csv de salida con el analisis de todas la capas vectoriales
* 
* Modos disponibles:
* Modo 1: UAE sistema - Variable usuario
* Modo 2: UAE usuario - Variable usuario
* Modo 3: UAE Gen usu - Variable usuario
* 
* IMPORTANTE: en la definicion de unidades espaciales no es necesario utilizar autonumericos correlativos, 
* utilizamos el indice (bnum) o el valor (bnid) segun proceda, pero hay que estar atentos
* IMPORTANTE: la primera columna de categoria es el indice, la segunda es el valor numerico, 
* el que se transpasa a los datos vectoriales. Es con este segundo con el que se reaiza el join
*/ 


/**
* GLOBAL VARIABLES
*/
char dirhom[60], dir_cat[OUT_SIZE], dir_out[OUT_SIZE];                                        //workig directories
char diruaer[100], diruaec[100], dirvarv[100], dirvarc[100], dirout[100];                     //directorios intermedios
char inues[150], invar[150], tlotes[100], tedifi[100], tcons[100];                            //nombre archivos de entrada
char name_incfg[IN_SIZE], name_uaefile[IN_SIZE], name_varfile[IN_SIZE], name_catas[IN_SIZE];   //rutas y nombres de archivos de entrada
char name_uaegrd[IN_SIZE], name_uescsv[IN_SIZE], name_varcsv[IN_SIZE], name_varcat[IN_SIZE];  //rutas y nombres de archivos de entrada
char out_uaecsv[OUT_SIZE], out_uaeccat[OUT_SIZE], out_uaegen[OUT_SIZE];                       //rutas y nombres de archivos de salida
int tipmodo;                                                                                  //parametros generales
int n_uaefile, n_uaecsv, n_varfile, n_var, n_vcate, n_lote, n_edif, n_cons;                   //conteo de datos
char namcsv[100], namgrd[100];                                                                //parametros infiles
char namvari[150], namcate[150], tvartip[100];                                                //parametros infiles                                               
int hayuae, hayvar, tipval;                                                                   //parametros infiles
int concat, lineal, usokm, vorder, vgorder, csvpros, valcat, unfam, discre, orden;            //parametros infiles
int vcol, vrow;                                                                               //funciones auxiliares
double globx, globy;                                                                          //funciones auxiliares
int  txtsize, txtsizefin;                                                                     //funciones auxiliares
int **catgrid, **vargrid;                                                                     //matrices para almacenar datos totales y por categorias
int vdicre[VARCON][2000];                                                                     //almacena discretizado
                                                                  
/**
* GLOBAL VARIABLES - UAE GRD
*/
char header[8];
int nullval;
int totdemcel;
int nx, ny, nxc, nyc;
int demmax, demmin;
double xlo, xhi, ylo, yhi, zlo, zhi, resx, resy, invresx, invresy, resxx, resyy;
double xc, yc, dx, dy, ddx, ddy, dx2, dy2;
int **rast_uae;

/**
* ALMACENAMIENTO DE DATOS EN ESTRUCTURAS
*/

/*! ALMACENA DATOS INFILES CAPAS UAE */
typedef struct  
{
	int uecalc;
	int uenulval;
	int uemin;
	int uemax;
	int ueuskm;
	int ueorden;
	char uecsvfile[100];
	char uegrdfile[100];
	//--
	double uexcor;
	double ueycor;
	float  uerad;
	float  ueres;
}UES;
UES uefiles[UAECON];

/*! ALMACENA DATOS INFILES CAPAS VECTORIALES */
typedef struct  
{
	int vaispro;          //es calculada
	int vaiscat;          //tiene categoria
	int vaislin;          //es linea
	int vaisdis;          //discretizado
	int vaorder;          //orden de la variable
	char varinifile[150]; 
	char varcatfile[150];
	char vartipo[100];    //grupo al que pertenece
}VAR;
VAR varfiles[VARCON];

/*! ALMACENA DATOS UAE-CSV */
typedef struct  
{  
	int    bnum;       //Indice UAE, empieza en 0
	int    bnid;       //valor ID UAE
	char   btxtid[20]; //codigo UAE
	double bperim;
	double barea;
	double bxcoor;
	double bycoor;
	//--
}UAE;
UAE stuaecsv[NUMUAE];

/*! ALMACENA DATOS VECTOR-CSV */
typedef struct  
{
	long int datid;
	int datyp;
	double datx;
	double daty;
}DAT;
DAT stvarcsv[NUMVAR];

/*! ALMACENA DATOS VECTOR-CSV CATEGORIA */
typedef struct  
{
	int catid;        //valor indice categoria
	int catid2;       //valor numerico categoria
	char catshor[50];
	char catexp[150];
}CAT;
CAT categoria[NUMCAT];



/*! DECLARE FUNCTIONS */
int calc_size2(const char* str1, const char* str2);
int calc_size3(const char* str1, const char* str2, const char* str3);
int calc_size4(const char* str1, const char* str2, const char* str3, const char* str4);
double** Make2DDoubleArray(int arraySizeX, int arraySizeY);
int** Make2IntegerArray(int arraySizeX, int arraySizeY);
void calc_vecindex(double vx, double vy);
void get_coor(int col, int row);
int getime(void);
void crea_mkdir(char *path, mode_t mode);

int crea_uae(int num);
int selec_uae(int num);
int selec_var(int num, int numuae);
int getnamcat(int numuae);
int getnamout(int varnum);

int freeuaegrd(void);
int freevargrid(void);
int freecatgrid(void);
int freeuaecsv(void);
void inistruvar(void);
void inistvarcat(void);

int countdat(int uaeval, int varval, int catval);
void countuecat(int idxue, int idxcat);

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
        return 0;
    }
    else
    {	
		fscanf(file,"%s", texto);
		fscanf(file,"%s %i", texto, &tipmodo);
		fscanf(file,"%s %i", texto, &unfam);
		fscanf(file,"%s", texto);
		fscanf(file,"%s %s", texto, diruaer);
		fscanf(file,"%s %s", texto, diruaec);
		fscanf(file,"%s %s", texto, dirvarv);
		fscanf(file,"%s %s", texto, dirvarc);
		fscanf(file,"%s %s", texto, dirout);
		fscanf(file,"%s", texto);
		fscanf(file,"%s %s", texto, inues);
		fscanf(file,"%s %s", texto, invar);
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
	//CREA DIRECTORIO CATEGORIAS PRINCIPAL SI NO EXISTE
	char dircat[txtsizefin];
	sprintf(dircat,"%s%sCATEGORIAS/", dirhom, dirout);
	if(txtsizefin < OUT_SIZE) sprintf(dir_cat,"%s", dircat);
	else 
	{
		printf("Se ha excedido dir_cat en %i\n", txtsizefin);
		exit(0);
	}
	crea_mkdir(dir_cat, 0777);
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//IMPRIME DATOS LEIDOS
	printf("TIPO MODO %i\n", tipmodo);
	printf("UNIDAD FAMILIAR %i\n", unfam);
	printf("DIR_IN UAE GRD %s\n", diruaer);
	printf("DIR_IN UAE CSV %s\n", diruaec);
	printf("DIR_INVAR CSV %s\n", dirvarv);
	printf("DIR_INVAR CATE %s\n", dirvarc);
	printf("DIR_OUTGEN %s\n", dirout);
	printf("UAE_CONFIG_FILE %s\n", inues);
	printf("VAR_CONFIG_FILE %s\n", invar);
	return 1;		
}	

/*! LEYENDO CONFIG UAE INFILES */
int read_uaefile(void)
{
FILE *file;
int i;
int tnul, tmin, tmax, tuskm, tcal, torden;
char texto[256], tcsv[100],  tgrd[100];	
	//LEE INFILES CON LISTADO DE UAES
	i = 0;
	printf("\n***Lectura archivo UAEs, %s***\n", name_uaefile);
	if ((file = fopen(name_uaefile,"rt"))== NULL)
    {
        printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        exit(0);
        return 0;
    }
    else
    {
		fgets(texto,256,file); 
		printf("%s\n",texto); //imprime cabecera
		while (fscanf(file,"%i %i %i %i %i %i %s %s" , 
					&tcal,
					&tnul,
					&tmin,
					&tmax,
					&tuskm,
					&torden,
					tcsv,
					tgrd
					) == 8)
		{
			uefiles[i].uecalc    = tcal;
			uefiles[i].uenulval  = tnul;
			uefiles[i].uemin     = tmin;
			uefiles[i].uemax     = tmax;
			uefiles[i].ueuskm    = tuskm;
			uefiles[i].ueorden   = torden;
			sprintf(uefiles[i].uecsvfile,"%s", tcsv);
			sprintf(uefiles[i].uegrdfile,"%s", tgrd);
			printf("%i %i %i %i %i %s %s\n", tcal, tnul, tmin, tmax, tuskm, tcsv, tgrd); //imprime filas
			if (tcal == 1) hayuae +=1;                                  //cuenta UAE que se van a ser procesadas
			i++;
		}
			
	}
	if (i == 0)
	{
		printf("Atencion error de lectura en unidades espaciales\n");
		exit(0);
	}	
	n_uaefile = i; //numero de UAEs totales
	fclose(file);
	printf("Numero total de ues = %i\n", i);
	printf("Numero total de ues a procesar = %i\n", hayuae);
	printf("end read file\n");
	if (hayuae==0)
	{
		printf("Atencion: No se han activado UAEs, revise configuracion\n");
		printf("Ponga el valor de la primera columna en 1 para las que quiera procesar\n");
		exit(0);
	}
	printf("---------------------------------\n\n");
	return 1;
}

/*! LEYENDO CONFIG UAE GENERADOR */
int read_uaefilegen(void)
{
FILE *file;
int i;
int tcal, tuskm, torden;
char texto[256], tname[100];	
float trad, tres;
double txcoor, tycoor;
	//LEE ARCHIVO CON LISTADO DE UAES PARA SER GENERADAS A PARTIR DE PARAMETROS
	i = 0;
	printf("\n***Lectura archivo unidades espaciales-generador, %s***\n", name_uaefile);
	if ((file = fopen(name_uaefile,"rt"))== NULL)
    {
        printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        exit(0);
        return 0;
    }
    else
    {
		fgets(texto,256,file); 
		printf("%s\n",texto); 
		while (fscanf(file,"%i %i %i %lf %lf %f %f %s" , 
					&tcal,
					&tuskm,
					&torden,
					&txcoor,
					&tycoor,
					&trad,
					&tres,
					tname
					) == 8)
		{
			uefiles[i].uecalc  = tcal;
			uefiles[i].ueuskm  = tuskm;
			uefiles[i].ueorden = torden;
			uefiles[i].uexcor  = txcoor;
			uefiles[i].ueycor  = tycoor;
			uefiles[i].uerad   = trad;
			uefiles[i].ueres   = tres;
			sprintf(uefiles[i].uecsvfile, "%s", tname);
			printf("%i %i %i %lf %lf %f %f %s\n", tcal, tuskm, torden, txcoor, tycoor, trad, tres, tname);
			if (tcal == 1) hayuae +=1;
			i++;
		}
			
	}
	n_uaefile = i; //numero de unidades espaciales
	fclose(file);
	printf("Numero total de ues-gen = %i\n", n_uaefile);
	printf("Numero total de ues-gen a procesar = %i\n", hayuae);
	printf("end read file\n");
	if (hayuae==0)
	{
		printf("Atencion: No se han activado UAEs, revise configuracion\n");
		printf("Ponga el valor de la primera columna en 1 para las que quiera procesar\n");
		exit(0);
	}
	printf("---------------------------------\n\n");
	return 1;
}

/*! LEYENDO CONFIG VECTOR-CSV INFILES */
int read_varfile(void)
{
FILE *file;
int i;
int tcat, tlin, tdis, tord, tpros;
char texto[256], tnamvar[100],  tnamcat[100], tvartip[100];	
	i = 0;
	//LEE ARCHIVO INFILE CON LISTADO DE DATOS VECTORIALES
	printf("\n***Lectura archivo variables, %s***\n", name_varfile);
	if ((file = fopen(name_varfile,"rt"))== NULL)
    {
        printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        exit(0);
        return 0;
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
			i++;
			
		}	
	}
	if (i == 0)
	{
		printf("Atencion error de lectura en variables\n");
		exit(0);
	}
	n_varfile = i; //numero de unidades espaciales
	fclose(file);
	printf("Numero total de variables = %i\n", i);
	printf("Numero total de variables a procesar = %i\n", hayvar);
	printf("end read file\n");
	if (hayvar == 0)  
	{
		printf("Atencion: No se han activado variables vectoriales, revise configuracion\n");
		printf("Ponga el valor de la primera columna en 1 para las que quiera procesar\n");
		exit(0);
	}
	printf("---------------------------------\n\n");
	return 1;
}	

/*! LEYENDO DATOS UAE EN CSV FORMAT */	
int read_uecsv(void)
{
FILE *file;
int i, tid;
char texto[256], txtid[20];
double txcoor, tycoor, tarea, tperi;
	//LEE ARCHIVO UAE-CSV, SOLO MODOS 1 Y 2
	i       =0;
	n_uaecsv=0;
	printf("\n***Lectura archivo ue-csv, %s***\n", name_uescsv);
	if ((file = fopen(name_uescsv,"rt"))== NULL)
    {
        printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        exit(0);
        return 0;
    }
    else
    {
		fgets(texto,256,file); 
		while (fscanf(file,"%i %s %lf %lf %lf %lf" , 
					&tid,
					txtid,
					&tperi,
					&tarea,
					&txcoor,
					&tycoor         
					) == 6)
	
		{	
			//printf("%i %s %lf %lf\n", tid, txtid, txcoor, tycoor);
			stuaecsv[i].bnum   = i;                         //valor indice 
			stuaecsv[i].bnid   = tid;                       //valor en raster
			sprintf(stuaecsv[i].btxtid,"%s", txtid);
			stuaecsv[i].bperim = tperi;
			stuaecsv[i].barea  = tarea;
			stuaecsv[i].bxcoor = txcoor;
			stuaecsv[i].bycoor = tycoor;
			i++;
		}	
	}
	n_uaecsv = i; //numero de entidades dentro de una UAE
	fclose(file);
	printf("Numero total de elementos en UAE = %i id max %i\n", i, stuaecsv[i-1].bnid);
	printf("end read file\n");
	printf("---------------------------------\n\n");
	return 1;
}

/*! LEYENDO DATOS UAE EN BINARY RASTER FORMAT */
int read_uegrd(void)
{
FILE *in;
char cabecera[8];
short int datoin[4];
double datodo[8];
float datofl[4];
int i, j, contnul, contgod;
	//LEE RASTER UAE EN MODO 1 Y 2 (NECESITA UAE-CSV)
	printf("\n***Lectura archivo UAE-grd, %s***\n", name_uaegrd);
    if((in=fopen(name_uaegrd,"rb"))==NULL)
	{
		printf("-------ERROR open file--------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		exit(0);
		return 0;
	}
	/**
	* leyendo cabecera en golden software binary *.grd
	*/
	resx=0;
	resy=0;
	fread(&cabecera,4,1,in);
	fread(&datoin,2*sizeof(short int),1, in);
	fread(&datodo,6*sizeof(double),   1, in);
	cabecera[4] = 0;
	nx  = datoin[0];
	ny  = datoin[1];
	xlo = datodo[0];
	xhi = datodo[1]; 
	ylo = datodo[2];
	yhi = datodo[3];
	zlo = datodo[4];
	zhi = datodo[5];
	resx = (xhi-xlo)/(double)(nx-1);
	resy = (yhi-ylo)/(double)(ny-1);
	contnul=0;
	contgod=0;
	for(j=0;j<8;j++)printf("%lf\n",datodo[j]);
	/**
	* Crea matriz en memoria y asigna valores
	*/
	//rast_uae = Make2DDoubleArray (nx, ny);                          
	rast_uae = Make2IntegerArray(nx, ny);                               /*!< crea matriz en memoria */
	for(j=0;j<ny;j++) //row
		{
		for(i=0;i<nx;i++)
		{                                  
			fread(&datofl,sizeof(float),1,in);                          /*!< lee dato a dato por cada linea */
			//printf("%f %i %i\n", datofl[0], demmax, demmin);  
			if((datofl[0] > demmax) || (datofl[0] <= demmin))           /*!< si el valor esta fuera de limite, se asigna valor nulo */
			{
				datofl[0] = nullval;                                
				contnul+=1;
			}
			else  contgod +=1;
			rast_uae[i][j]  = (int)datofl[0]; 
			//rast_uae[i][j]  = (double)datofl[0]; 			
		}                                          
	}    
	fclose(in);
	/**
	* Calcula resoluciones y escalas
	*/
	invresx = 1/resx;
	invresy = 1/resy;
	resxx = resx*resx;
	resyy = resy*resy;
	totdemcel = nx * ny;
	/**
	* Imprime resultados
	*/    
	printf("Cabecera archivo RASTER\n");
	printf("RASTER tipo: %s\n", cabecera);
	printf("colx  = %5i -- rowy = %5i\n", nx, ny);
	printf("xmin = %f -- ymin = %f\n", xlo, ylo);
	printf("xmax = %f -- ymax = %f\n", xhi, yhi);
	printf("zlo = %10.4f -- zhi = %10.4f\n", zlo, zhi);
	printf("RASTER resolucion en x = %f\n", resx);
	printf("RASTER resolucion en y = %f\n", resy);
	printf("RASTER invresx = %f and invresy = %f\n", invresx, invresy);
	printf("RASTER resx2 = %f and resy2 = %f\n", resxx, resyy);
	printf("---------------------------------\n");
	printf("Total celdas en RASTER = %i\n", totdemcel);
	printf("Area de trabajo %lf\n", contgod * resx * resy);
	printf("Total celdas en nulas en RASTER = %i\n", contnul);
	printf("Area nula %lf\n", contnul * resx * resy);
	printf("Final de lectura de RASTER\n");  
	printf("---------------------------------\n\n");
	return 1;
}

/*! CREANDO UAE A PARTIR DE CENTROIDE Y RADIO */
int crea_uae(int num)
{
int i, j;
	//CREA UAE-RASTER A PARTIR DE PARAMENTROS, SOLO MODO 3
	printf("Genera raster a partir de punto\n");
	// Crea cabecera de raster
	xlo  = uefiles[num].uexcor - uefiles[num].uerad;           //coordenada x minima
	ylo  = uefiles[num].ueycor - uefiles[num].uerad;           //coordenada y minima
	xhi  = uefiles[num].uexcor + uefiles[num].uerad;           //coordenada x maxima
	yhi  = uefiles[num].ueycor + uefiles[num].uerad;           //coordenada y maxima
	resx = uefiles[num].ueres;                                 //resolucion en x
	resy = uefiles[num].ueres;                                 //resolucion en y
	nx   = (xlo + (2* uefiles[num].uerad) - xlo) / resx;       //numero de celdas en x
	ny   = (ylo + (2* uefiles[num].uerad) - ylo) / resy;       //numero de celdas en y
	// imprime valores de cabecera
	printf("colx  = %5i -- rowy = %5i\n", nx, ny);
	printf("Num total celdas %i\n", nx * ny);
	printf("xmin = %f -- ymin = %f\n", xlo, ylo);
	printf("xmax = %f -- ymax = %f\n", xhi, yhi);
	printf("DEM resolution in resx = %f\n", resx);
	printf("DEM resolution in resy = %f\n", resy);
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	if (nx > 22000)
	{
		printf("Atencion, raster ocupa demasiada memoria\n");
		printf("Por seguridad no deberia superar %i celdas\n", nx);
		printf("Reduzca la resolucion o el radio de busqueda y repita\n");
		exit(0);
	}
	//crea matriz de valores con el raster
	rast_uae = Make2IntegerArray(nx, ny);  //construye matriz
	for(j=0;j<ny;j++) //row
	{
		for(i=0;i<nx;i++)rast_uae[i][j] = 1; //asigna valores a cada celda, todos 1
	}
	return 1;
}

/*! LEYENDO DATOS VECTOR-CSV PUNTOS */
int read_var(void)
{
FILE *file;
int i, typ;
long int tid;
char texto[256];
double txcoor, tycoor;
	//LECTURA DE ARCHIVO DE VARIABLE VECTORIAL
	i     =0;
	n_var =0;
	printf("\n***Lectura archivo variable-csv, %s***\n", name_varcsv);
	if ((file = fopen(name_varcsv,"rt"))== NULL)
    {
        printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        exit(0);
        return 0;
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
			stvarcsv[i].datid  = tid;
			stvarcsv[i].datyp  = typ;
			stvarcsv[i].datx   = txcoor;
			stvarcsv[i].daty   = tycoor;
			i++;
		}	
	}
	n_var = i; //numero de elementos
	fclose(file);
	printf("Numero total de puntos = %i\n", i);
	printf("end read file\n");
	printf("---------------------------------\n\n");
	return 1;
}

/*! LEYENDO DATOS CATEGORIAS-CSV */
int read_varcate(void)
{
FILE *file;
int i, tid, tid2, difcat;
char texto[256], tshort[50], texpan[100];
	//LECTURA DE ARCHIVO DE DICCIONARIO DE CATEGORIAS, SI EXISTE
	i      =0;
	n_vcate=0;
	printf("\n***Lectura archivo categoria, %s***\n", name_varcat);
	if ((file = fopen(name_varcat,"rt"))== NULL)
    {
        printf("-------ERROR open file--------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        printf("-----------ERROR--------------\n");
        exit(0);
        return 0;
    }
    else
    {
		fgets(texto,256,file); 
		while (fscanf(file,"%i %i %s %s" , 
					&tid,
					&tid2,
					tshort,
					texpan         
					) == 4)
	
		{	
			//printf("%s - %s\n", tshort, texpan);
			categoria[i].catid  = tid;                          //indice categoria
			categoria[i].catid2 = tid2;                         //valor numerico categoria
			sprintf(categoria[i].catshor,"%s", tshort);         //nombre categoria, deben eliminarse espacios en blanco
			sprintf(categoria[i].catexp,"%s", texpan);          //si existe, segundo nombre categoria, deben eliminarse espacios en blanco
			i++;
		}	
	}
	//DEFINE EL TIPO DE DICCIONARIO DE CATEGORIA
	tipval=0;
	difcat = tid2 - tid;
	if (difcat == 0)tipval = 0;
	if (difcat == 1)tipval = 1; //numeracion correlativa entre indice y catval
	if (difcat  > 1)tipval = 2; //el catval es numerico en origen y puede ser cuaquiera
	printf("%i  %i  %i :: %i\n", difcat, tid2, tid, tipval);
	n_vcate = i; //numero de categorias
	fclose(file);
	printf("Numero total de categorias = %i\n", i);
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

/*! GENERADOR DE MATRIZ DINAMICA EN DECIMALES */
double** Make2DDoubleArray(int arraySizeX, int arraySizeY) 
{
int i;	
double** theArray;
	theArray = (double**) malloc(arraySizeX*sizeof(double*));
	for(i=0; i<arraySizeX;i++)
	{
		theArray[i] = (double*) malloc(arraySizeY*sizeof(double));
		if(theArray[i]== NULL)printf("error 2\n");
	}	
	return theArray;
} 

/*! GENERADOR DE MATRIZ DINAMICA EN ENTEROS */
int** Make2IntegerArray(int arraySizeX, int arraySizeY) 
{
int i;	
int** theArray;
	theArray = (int**) malloc(arraySizeX*sizeof(int*));
	for(i=0; i<arraySizeX;i++)
	{
		theArray[i] = (int*) malloc(arraySizeY*sizeof(int));
		if(theArray[i]== NULL)printf("error 2\n");
	}	
	return theArray;
} 

/*! CALCULA INDICES DE FILA Y COLUMNA */
void calc_vecindex(double vx, double vy)
{
double difx, dify, demx, demy;

	//Indice de fila
	difx = vx - xlo; 
	vcol = (int)(difx / resx);
	demx = (vcol * resx) + xlo; 
	if ((vx - demx) > (resx/2))vcol++; 
	//Indice de columna
	dify = vy - ylo;
	vrow = (int)(dify / resy);
	demy = (vrow * resy) + ylo;
	if ( (vy - demy) > (resy/2))vrow++;
}

/*! CALCULA X E Y A PARTIR DE LOS INDICES DEL RASTER */
void get_coor(int col, int row)
{
	globx = 0;
	globy = 0;
	globx = xlo + (col * resx);
	globy = ylo + (row * resy);
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

/*! LEE DATOS UAE GRD Y CSV */
int selec_uae(int num)
{
int txtsize;
	if (tipmodo<=2)
	{
		//CAPTURA VALORES DE UAE DESDE INFILE
		nullval = uefiles[num].uenulval;                      //valor nulo a utilizar
		demmin  = uefiles[num].uemin;                         //valor grd minimo
		demmax  = uefiles[num].uemax;                         //valor grd maximo
		usokm   = uefiles[num].ueuskm;                        //segun area ue se convierte a km o se mantiene en m
		orden   = uefiles[num].ueorden;                       //orden en el que aparece la UAE
		sprintf(namcsv,"%s", uefiles[num].uecsvfile);         //nombre corto csv file
		sprintf(namgrd,"%s", uefiles[num].uegrdfile);         //nombre corto grd file
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//CONSTRUYE RUTA Y NOMBRES DE ENTRADA
		txtsize = calc_size3(dirhom, diruaec, namcsv);
		char uescsv_ini[txtsize];
		sprintf(uescsv_ini,"%s%s%s", dirhom, diruaec, namcsv);
		if(txtsize < IN_SIZE) sprintf(name_uescsv,"%s", uescsv_ini);
		else 
		{
			printf("Se ha excedido name_uescsv en %i\n", txtsize);
			exit(0);
		}
		txtsize = calc_size3(dirhom, diruaer, namgrd);
		char uaegrd_ini[txtsize];
		sprintf(uaegrd_ini,"%s%s%s", dirhom, diruaer, namgrd);
		if(txtsize < IN_SIZE) sprintf(name_uaegrd,"%s", uaegrd_ini);
		else 
		{
			printf("Se ha excedido name_uaegrd en %i\n", txtsize);
			exit(0);
		}
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//LEE ARCHIVOS
		printf("Calucla UE %i -- orden %i nul %i min %i max %i\n\n", num, orden, nullval, demmin, demmax);
		read_uegrd();                                       //read unidad espacial grd
		read_uecsv();                                       //read unidad espacial csv  
	} 
	//GENERA VALORES DE UAE DESDE INFILE - GEN  
	if (tipmodo==3)  
	{   
		//CONSTRUYE UAE CON FORMA DE CUADRADO
		sprintf(namcsv,"%s", uefiles[num].uecsvfile);
		nullval=-9999;
		stuaecsv[num].bnum = 0;                                      //numero inicio en cero 
		stuaecsv[num].bnid = 1;                                      //UAE id 
		sprintf(stuaecsv[num].btxtid, "%s", uefiles[num].uecsvfile); //codigo 
		stuaecsv[num].bperim = (uefiles[num].uerad * 2) + (uefiles[num].uerad * 2) + (uefiles[num].uerad * 2) + (uefiles[num].uerad * 2);
		stuaecsv[num].barea  = (uefiles[num].uerad * 2) * (uefiles[num].uerad * 2);
		stuaecsv[num].bxcoor = uefiles[num].uexcor;
		stuaecsv[num].bycoor = uefiles[num].ueycor;
		n_uaecsv=1;
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		crea_uae(num);
	}
	//GENERA NOMBRES DE ARCHIVO DE SALIDA
	getnamout(num);	
	return 1;
}

/*! CONSTRUYE NOMBRE ARCHIVO DE SALIDA PARA UAE */
int getnamout(int numuae)
{
char uaename[20];

	if (tipmodo == 1)sprintf(uaename, "RV_r-sis_v-usu_");
	if (tipmodo == 2)sprintf(uaename, "RV_r-usu_v-usu_");
	if (tipmodo == 3)sprintf(uaename, "RV_r-usu-gen_v-usu_");
	
	//NOMBRA UAE DE SALIDA A PARTIR UAE-CSV
	txtsize = calc_size4(dirhom, dirout, uaename, namcsv);
	txtsizefin = txtsize+strlen("_");
	char csvrout[txtsizefin];
	if (tipmodo <= 2)sprintf(csvrout,"%s%s%s%i_%s", dirhom, dirout, uaename, orden, namcsv);
	if (tipmodo == 3)sprintf(csvrout,"%s%s%s%i_%s.csv", dirhom, dirout, uaename, orden, namcsv);
	if(txtsizefin < OUT_SIZE) sprintf(out_uaecsv,"%s", csvrout);
	else 
	{
		printf("Se ha excedido out_uaecsv en %i\n", txtsizefin);
		exit(0);
	}
	//NOMBRA UAE DE SALIDA GRD POR RADIO Y CENTROIDE
	if (tipmodo == 3)
	{
		txtsize = calc_size4(dirhom, dirout, uaename, namcsv);
		txtsizefin = txtsize+10;
		char grdout[txtsizefin];
		sprintf(grdout,"%s%s%s%i_%s.grd", dirhom, dirout, uaename, orden, namcsv);
		if(txtsizefin < OUT_SIZE) sprintf(out_uaegen,"%s", grdout);
		else 
		{
			printf("Se ha excedido out_uaegen en %i\n", txtsizefin);
			exit(0);
		}
		
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	//CREA DIRECTORIO CATEGORIAS ESPECIFICO SI NO EXISTE PARA CADA UAE PROCESADA
	txtsize = calc_size2(dirhom, dirout);
	txtsizefin = txtsize+strlen("CATEGORIAS/UAE-XX");
	char dircat[txtsizefin];
	sprintf(dircat,"%s%sCATEGORIAS/UAE-%i", dirhom, dirout, orden);
	if(txtsizefin < OUT_SIZE) sprintf(dir_cat,"%s", dircat);
	else 
	{
		printf("Se ha excedido dir_cat en %i\n", txtsizefin);
		exit(0);
	}
	crea_mkdir(dir_cat, 0777);
	return 1;
}

/*! LEE DATOS VECTOR-CSV */
int selec_var(int num, int numuae)
{
	//CONTENIDO ARCHIVO INFILE...
	concat = varfiles[num].vaiscat;                     //tiene categoria a evaluar
	lineal = varfiles[num].vaislin;                     //es lineal
	discre = varfiles[num].vaisdis;                     //discretizado utilizado
	vorder = varfiles[num].vaorder;                     //orden de lectura y asignacion de variable
	sprintf(namvari,"%s", varfiles[num].varinifile);    //mombre archivo variable
	sprintf(namcate,"%s", varfiles[num].varcatfile);    //nombre archivo categoria
	sprintf(tvartip,"%s", varfiles[num].vartipo);       //nombre corto variable
	vdicre[num][vorder] = discre;                       //almacena discretizado
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//ASIGNA NOMBRE ARCHIVO VARIABLE
	printf("Calucla UE %i-%i en VAR-user %i %s\n\n", numuae, orden, vorder, tvartip);
	txtsize = calc_size3(dirhom, dirvarv, namvari);
	char varcsv_ini[txtsize];
	sprintf(varcsv_ini,"%s%s%s", dirhom, dirvarv, namvari);
	if(txtsize < IN_SIZE) sprintf(name_varcsv,"%s", varcsv_ini);
	else 
	{
		printf("Se ha excedido name_varcsv en %i\n", txtsize);
		exit(0);
	}
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//LEE CAPA VECTORIAL
	read_var();
	//SI TIENE CATEGORIAS, LEE DICCIONARIO
	if (concat>0)
	{ 
		getnamcat(numuae);
	}
	return 1;
}

/*! CONSTRUYE NOMBRE ARCHIVO DE SALIDA PARA UAE POR CATEGORIA */
int getnamcat(int numuae)
{
int k, l;
char tipocat[15];
char uaename[20];

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//CAPTURA NOMBRE DE ENTRADA DE CATEGORIA
	txtsize = calc_size3(dirhom, dirvarc, namcate);
	char varcat_ini[txtsize];
	sprintf(varcat_ini,"%s%s%s", dirhom, dirvarc, namcate);
	if(txtsize < IN_SIZE) sprintf(name_varcat,"%s", varcat_ini);
	else 
	{
		printf("Se ha excedido name_varcsv en %i\n", txtsize);
		exit(0);
	}
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//LEE ARCHIVO CATEGORIA
	read_varcate();
	//GENERA ARRAY DINAMICA PARA ALMACENAR VALORES
	printf("n_vcate %i col, n_uaecsv %i rows\n", n_vcate, n_uaecsv);
	catgrid = Make2IntegerArray(n_vcate, n_uaecsv);
	for(k=0;k<n_uaecsv;k++) //rows
	{
		for(l=0;l<n_vcate;l++)catgrid[l][k]=0;            //asigna valores ceros iniciales
	}	 
	int glo = sizeof(catgrid);
	printf("global of array: %d\n", glo);
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//DEFINE NOMBRE DE SALIDA
	if (tipmodo == 1)sprintf(uaename, "RV_r-sis_v-usu_");
	if (tipmodo == 2)sprintf(uaename, "RV_r-usu_v-usu_");
	if (tipmodo == 3)sprintf(uaename, "RV_r-usu-gen_v-usu_");
	
	txtsize = calc_size4(dirhom, dirout, tipocat, tvartip);
	txtsizefin = txtsize+10+strlen("CATEGORIAS/UAE-CAT--XX");
	char uaeccat_ini[txtsizefin];
	sprintf(uaeccat_ini,"%s%sCATEGORIAS/UAE-%i/%sCAT-%s-%i.csv", dirhom, dirout, orden, uaename, tvartip, vorder);
	if(txtsizefin < OUT_SIZE) sprintf(out_uaeccat,"%s", uaeccat_ini);
	else 
	{
		printf("Se ha excedido out_uaeccat en %i\n", txtsizefin);
		exit(0);
	}
	return 1;
}

/*! CALCULA INDICES DE UAE Y CATEGORIA Y SUMA VALORES */
int countdat(int uaeval, int varval, int catval)
{
int j, k, tid, uaeidx, idxcat, ok;
	//CUENTA ELEMENTOS
	ok=0;
	//CACULA EL INDICE DE LA UAE
	if (tipmodo<=2)
	{
		for (j=0;j<n_uaecsv;j++)                 //buscamos indice uae en csv, posicion que ocupa empenzando en 0
		{
			tid = stuaecsv[j].bnid;
			if (tid == uaeval) 
			{
				uaeidx = stuaecsv[j].bnum;      //captura indice de uae en csv
				ok=1;
				break;
			}
		}
	}
	if (tipmodo==3) 
	{
		uaeidx = 0;
		ok=1;
	}
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//INCREMENTAMOS CONTEO EN INDICADOR
	//printf("adiciona en %i %i %i %i\n", uaeval, uaeidx, varval, catval);
	if (ok == 1)vargrid[varval][uaeidx] +=1;
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//SI TIENE CATEGORIA
	if (concat>0)                                  
	{
		//ASIGNA INDICE CATEGORIA
		idxcat = -1;
		for (k=0;k<n_vcate;k++)                             //buscamos indice categoria en csv-cat, posicion que ocupa empenzando en 0
		{
			valcat = categoria[k].catid2;
			if (valcat == catval) 
			{
				idxcat = categoria[k].catid;
				break;
			}
		}
		if (idxcat > n_vcate || idxcat == -1)
		{
			//printf("%i -- %i - %i  :: %i - %i\n", vorder, idxcat, n_vcate, uaeval, n_uaecsv);
			printf("ATENCION, valores fuera de lo permitido, revise la categoria y el raster\n");
			if (idxcat  > -1)printf("Tiene que adicionar campos, pasar de %i a %i\n", n_vcate, idxcat); 
			if (idxcat == -1)printf("No se econtro la categoria %i en el archivo de categorias\n", catval); 
			exit(0);
		}
		//EVALUAMOS CATEGORIA
		if (idxcat != -1)countuecat(uaeidx, idxcat);
	}
	return ok;
}

/*! ALMACENA DATOS DE CATEGORIA EN MATRIZ */
void countuecat(int idxue, int idxcat)
{
	catgrid[idxcat][idxue] +=1;    //SUMA DE VALORES POR CATEGORIA
}	

/*! LIBERA MEMORIA DE UAE-RASTER */
int freeuaegrd(void)
{
int k;
	printf("Liberando UAE-raster\n");
	for(k=0;k<nx;k++)free(rast_uae[k]);
	free(rast_uae);
	rast_uae = NULL;
	return 1;
}

/*! LIBERA MEMORIA DE UAE-CSV */
int freevargrid(void)
{
int k;
	printf("Liberando vargrid raster\n");
	for(k=0;k<hayvar;k++)free(vargrid[k]);
	free(vargrid);
	catgrid = NULL;
	return 1;						
}

/*! LIBERA MEMORIA DE UAE VARIABLE-CAT */
int freecatgrid(void)
{
int k;
	printf("Liberando catgrid raster\n");
	for(k=0;k<n_vcate;k++)free(catgrid[k]);
	free(catgrid);
	catgrid = NULL;
	return 1;						
}

/*! INICIALIZA ESTRUCTURA DE DATOS UAE */
void inistrucuae(void)
{
int i;	
	//INICIALIZA ESTRUCTURA UAE CSV
	for(i=0;i<n_uaecsv;i++)  
	{ 
		stuaecsv[i].bnum   = 0;
		stuaecsv[i].bnid   = 0;       
		sprintf(stuaecsv[i].btxtid,"0"); 
		stuaecsv[i].bperim = 0;
		stuaecsv[i].barea  = 0;
		stuaecsv[i].bxcoor = 0.0;
		stuaecsv[i].bycoor = 0.0;
	}
}

/*! INICIALIZA ESTRUCTURA VECTORIAL */
void inistruvar(void)
{
int i;	
	//INICIALIZA ESTRUCTURA PARA ALMACENAR VARIABLE VECTORIAL
	for(i=0;i<n_var;i++)  
	{ 
		stvarcsv[i].datid = 0;
		stvarcsv[i].datyp = 0;
		stvarcsv[i].datx  = 0.0;
		stvarcsv[i].daty  = 0.0;
	}
}		

/*! INICIALIZA ESTRUCTURA VECTORIAL-CAT */
void inistvarcat(void)
{
int i;	
	//INICIALIZA ESTRUCTURA PARA ALMACENAR CAT DE VARIABLE VECTORIAL
	for(i=0;i<NUMCAT;i++)  
	{ 
		categoria[i].catid  = 0;
		categoria[i].catid2 = 0;
		sprintf(categoria[i].catshor, "0");
		sprintf(categoria[i].catexp, "0");
	}
}		


//**********************************************************************
//CALCULA---------------------------------------------------------------
//**********************************************************************

/*! CALCULA DATOS VECTORIALES-CSV EN UAE RASTER */
int calc_dat(int ue, int var)
{
int i, uaeval, out, nul, in, fin; 
double txcoor, tycoor;
	printf("Calcula UAE %i-%i para variable %i\n", ue, orden, vorder);
	out=0;
	in =0;
	nul=0;
	//PARA CADA ELEMENTO DE LA VARIABLE
	for (i=0;i<n_var;i++)
	{
		txcoor = stvarcsv[i].datx;
		tycoor = stvarcsv[i].daty;
		valcat = stvarcsv[i].datyp;      //valor categoria, si existe
		fin = 0;
		//SI ESTA DENTRO DEL RASTER
		if((txcoor > xlo && txcoor < xhi) && (tycoor > ylo && tycoor < yhi)) //si esta en el raster
		{
			//OBTENEMOS FILA Y COLUMNA DEL RASTER
			calc_vecindex(txcoor, tycoor);
			//SI LOS VALORES ESTAN EN UAE RASTER
			if (vcol < nx && vrow < ny)
			{	
				//OBTENERMOS VALOR DE LA UAE DONDE CONCIDE LA VARIABLE VECTORIAL
				uaeval = rast_uae[vcol][vrow];
				if (uaeval != nullval)                                  //si no es nulo
				{
					//CUENTA VALORES EN UAE-CSV Y CATEGORIAS SI EXISTEN
					fin = countdat(uaeval, var, valcat);
					if (fin == 1)in+=1;                                 //cuenta valores dentro de UAE
				}
				else nul+=1;                                            //cuenta casos nulos
			}
			else out+=1;	                                            //cuenta casos fuera de raster		
		}
		else out+=1;                                                    //cuenta casos fuera de raster		
	}				
	printf("Finalizado Cruce en %i-%i\n", ue, orden);
	printf("Elementos dentro UAE: %i\n", in);	
	printf("Elementos fuera  UAE: %i\n", nul);	
	printf("Elementos fuera raster: %i\n", out);	
	printf("***********************\n\n");
	return in;
}	
	
	
//**********************************************************************
//ESCRIBE---------------------------------------------------------------
//**********************************************************************

/*! ESCRIBE DATOS UAE-CSV CON INDICADORES */
int write_uaecsv(void)
{
FILE *file;
int i, j;
int okfin, val_ord[n_uaefile];
	printf("\Escribe uae-csv global %s\n", out_uaecsv);
	if((file = fopen(out_uaecsv,"wt"))== NULL)
	{
		printf("ERROR %s\n",out_uaecsv);
		printf("ATENCION: LA RUTA DE SALIDA DEL ARCHIVO NO ES CORRECTA\n");
		exit(0);
	}
	else
	{ 
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//CONSTRUYE CABECERA 
		fprintf(file,"%s", "NUM ID CODE ARE PER X Y "); //primera linea
		okfin=0;
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//CALCULA NUMERO DE RASTER-ORDEN
		j=0;
		for(i=0;i<n_varfile;i++)
		{
			if (varfiles[i].vaispro  == 1)
			{
				val_ord[j] = varfiles[i].vaorder;
				j++;
			}
		}
		//COMPLETA CABECERA CON COLUMNAS INDICADORES
		for(i=0;i<hayvar;i++)
		{
			if (i  < hayvar -1) fprintf(file,"VAR-%i ", val_ord[i]); 
			if (i == hayvar -1) 
			{
				fprintf(file,"VAR-%i\n",val_ord[i]);
				okfin=1;
			}
		}
		if (okfin==0)fprintf(file,"\n");
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//ESCRIBE DATOS
        for(i=0;i<n_uaecsv;i++)
        {		
			fprintf(file,"%i %i %s %.2lf %.2lf %lf %lf ",
				
				stuaecsv[i].bnum,
				stuaecsv[i].bnid,
				stuaecsv[i].btxtid, 
				stuaecsv[i].bperim,
				stuaecsv[i].barea,
				stuaecsv[i].bxcoor, 
				stuaecsv[i].bycoor
				);

			for(j=0;j<hayvar;j++)
			{
				if (j  < hayvar -1) fprintf(file,"%i ", vargrid[j][i]); 
				if (j == hayvar -1) fprintf(file,"%i\n",vargrid[j][i]);
			}
		}
	}	
	fclose(file);
	printf("Finaliza escritura archivo de salida\n");
	printf("---------------------------------\n\n");
	return 1;			
}

/*! ESCRIBE DATOS UAE-CSV POR CATEGORIA */
int write_catfile(void)
{	
FILE *file;
int i,j;
float valin;

	printf("Escribe uae-csv especifico por categoria %s\n", out_uaeccat);
	if((file = fopen(out_uaeccat,"wt"))== NULL)
	{
		printf("ERROR %s\n",out_uaeccat);
		printf("ATENCION: LA RUTA DE SALIDA DEL ARCHIVO NO ES CORRECTA\n");
		exit(0);
	}
	else
	{        
		if (lineal==0) printf("Archivo espe no lineal\n");
		if (lineal==1) printf("Archivo espe lineal\n");
		
		if (usokm  > 1)printf("Archivo uae small\n");
		if (usokm == 1)printf("Archivo uae grande\n");
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//ESCRIBE CABECERA
		fprintf(file,"NUM ID CODE ARE PER X Y ");
		for(i=0;i<n_vcate;i++)
		{
			if (concat==1)  //si los nombres cortos son manejabes
			{
				if (i  < n_vcate -1) fprintf(file,"%s ", categoria[i].catshor); 
				if (i == n_vcate -1) fprintf(file,"%s\n", categoria[i].catshor);
			}
			if (concat==2)  //caso contrario se usan numeros
			{
				if (i  < n_vcate -1) fprintf(file,"CAT-%i ", categoria[i].catid2); 
				if (i == n_vcate -1) fprintf(file,"CAT-%i\n", categoria[i].catid2);
			}
		}
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//ESCRIBE DATOS
		for(j=0;j<n_uaecsv;j++) //col
		{
			//DATOS INICIALES DE UAE
			fprintf(file,"%i %i %s %.2lf %.2lf %lf %lf ",
				stuaecsv[j].bnum,
				stuaecsv[j].bnid, 
				stuaecsv[j].btxtid,
				stuaecsv[j].bperim,
				stuaecsv[j].barea,
				stuaecsv[j].bxcoor, 
				stuaecsv[j].bycoor);
			//INDICADORES
			for(i=0;i<n_vcate;i++)
			{  
				//si no son lineales
				if (lineal==0)
				{
					if (i  < n_vcate -1) fprintf(file,"%i ", catgrid[i][j]); 
					if (i == n_vcate -1) fprintf(file,"%i\n", catgrid[i][j]);
				}	
				//si son lineales
				if (lineal==1) 
				{
					//si hay que pasarlos a km o no
					if (usokm >  1)valin = catgrid[i][j] * discre;          //en m
					if (usokm == 1)valin = (catgrid[i][j] * discre) / 1000.0; //en km
					if (i  < n_vcate -1) fprintf(file,"%.2f ", valin); 
					if (i == n_vcate -1) fprintf(file,"%.2f\n", valin); 
				}	
			}
		}
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	}
	fclose(file);
	printf("Finaliza escritura archivo de salida\n");
	printf("---------------------------------\n\n");
	return 1;			
}

/*! ESCRIBE UAE-RASTER GENERADA POR USUARIO */
int write_uaerast()
{
char   buffer[255];
short int buff_int[4];
double buff_double[32];
float  *buff_float;
//int *buf_dat;
int i, j;
FILE *out;
    
	printf("Escribe raster generado %s\n", out_uaegen);		
	/**
	* Captura valores para cabecera
	*/
	sprintf(buffer,"DSBB");
	buff_int[0]    = nx;
	buff_int[1]    = ny;
	buff_double[0] = xlo;
	buff_double[1] = xhi;
	buff_double[2] = ylo;
	buff_double[3] = yhi;
	buff_double[4] = 0;
	buff_double[5] = 1;
	/**
	* Crea buffer dinamico para almacenar las lineas de cabecera
	*/
	buff_float = (float *)malloc(sizeof(float)*nx);
	//buf_dat = (int *)malloc(sizeof(int)*nx);
	/**
	* Crea y abre archivo de salida  
	*/
    if((out=fopen(out_uaegen,"wb"))==NULL)
	{
		printf("-------ERROR open file--------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("ATENCION: LA RUTA DE SALIDA DEL ARCHIVO NO ES CORRECTA\n");
		exit(0);
	}
	/**
	* Escribe datos de cabecera
	*/
	fwrite(buffer, 4, 1, out);
	fwrite(buff_int,    sizeof(short int)*2, 1, out);
	fwrite(buff_double, sizeof(double)*6, 1, out);
	/**
	* Lee y escribe linea a linea 
	*/
	for (j=0;j<ny;j++)
	{
		for(i=0;i<nx;i++)buff_float[i] = (float)rast_uae[i][j];         /*!< Almacena toda la linea en un buffer */
		fwrite(buff_float, sizeof(float)*nx, 1, out);                   /*!< Escribe el buffer en el archivo de salida */
	}
	fclose(out);
	printf("Finaliza escritura raster generado\n");
	return 1;
}

//**********************************************************************
//**********************************************************************
//MAIN------------------------------------------------------------------
//**********************************************************************
//**********************************************************************


int main(int argn, char **args)
{
int i, j, k, l;	
int call, tcal;
 
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
	if(args[1])sprintf(name_incfg,"%s",args[1]);                        /*!< Entra archivo config general como argumento */
	else
	{
		printf("ERROR: Por favor ingrese el archivo de configuracion\n");
		exit(0);
	}	
	call = read_cfg();
	if (call == 1)
	{
		//LEE ARCHIVOS INFILES
		sprintf(name_uaefile, "%s", inues);          //nombre uae infiles config file
		sprintf(name_varfile, "%s", invar);          //nombre vector infiles config file
		printf("%s\n", name_uaefile);
		printf("%s\n", name_varfile);
		if (tipmodo<=2)read_uaefile();                //lee archivo infiles con UAEs a procesar
		if (tipmodo==3)read_uaefilegen();             //lee archivo infiles para crear UAEs
		read_varfile();                               //lee archivo infiles de capas vectoriales a procesar		
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//EJECUTA SEGUN MODO
		if (tipmodo<=2) 
		{
			//PARA CADA UAE
			for(j=0;j<n_uaefile;j++)                                    // <----****----
			{
				tcal    = uefiles[j].uecalc;
				//SI ES PROCESABLE
				if (tcal == 1)
				{
					//LEE ARCHIVO UAE
					selec_uae(j);
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					//GENERA MATRIZ PARA INDICADORES
					printf("Genera matriz vargrid en %i cols por %i rows\n", hayvar, n_uaecsv); //agrega una columna por cada capa disponible
					vargrid = Make2IntegerArray(hayvar, n_uaecsv);
					for(k=0;k<n_uaecsv;k++) //col
					{
						for(l=0;l<hayvar;l++)vargrid[l][k]=0;           //pone todos los valores de la matriz a 0 (no lo hace al generarla)
					}
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					//PARA CADA CAPA VECTOR
					for(i=0;i<n_varfile;i++)                            // <----****----
					{
						csvpros = varfiles[i].vaispro;
						if (csvpros == 1)                               //si es procesada o no
						{
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
							//LEE CAPA VECTORIAL
							selec_var(i, j);
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
							//CALCULA DATOS
							calc_dat(j, i);
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
							//SI TIENE CATEGORIA
							if (concat>0)
							{
								write_catfile();                        //escribe archivo UAE-cat de salida
								freecatgrid();                          //libera matriz var-cat
								inistvarcat();                          //libera estrcutura var-cat 
							}	
							inistruvar();                               //libera estructura datos vectoriales
						}
					}
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					//ESCRIBE ARCHIVOS DE SALIDA Y LIBERA MEMORIA
					write_uaecsv();                                  //escribe archivo UAE de salida
					//--
					inistrucuae();                                      //libera estructura uae csv
					freeuaegrd();                                       //libera matriz uae raster
					freevargrid();                                      //libera matriz vectoriales
					printf("*********************************\n");
					printf("*********************************\n");
					printf("*********************************\n");
					printf("*********************************\n\n");
				}
			}
		}
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		if (tipmodo==3)      //ue usuario construida por parametros -- var csv sistema - Requiere leer input distinto
		{
			//PARA CADA UAE
			for(j=0;j<n_uaefile;j++)                                    // <----****----
			{
				tcal    = uefiles[j].uecalc;
				//SI ES PROCESABLE
				if (tcal == 1)
				{
					//LEE ARCHIVO UAE
					selec_uae(j);
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					//GENERA MATRIZ PARA INDICADORES
					printf("Genera matriz vargrid en %i cols por %i rows\n", hayvar, hayuae);
					vargrid = Make2IntegerArray(hayvar, n_uaecsv);      //la matriz se mantiene hasta terminar todas las capas vectoriales
					for(k=0;k<n_uaecsv;k++) //col
					{
						for(l=0;l<hayvar;l++)vargrid[l][k]=0;           //pone todos los valores de la matriz a 0 (no lo hace al generarla)
					}
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					//PARA CADA CAPA VECTOR 
					for(i=0;i<n_varfile;i++)                            // <----****----
					{
						csvpros = varfiles[i].vaispro;
						if (csvpros == 1)                               //si es procesada o no
						{
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
							//LEE CAPA VECTORIAL
							selec_var(i, j);
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
							//CALCULA DATOS
							calc_dat(j, i);
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
							//SI TIENE CATEGORIA
							if (concat>0)
							{
								write_catfile();
								freecatgrid();                          //libera var-cat raster estrcutura
								inistvarcat();                          //libera var-cat estrcutura
							}	
							inistruvar();                               //libera variable estrcutura
						}
					}
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					//ESCRIBE ARCHIVOS DE SALIDA Y LIBERA MEMORIA
					write_uaecsv();
					write_uaerast();
					//--
					inistrucuae();                                      //libera uae csv
					freeuaegrd();                                       //libera uae raster UAE
					freevargrid();                                      //libera uae estrcutura variable
					printf("*********************************\n");
					printf("*********************************\n");
					printf("*********************************\n");
					printf("*********************************\n\n");
				}
			}
		}
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		if (tipmodo>3 || tipmodo<1)	
		{
			printf("Atencion: modo fuera de rango\n");
			printf("Selecciones valores 1, 2 o 3\n");
			exit(0);
		}
	}
	printf("Finalizacion del proceso de calculo\n");
	getime();				
}	


