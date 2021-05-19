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
#define NUMUAE    30000      //NUM MAX DE UNIDADES ESPACIALES
#define NUMVAR  2000000      //NUM MAX DE PUNTOS POR VARIABLE CSV  SI SE SUPERA UN NUMERO MAXIMO DESBORDA MEMORA MINIMA
#define NUMCAT     1000      //NUM MAX DE CATEGORIAS POR VARIABLE CSV
#define CWD_MAX     256      //SIZE DIR TRABAJO
#define IN_SIZE     256      //SIZE NOMBRE IN TOTAL
#define IN_SIZE2    510      //SIZE NOMBRE IN TOTAL
#define OUT_SIZE    510      //SIZE NOMBRE OUT TOTAL

/**
* Copyright (C) 2019  Jose M. Marrero <josemarllin@gmail.com> and Hugo Yepes <hyepes@igepn.edu.ec>
* You may use, distribute and modify this code under the terms of the MIT License.
* The authors will not be held responsible for any damage or losses or for any implications 
* whatsoever resulting from downloading, copying, compiling, using or otherwise handling this 
* source code or the program compiled.
* Nombre script: Analiza_VR.c
* Version: 0.1-2021-02-11
* Linea de compilacion: gcc Analiza_VR.c -o Analiza_VR -lm
* Linea de ejecucion:  ./Analiza_VR configfilename.cfg
* Transfiere valores raster a cada elemento vectorial coincidente
* Genera un unico archivo vector-csv de salida por capa grupo de raster de entrada
* 
* Modo 1: una o varias variables usuario -> uae admin en sistema (todas o las elegidas)
* Modo 2: una o varias variables usuario -> uae usuario, solo 1
* Modo 3: una o varias variables usuario -> UAE usuario construida por parametros
* 
* Nota: En el modo sistema (administracion), el raster-csv tiene un campo mas con el el nombre
*/ 


/**
* GLOBAL VARIABLES
*/
char dirhom[100], dir_out[OUT_SIZE];                                                     //workig directories
char diruaer[100], diruaec[100], dirvarv[100], dirout[100];                              //directorios intermedios
char inues[150], invar[150];                                                             //nombres de archivos de entrada
char name_incfg[IN_SIZE], name_uaefile[IN_SIZE2], name_varfile[IN_SIZE];                 //rutas y nombres de archivos de entrada
char name_uaegrd[IN_SIZE], name_uescsv[IN_SIZE2], name_varcsv[IN_SIZE2];                 //rutas y nombres de archivos de entrada
char out_varcsv[OUT_SIZE], out_rasgen[OUT_SIZE];                                         //rutas y nombres de archivos de salida
int tipmodo, esint;                                                                      //parametros generales
int n_uaefile, n_uaecsv, n_varfile, n_var;                                               //conteo de datos
char namcsv[100], namgrd[100];                                                           //parametros infiles capa UAE
char namvari[150], tvartip[100];                                                         //parametros infiles capa vector
int orden, lineal, usokm, vorder, csvpros, discre, hayuae, hayvar;                       //parametros infiles
int txtsize, txtsizefin;                                                                 //funciones auxiliares
int vcol, vrow;                                                                          //funciones auxiliares     
double globx, globy;                                                                     //funciones auxiliares  
  
/**
* GLOBAL VARIABLES - RASTER
*/
int nuaes;
char header[8];
int nullval;
int totdemcel;
int nx, ny, nxc, nyc;
float demmax, demmin;
double xlo, xhi, ylo, yhi, zlo, zhi, resx, resy, invresx, invresy, resxx, resyy;
double xc, yc, dx, dy, ddx, ddy, dx2, dy2;
int **rast_uae, **intgrid;                                                               //matrices para raster y datos en enteros
double **rast_var, **flogrid;                                                            //matrices para raster y datos en decimal

/**
* ALMACENAMIENTO DE DATOS EN ESTRUCTURAS
*/

/*! ALMACENA DATOS INFILES CAPAS RASTER */
typedef struct  
{
	
	int    uecalc;
	int    uenulval;
	float  uemin;
	float  uemax;
	int    ueposi;
	char   uecsvfile[100];
	char   uegrdfile[100];
	char   uename[80];       //uae nombre
	//--
	int    ueuskm;           //para rastser generado
	double uexcor;
	double ueycor;
	float  uerad;
	float  ueres;
}UES;
UES uefiles[UAECON];

/*! ALMACENA DATOS INFILES CAPAS VEC-BASE */
typedef struct  
{
	int vaispro;           //es calculada
	int vaislin;           //es linea
	int vaisdis;           //discretizado
	int vaorder;           //orden de la variable
	char varinifile[150]; 
	char vartipo[100];     //grupo al que pertenece
}VAR;
VAR varfiles[VARCON];

/*! ALMACENA DATOS CAPA RASTER-CSV ADMIN */
typedef struct  
{  
	int    bnum;       //barrio numero inicio en cero 
	int    bnid;       //barrio id 
	char   btxtid[20]; //codigo id
	double bperim;
	double barea;
	double bxcoor;
	double bycoor;
	char   bname[80];  //ue nombre
}UAE;
UAE stuaecsv[NUMUAE];

/*! ALMACENA DATOS CAPA VEC-BASE */
typedef struct  
{
	int    datid;
	int    datyp;
	char   datcod[30];
	double datx;
	double daty;
	//--
	float dgrdvar;       //valor directo variable de raster
	//--
	int   dzonid;        //zonales dmq id 1
	char dzoncod[50];    //zonales dmq code 1
	char dzonam[80];     //zonales dmq name 1
	int dszonid;          //subzonales dmq ide 2
	char dszonco[50];    //subzonales dmq code 2
	char dszonam[80];    //subzonales dmq name 2
	int dparid;          //parroquia dmq id 3
	char dparcod[50];    //parroquia dmq code 3
	char dparnam[80];    //parroquia dmq name 3
	int dbarrid;         //barrios
	char dbarcod[50];    //barrios dmq code 4
	char dbarnam[80];    //barrios dmq name 4
	int dparinid;        //parroquia inec 5
	char dparincod[50];  //parroquia inec code 5
	char dparinnam[80];  //parroquia inec name 5
	int dsecid;          //sector censal inec 6
	char dseccod[50];    //sector censal inec code 6
	char dsecnam[80];    //sector censal inec name 6
	int dmaseid;         //mixto sector censal vs manzana inec 7
	char dmasecod[50];   //mixto sector censal vs manzana inec code 7
	char dmasenam[80];   //mixto sector censal vs manzana inec name 7
	//--
}DAT;
DAT stvarcsv[NUMVAR];


/**
* DECLARACION DE FUNCIONES
*/
int calc_size2(const char* str1, const char* str2);
int calc_size3(const char* str1, const char* str2, const char* str3);
int calc_size4(const char* str1, const char* str2, const char* str3, const char* str4);
double** Make2DDoubleArray(int arraySizeX, int arraySizeY);
int** Make2IntegerArray(int arraySizeX, int arraySizeY);
void calc_vecindex(double vx, double vy);
void get_coor(int col, int row);
int getime(void);
void crea_mkdir(char *path, mode_t mode);
void inistruc(void);
int freeuaegrd(void);
int read_cfg(void);


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
		fscanf(file,"%s %i", texto, &esint);                            //tipo de raster, integer (1) o float (0)
		fscanf(file,"%s", texto);
		fscanf(file,"%s %s", texto, diruaer);
		fscanf(file,"%s %s", texto, diruaec);
		fscanf(file,"%s %s", texto, dirvarv);
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
	//IMPRIME DATOS LEIDOS
	printf("TIPO MODO %i\n", tipmodo);
	printf("CON GRD-CSV %i\n", esint);
	printf("DIR_IN UAE GRD %s\n", diruaer);
	printf("DIR_IN UAE CSV %s\n", diruaec);
	printf("DIR_INVAR CSV %s\n", dirvarv);
	printf("DIR_OUTGEN %s\n", dirout);
	printf("UAE_CONFIG_FILE %s\n", inues);
	printf("VAR_CONFIG_FILE %s\n", invar);
	return 1;		
}	

/*! LEYENDO CONFIG RASTER INFILES */
int read_uaefile(void)
{
FILE *file;
int i;
int tpos, tnul, tcal;
char texto[256], tcsv[100],  tgrd[100], tname[100];	
float tmin, tmax;
	//LEE INFILES CON LISTADO DE RASTER
	i = 0;
	hayuae=0;
	printf("\n***Lectura archivo raster, %s***\n", name_uaefile);
	if ((file = fopen(name_uaefile,"rt"))== NULL)
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
		printf("%s\n",texto); 
		while (fscanf(file,"%i %i %f %f %i %s %s" , 
					&tcal,
					&tnul,
					&tmin,
					&tmax,
					&tpos,
					tcsv,
					tgrd
					) == 7)
		{
			uefiles[i].ueposi    = tpos;
			uefiles[i].uecalc    = tcal;
			uefiles[i].uenulval  = tnul;
			uefiles[i].uemin     = tmin;
			uefiles[i].uemax     = tmax;
			sprintf(uefiles[i].uecsvfile,"%s", tcsv);
			sprintf(uefiles[i].uegrdfile,"%s", tgrd);
			sprintf(uefiles[i].uename, "%s", tname);
			printf("%i: %i %i %f %f %i %s %s\n", i, tcal, tnul, tmin, tmax, tpos, tcsv, tgrd); //imprime filas
			if (tcal == 1) hayuae +=1;                                  //cuenta raster que se van a ser procesadas
			i++;
		}
	}
	if (i == 0)
	{
		printf("Atencion error de lectura en unidades espaciales\n");
		exit(0);
	}	
	n_uaefile = i; //numero de unidades espaciales
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

/*! LEYENDO CONFIG RASTER GENERADOR INFILES */
int read_uaefilegen(void)
{
FILE *file;
int i;
int tuskm, tcal, tidpt;
char texto[256], tname[100];	
float trad, tres;
double txcoor, tycoor;
	//LEE ARCHIVO CON LISTADO DE RASTER PARA SER GENERADAS A PARTIR DE PARAMETROS
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
	}
	else
	{
		fgets(texto,256,file); 
		printf("%s\n",texto); 
		while (fscanf(file,"%i %i %i %lf %lf %f %f %s" , 
					&tcal,
					&tuskm,
					&tidpt,
					&txcoor,
					&tycoor,
					&trad,
					&tres,
					tname
					) == 8)
		{
			uefiles[i].uecalc  = tcal;
			uefiles[i].ueuskm  = tuskm;
			uefiles[i].ueposi  = tidpt;
			uefiles[i].uexcor  = txcoor;
			uefiles[i].ueycor  = tycoor;
			uefiles[i].uerad   = trad;
			uefiles[i].ueres   = tres;
			sprintf(uefiles[i].uecsvfile, "%s", tname);
			printf("%i: %i %i %i %lf %lf %f %f %s\n", i, tcal, tuskm, tidpt, txcoor, tycoor, trad, tres, tname); //imprime filas
			if (tcal == 1) hayuae +=1;
			i++;
		}
	}
	n_uaefile = i; //numero de unidades espaciales
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

/*! LEYENDO CONFIG VECTOR-CSV INFILES */
int read_varfile(void)
{
FILE *file;
int i;
int tlin, tdis, tord, tpros;
char texto[256], tnamvar[100], tvartip[100];	
	i = 0;
	printf("\n***Lectura archivo vector, %s***\n", name_varfile);
	if ((file = fopen(name_varfile,"rt"))== NULL)
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
		while (fscanf(file,"%i %i %i %i %s %s" , 
					&tpros,
					&tlin,
					&tdis,
					&tord,
					tnamvar,
					tvartip
					) == 6)
		{
			varfiles[i].vaispro  = tpros;
			varfiles[i].vaislin  = tlin;
			varfiles[i].vaisdis  = tdis;
			varfiles[i].vaorder  = tord;
			sprintf(varfiles[i].varinifile,"%s", tnamvar);
			sprintf(varfiles[i].vartipo,"%s", tvartip);
			printf("%i %i %i %i %s %s\n", tpros, tlin, tdis, tord, tnamvar, tvartip);
			if (tpros == 1) hayvar +=1;
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

/*! LEYENDO DATOS RASTER EN CSV FORMAT */
int read_uecsv(void)
{
FILE *file;
int i, tid;
char texto[256], txtid[20], tname[100];
double txcoor, tycoor, tarea, tperi;
	
	i       =0;
	n_uaecsv=0;
	printf("\n***Lectura archivo raster-csv, %s***\n", name_uescsv);
	if ((file = fopen(name_uescsv,"rt"))== NULL)
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
		while (fscanf(file,"%i %s %lf %lf %lf %lf %s" , 
					&tid,
					txtid,
					&tperi,
					&tarea,
					&txcoor,
					&tycoor,
					tname      
					) == 7)
	
		{	
			//printf("%i %s %lf %lf\n", tid, txtid, txcoor, tycoor);
			stuaecsv[i].bnum   = i;
			stuaecsv[i].bnid   = tid;
			sprintf(stuaecsv[i].btxtid,"%s", txtid);
			stuaecsv[i].bperim = tperi;
			stuaecsv[i].barea  = tarea;
			stuaecsv[i].bxcoor = txcoor;
			stuaecsv[i].bycoor = tycoor;
			sprintf(stuaecsv[i].bname, "%s", tname);
			i++;
		}	
	}
	n_uaecsv = i; //numero de tipologias
	fclose(file);
	printf("Numero total de unidades espaciales = %i id max %i\n", i, stuaecsv[i-1].bnid);
	printf("end read file\n");
	printf("---------------------------------\n\n");
	return 1;
}

/*! LEYENDO DATOS RASTER EN BINARY RASTER FORMAT */
int read_uegrd(void)
{
FILE *in;
char cabecera[8];
short int datoin[4];
double datodo[8];
float datofl[4];
int i, j, contnul, conespe, contgod;
	printf("\n***Lectura archivo raster-grd, %s***\n", name_uaegrd);
	if((in=fopen(name_uaegrd,"rb"))==NULL)
	{
		printf("-------ERROR open file--------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		printf("-----------ERROR--------------\n");
		exit(0);
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
	conespe=0;
	for(j=0;j<8;j++)printf("%lf\n",datodo[j]);
	/**
	* Crea matriz en memoria y asigna valores
	*/                         
	if (esint == 1)rast_uae = Make2IntegerArray(nx, ny);                /*!< crea matriz en memoria valores enteros */
	if (esint == 0)rast_var = Make2DDoubleArray(nx, ny);                /*!< crea matriz en memoria valores decimales */
	for(j=0;j<ny;j++)
	{
		for(i=0;i<nx;i++)
		{                       
			fread(&datofl,sizeof(float),1,in);                          /*!< lee dato a dato por cada linea */
			//ASIGNAMOS VALORES NULOS
			if((datofl[0] > demmax) || (datofl[0] <= demmin))           /*!< si el valor esta fuera de limite, se asigna valor nulo */
			{
				datofl[0] = nullval;                                
				contnul+=1;
			}
			//ASIGNAMOS VALORES VALIDOS
			else if((datofl[0] <= demmax) && (datofl[0] > demmin))
			{
				contgod +=1;
			}
			else	
			{
				datofl[0] = nullval;   //cuando el valor nulo es positivo muy proximo a cero
				conespe+=1;
			} 
			if (esint == 1)rast_uae[i][j]  = (int)datofl[0]; 
			if (esint == 0)rast_var[i][j]  = (double)datofl[0];  		
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
	printf("---------------------------------\n\n");
	printf("Total celdas en RASTER = %i\n", totdemcel);
	printf("Area de trabajo %lf\n", contgod * resx * resy);
	printf("Total celdas en nulas en RASTER = %i\n", contnul);
	printf("Area nula %lf\n", contnul * resx * resy);
	printf("Total celdas en nulas-especiales en RASTER = %i\n", conespe);
	printf("Area especial no valida %lf\n", conespe * resx * resy);
	printf("Final de lectura de RASTER\n");  
	printf("---------------------------------\n\n");
	return 1;
}

/*! CREANDO UAE A PARTIR DE CENTROIDE Y RADIO */
int crea_uae(int num)
{
int i, j;
	//CREA UAE-RASTER A PARTIR DE PARAMENTROS, SOLO MODO 3
	printf("Genera raster a partir de punto y radio %i\n", num);
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
int i, tid;
char texto[256], typ[30];
double txcoor, tycoor;
	
	i = 0;
	printf("\n***Lectura archivo vector-csv, %s***\n", name_varcsv);
	if ((file = fopen(name_varcsv,"rt"))== NULL)
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
		while (fscanf(file,"%i %s %lf %lf" , 
					&tid,
					typ,
					&txcoor,
					&tycoor         
					) == 4)
	
		{	
			stvarcsv[i].datid  = tid;
			sprintf(stvarcsv[i].datcod,"%s", typ);
			stvarcsv[i].datx   = txcoor;
			stvarcsv[i].daty   = tycoor;
			i++;
		}	
	}
	n_var = i;
	fclose(file);
	printf("Numero total de puntos = %i\n", i);
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

	//calculate column and row-----------------------------------------
	//row number
	difx = vx - xlo; 
	vcol = (int)(difx / resx);
	demx = (vcol * resx) + xlo; 
	if ((vx - demx) > (resx/2))vcol++; 
	//clomumn number
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

/*! LEE DATOS RASTER GRD Y CSV */
int selec_uae(int num)
{
int txtsize;
	if (tipmodo<=2)
	{
		//LEE UNIDAD DE ANALISIS ESPACIAL
		orden   = uefiles[num].ueposi;
		nullval = uefiles[num].uenulval;                      //valor nulo a utilizar
		demmin  = uefiles[num].uemin;                         //valor grd minimo
		demmax  = uefiles[num].uemax;                         //valor grd maximo
		sprintf(namcsv,"%s", uefiles[num].uecsvfile);         //nombre corto csv file (en float se pone alguna palabra)
		sprintf(namgrd,"%s", uefiles[num].uegrdfile);         //nombre corto grd file
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//CONSTRUYE RUTA Y NOMBRES DE ENTRADA
		txtsize = calc_size3(dirhom, diruaec, namcsv);
		char uescsv_ini[txtsize];
		sprintf(uescsv_ini,"%s%s%s", dirhom, diruaec, namcsv);
		if(txtsize < IN_SIZE2) sprintf(name_uescsv,"%s", uescsv_ini);
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
		printf("Calcula UAE %i -- nul %i min %f max %f\n\n", num, nullval, demmin, demmax);
		read_uegrd();                                       //read unidad espacial grd
		if (tipmodo==1) read_uecsv();                       //read unidad espacial csv si tiene archivo csv
	}
	//GENERA VALORES DE UAE DESDE INFILE - GEN  
	if (tipmodo==3)  
	{   
		//CONSTRUYE UAE CON FORMA DE CUADRADO
		orden = uefiles[num].ueposi;
		nullval=-9999;
		sprintf(namcsv,"%s", uefiles[num].uecsvfile);
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		crea_uae(num);
	}
	return 1;
}

/*! LEE DATOS VECTOR-CSV */
int selec_var(int num)
{
	//CONTENIDO ARCHIVO INFILE...
	lineal = varfiles[num].vaislin;                     //es lineal
	discre = varfiles[num].vaisdis;                     //discretizado utilizado
	vorder = varfiles[num].vaorder;                     //orden de lectura y asignacion de variable
	sprintf(namvari,"%s", varfiles[num].varinifile);    //nombre archivo variable
	sprintf(tvartip,"%s", varfiles[num].vartipo);       //nombre corto variable
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//ASIGNA NOMBRE ARCHIVO VARIABLE
	printf("Calucla Vector %i tipo %s\n\n", vorder, tvartip);
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
	return 1;
}

/*! CONSTRUYE NOMBRE ARCHIVO DE SALIDA PARA VECTOR */
int getnamout(void)
{
char uaename[25];
	
	if (tipmodo==1)sprintf(uaename, "VR_r-sis_");
	if (tipmodo==2)
	{
		if (esint == 1)sprintf(uaename, "VR_r-user-int_");
		if (esint == 0)sprintf(uaename, "VR_r-user-float_");
	}
	if (tipmodo==3)sprintf(uaename, "VR_r-user-gen_");
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//Captura nombres de entrada sin extension
	char *namebase = strtok(namvari, ".");
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	txtsize = calc_size4(dirhom, dirout, tvartip, namvari);
	txtsizefin = txtsize+5+strlen(uaename);
	char csvaout[txtsizefin];
	sprintf(csvaout,"%s%s%s%s_%i.csv", dirhom, dirout, uaename, namebase, vorder);
	if(txtsizefin < OUT_SIZE) sprintf(out_varcsv,"%s", csvaout);
	else 
	{
		printf("Se ha excedido out_varcsv en %i\n", txtsizefin);
		exit(0);
	}
	return 1;
}	

/*! CONSTRUYE NOMBRE ARCHIVO DE SALIDA PARA RASTER-GEN */
int getnamout_rast(void)
{
char uaename[25];
	sprintf(uaename, "VR_r-user-gen_");
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//Captura nombres de entrada sin extension
	char *namebase = strtok(namvari, ".");
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	txtsize = calc_size4(dirhom, dirout, uaename, namcsv);
	txtsizefin = txtsize+10;
	char grdout[txtsizefin];
	sprintf(grdout,"%s%s%s%s_%i.grd", dirhom, dirout, uaename, namebase, vorder);
	if(txtsizefin < OUT_SIZE) sprintf(out_rasgen,"%s", grdout);
	else 
	{
		printf("Se ha excedido out_rasgen en %i\n", txtsizefin);
		exit(0);
	}
	return 0;
}

/*! INICIALIZA ESTRUCTURA VECTORIAL */
void inistruc(void)
{
int i;	
	for(i=0;i<n_uaecsv;i++)  
	{ 
		stuaecsv[i].bnum   = 0;
		stuaecsv[i].bnid   = 0.0;
		stuaecsv[i].bperim = 0.0;
		stuaecsv[i].barea  = 0.0;
		stuaecsv[i].bxcoor = 0.0;
		stuaecsv[i].bycoor = 0.0;
		sprintf(stuaecsv[i].btxtid," ");
		sprintf(stuaecsv[i].bname," ");
	}
}		

/*! LIBERA MEMORIA DE RASTER */
int freeuaegrd(void)
{
int k;
	if (esint == 1)
	{
		printf("Liberando raster integer\n");
		for(k=0;k<nx;k++)free(rast_uae[k]);
		free(rast_uae);
		rast_uae = NULL;
	}
	if (esint == 0)
	{
		printf("Liberando raster float\n");
		for(k=0;k<nx;k++)free(rast_var[k]);
		free(rast_var);
		rast_var = NULL;
	}
	return 1;
}

/*! LIBERA MEMORIA DE MATRICES */
int freedatgrd(void)
{
int k;
	if (esint == 1)
	{
		printf("Liberando matriz datos integer\n");
		for(k=0;k<nuaes;k++)free(intgrid[k]);
		free(intgrid);
		intgrid = NULL;
	}
	if (esint == 0)
	{
		printf("Liberando matriz datos float\n");
		for(k=0;k<nuaes;k++)free(flogrid[k]);
		free(flogrid);
		flogrid = NULL;
	}
	return 1;
}


//**********************************************************************
//CALCULA---------------------------------------------------------------
//**********************************************************************

/*! CALCULA EN MODO SISTEMA - NIVELES ADMINISTRATIVOS */
int caladmin(int posicion)
{
int i,j, value, tid, ok;
int validas, novalid, fuerade;
double txcoor, tycoor;
	validas=0;
	novalid=0;
	fuerade=0;
	for (i=0;i<n_var;i++)
	{
		txcoor = stvarcsv[i].datx;
		tycoor = stvarcsv[i].daty;
		if((txcoor > xlo && txcoor < xhi) && (tycoor > ylo && tycoor < yhi)) //si esta en el raster
		{
			calc_vecindex(txcoor, tycoor);
			value = rast_uae[vcol][vrow];
			if (value != nullval)  //si no es nulo
			{
				ok=0;
				for (j=0;j<n_uaecsv;j++)
				{
					tid = stuaecsv[j].bnid;
					//if (posicion == 4) printf("fallidas %i val %i :: %i\n", i, value, tid);
					if (value == tid)
					{
						//if (posicion == 4) printf("Acierto %i val %i :: %i ---------------------------------------\n", i, value, tid);
						if (posicion == 1) //zonales
						{
							stvarcsv[i].dzonid = value;
							sprintf(stvarcsv[i].dzoncod,"%s", stuaecsv[j].btxtid);  
							sprintf(stvarcsv[i].dzonam,"%s", stuaecsv[j].bname); 
						}	
						if (posicion == 2)  //subzonal
						{
							stvarcsv[i].dszonid = value;
							sprintf(stvarcsv[i].dszonco,"%s", stuaecsv[j].btxtid);  
							sprintf(stvarcsv[i].dszonam,"%s", stuaecsv[j].bname);
						}
						if (posicion == 3)  //parroquias dmq
						{
							stvarcsv[i].dparid = value;
							sprintf(stvarcsv[i].dparcod,"%s", stuaecsv[j].btxtid);  
							sprintf(stvarcsv[i].dparnam,"%s", stuaecsv[j].bname);
						}
						if (posicion == 4) //barrios dmq
						{
							stvarcsv[i].dbarrid = value;
							sprintf(stvarcsv[i].dbarcod,"%s", stuaecsv[j].btxtid);  //zonales dmq code
							sprintf(stvarcsv[i].dbarnam,"%s", stuaecsv[j].bname);
						}
						if (posicion == 5) //parroquias inec
						{
							stvarcsv[i].dparinid = value;
							sprintf(stvarcsv[i].dparincod,"%s", stuaecsv[j].btxtid);  //zonales dmq code
							sprintf(stvarcsv[i].dparinnam,"%s", stuaecsv[j].bname);
						}
						if (posicion == 6) //secotres censales inec
						{
							stvarcsv[i].dsecid = value;
							sprintf(stvarcsv[i].dseccod,"%s", stuaecsv[j].btxtid);  //zonales dmq code
							sprintf(stvarcsv[i].dsecnam,"%s", stuaecsv[j].bname);
						}
						if (posicion == 7) //manzanas secotes inec
						{
							//printf("%i val %i cod %s nam %s\n", i, value, uniespa[j].btxtid, uniespa[j].bname);
							stvarcsv[i].dmaseid = value;
							sprintf(stvarcsv[i].dmasecod,"%s", stuaecsv[j].btxtid);  //zonales dmq code
							sprintf(stvarcsv[i].dmasenam,"%s", stuaecsv[j].bname);
						}
						validas+=1;
						ok=1;
					}
				}
				//if (posicion == 4 && ok==0) printf("fallidas %i val %i\n", i, value);
			}
			if (value == nullval || ok==0)
			{
				if (posicion == 1) //zonales
				{
					stvarcsv[i].dzonid = -9999;
					sprintf(stvarcsv[i].dzoncod,"-9999");  
					sprintf(stvarcsv[i].dzonam,"FUERA_LIMITE"); 
				}	
				if (posicion == 2)  //subzonal
				{
					stvarcsv[i].dszonid =  -9999;
					sprintf(stvarcsv[i].dszonco,"-9999");  
					sprintf(stvarcsv[i].dszonam,"FUERA_LIMITE");
				}
				if (posicion == 3)  //parroquias dmq
				{
					stvarcsv[i].dparid =  -9999;
					sprintf(stvarcsv[i].dparcod,"-9999");  
					sprintf(stvarcsv[i].dparnam,"FUERA_LIMITE");
				}
				if (posicion == 4) //barrios dmq
				{
					stvarcsv[i].dbarrid =  -9999;
					sprintf(stvarcsv[i].dbarcod,"-9999");  //zonales dmq code
					sprintf(stvarcsv[i].dbarnam,"FUERA_LIMITE");
				}
				if (posicion == 5) //parroquias inec
				{
					stvarcsv[i].dparinid =  -9999;
					sprintf(stvarcsv[i].dparincod,"-9999");  //zonales dmq code
					sprintf(stvarcsv[i].dparinnam,"FUERA_LIMITE");
				}
				if (posicion == 6) //secotres censales inec
				{
					stvarcsv[i].dsecid =  -9999;
					sprintf(stvarcsv[i].dseccod,"-9999");  //zonales dmq code
					sprintf(stvarcsv[i].dsecnam,"FUERA_LIMITE");
				}
				if (posicion == 7) //manzanas secotes inec
				{
					stvarcsv[i].dmaseid =  -9999;
					sprintf(stvarcsv[i].dmasecod,"-9999");  //zonales dmq code
					sprintf(stvarcsv[i].dmasenam,"FUERA_LIMITE");
				}
				novalid+=1;
			}
		}
		else
		{
			if (posicion == 1) //zonales
			{
				stvarcsv[i].dzonid = -9999;
				sprintf(stvarcsv[i].dzoncod,"-9999");  
				sprintf(stvarcsv[i].dzonam,"FUERA_LIMITE"); 
			}	
			if (posicion == 2)  //subzonal
			{
				stvarcsv[i].dszonid =  -9999;
				sprintf(stvarcsv[i].dszonco,"-9999");  
				sprintf(stvarcsv[i].dszonam,"FUERA_LIMITE");
			}
			if (posicion == 3)  //parroquias dmq
			{
				stvarcsv[i].dparid =  -9999;
				sprintf(stvarcsv[i].dparcod,"-9999");  
				sprintf(stvarcsv[i].dparnam,"FUERA_LIMITE");
			}
			if (posicion == 4) //barrios dmq
			{
				stvarcsv[i].dbarrid =  -9999;
				sprintf(stvarcsv[i].dbarcod,"-9999");  //zonales dmq code
				sprintf(stvarcsv[i].dbarnam,"FUERA_LIMITE");
			}
			if (posicion == 5) //parroquias inec
			{
				stvarcsv[i].dparinid =  -9999;
				sprintf(stvarcsv[i].dparincod,"-9999");  //zonales dmq code
				sprintf(stvarcsv[i].dparinnam,"FUERA_LIMITE");
			}
			if (posicion == 6) //secotres censales inec
			{
				stvarcsv[i].dsecid =  -9999;
				sprintf(stvarcsv[i].dseccod,"-9999");  //zonales dmq code
				sprintf(stvarcsv[i].dsecnam,"FUERA_LIMITE");
			}
			if (posicion == 7) //manzanas secotes inec
			{
				stvarcsv[i].dmaseid =  -9999;
				sprintf(stvarcsv[i].dmasecod,"-9999");  //zonales dmq code
				sprintf(stvarcsv[i].dmasenam,"FUERA_LIMITE");
			}
			fuerade+=1;
		}
	}
	printf("Resultados del cruce, total %i\n", n_var);
	printf("Elementos dentro UAE %i\n", validas);
	printf("Elementos fuera  UAE: %i\n", novalid);
	printf("Elementos fuera raster: %i\n", fuerade);
	printf("****************\n\n");
	return 1;
}

/*! CALCULA EN MODO USUARIO */
int calcusu(int numuae)
{
int i, value;
int validas, novalid, fuerade;
double txcoor, tycoor, valdir;
	validas=0;
	novalid=0;
	fuerade=0;
	for (i=0;i<n_var;i++)
	{
		//coordenadas punto capa vectorial
		txcoor = stvarcsv[i].datx;
		tycoor = stvarcsv[i].daty;
		//SI ESTA DENTRO DEL RASTER
		if((txcoor > xlo && txcoor < xhi) && (tycoor > ylo && tycoor < yhi)) //si esta en el raster
		{
			//OBTENEMOS FILA Y COLUMNA DEL RASTER
			calc_vecindex(txcoor, tycoor);                //calculo indices de la matriz
			//SI LOS VALORES ESTAN EN UAE RASTER
			if (vcol < nx && vrow < ny)
			{
				if (esint == 1)                               //si es entero
				{
					value = rast_uae[vcol][vrow];             //capturo valor del raster
					if (value != nullval)                     //si no es nulo
					{
						intgrid[numuae][i] = value;           //asigno valor del raster a la matriz de datos
						validas+=1;
					}
					if (value == nullval)
					{
						intgrid[numuae][i] = -9999;
						novalid+=1;
					}
						
				}	
				if (esint == 0)                               //si es decimal
				{
					valdir = rast_var[vcol][vrow];
					if (valdir != nullval)                    //si no es nulo
					{
						flogrid[numuae][i] = valdir;          //asigno valor del raster a la matriz de datos
						validas+=1;
					}
					if (valdir == nullval)
					{
						flogrid[numuae][i] = -9999;
						novalid+=1;
					}
				}
			}
			else fuerade+=1;
		}
		else                                              //si esta fuera de raster
		{
			if (esint == 1)intgrid[numuae][i] = -9999;
			if (esint == 0)flogrid[numuae][i] = -9999;
			fuerade+=1;
		}
	}
	printf("Resultados del cruce, total %i\n", n_var);
	printf("Fuera Raster %i\n", fuerade);
	printf("Dentro fallidas %i\n", novalid);
	printf("Dentro correctas %i\n", validas);
	printf("****************\n\n");
	return 1;
}

//**********************************************************************
//ESCRIBE---------------------------------------------------------------
//**********************************************************************

/*! ESCRIBE DATOS VECTOR-CSV EN MODO SISTEMA */
int write_vecsis(void)
{
FILE *file;
int i;
	printf("Escribe archivo vector base en modo sistema %s\n", out_varcsv);
	i = 0;
	if((file = fopen(out_varcsv,"w"))== NULL)
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
	else
	{
		fprintf(file,"%s\n",
			"codid tipodat xcoor ycoor zonid zoncod zoname subzoid subzocod subzonam pardmqid pardmqcod pardmqnam barrid barrcod barrnom parinecid parinecod parinecnam secid secod secnam mansecid mansecod mansecnam"); //primera linea
		for(i=0;i<n_var;i++)
		{		
			fprintf(file,"%i %s %lf %lf %i %s %s %i %s %s %i %s %s %i %s %s %i %s %s %i %s %s %i %s %s\n",
				
				stvarcsv[i].datid,
				stvarcsv[i].datcod,
				stvarcsv[i].datx,
				stvarcsv[i].daty,
				stvarcsv[i].dzonid,        //zonales dmq id
				stvarcsv[i].dzoncod,    //zonales dmq code
				stvarcsv[i].dzonam,     //zonales dmq name
				stvarcsv[i].dszonid,          //subzonales dmq ide
				stvarcsv[i].dszonco,    //subzonales dmq code
				stvarcsv[i].dszonam,    //subzonales dmq name
				stvarcsv[i].dparid,          //parroquia dmq
				stvarcsv[i].dparcod,    //parroquia dmq code
				stvarcsv[i].dparnam,    //parroquia dmq name
				stvarcsv[i].dbarrid,         //barrios
				stvarcsv[i].dbarcod,    //barrios dmq code
				stvarcsv[i].dbarnam,   //barrios dmq name
				stvarcsv[i].dparinid,        //parroquia inec
				stvarcsv[i].dparincod,  //parroquia inec code
				stvarcsv[i].dparinnam,  //parroquia inec name
				stvarcsv[i].dsecid,          //sector censal inec
				stvarcsv[i].dseccod,    //sector censal inec code
				stvarcsv[i].dsecnam,    //sector censal inec name
				stvarcsv[i].dmaseid,      //mixto sector censal vs manzana inec
				stvarcsv[i].dmasecod, //mixto sector censal vs manzana inec code
				stvarcsv[i].dmasenam  //mixto sector censal vs manzana inec name
				);	
		}
	}
	fclose(file);
	printf("End writing output file\n");
	printf("---------------------------------\n");
	return 1;
}	

/*! ESCRIBE DATOS VECTOR-CSV EN MODO USUARIO */
int write_vecuser(void)
{
FILE *file;
int i, j, okfin;
char colnam[15], val_ord[n_uaefile];

	printf("Escribe archivo vector base en modo usuario %s\n", out_varcsv);
	i = 0;
	if((file = fopen(out_varcsv,"w"))== NULL)
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
	else
	{
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//CONSTRUYE CABECERA
		fprintf(file,"%s ", "ID TIPO XCOOR YCOOR"); //primera linea
		okfin=0;
		if (esint == 1)sprintf(colnam,"RAS-I");
		if (esint == 0)sprintf(colnam,"RAS-F");
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//CALCULA NUMERO DE RASTER-ORDEN
		j=0;
		for(i=0;i<n_uaefile;i++)
		{
			if (uefiles[i].uecalc == 1)
			{
				val_ord[j] = uefiles[i].ueposi;
				j++;
			}
		}
		//COMPLETA CABECERA CON COLUMNAS INDICADORES
		for(i=0;i<hayuae;i++)
		{
			if (i  < hayuae -1) fprintf(file,"%s-%i ", colnam, val_ord[i]); 
			if (i == hayuae -1) 
			{
				fprintf(file,"%s-%i\n", colnam,val_ord[i]);
				okfin=1;
			}
		}
		if (okfin==0)fprintf(file,"\n");;
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//ESCRIBE DATOS 
        for(i=0;i<n_var;i++)
        {		
			fprintf(file,"%i %s %lf %lf ",
				
				stvarcsv[i].datid,
				stvarcsv[i].datcod,
				stvarcsv[i].datx,
				stvarcsv[i].daty
				);
			if (esint == 1)
			{
				
				for(j=0;j<hayuae;j++)
				{ 
					if (j  < hayuae -1) fprintf(file,"%i ", intgrid[j][i]); 
					if (j == hayuae -1) fprintf(file,"%i\n",intgrid[j][i]);
				}
			}
			if (esint == 0)
			{
				for(j=0;j<hayuae;j++)
				{
					if (j  < hayuae -1) fprintf(file,"%.2lf ", flogrid[j][i]); 
					if (j == hayuae -1) fprintf(file,"%.2lf\n",flogrid[j][i]);
				}
			}		
		}
	}
	fclose(file);
	printf("Finaliza escritura archivo de salida\n");
	printf("---------------------------------\n");
	return 1;
}

/*! ESCRIBE UAE-RASTER GENERADA POR USUARIO */
int write_rastgen()
{
char   buffer[255];
short int buff_int[4];
double buff_double[32];
float  *buff_float;
//int *buf_dat;
int i, j;
FILE *out;
    
	printf("Write rast-gen %s\n", out_rasgen);		
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
    if((out=fopen(out_rasgen,"wb"))==NULL)
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
int j, i, k, l, call, tcal;
int idxuae;

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
	//lee archivo de configuracion principal	
	call = read_cfg();
	if (call == 1)
	{
		//LEE ARCHIVOS INFILES
		sprintf(name_uaefile, "%s", inues);
		sprintf(name_varfile, "%s", invar);
		printf("%s\n", name_uaefile);
		printf("%s\n", name_varfile);
		if (tipmodo<=2)read_uaefile();                                  //lee archivo con raster a procesar
		if (tipmodo==3)read_uaefilegen();                               //lee archivo con raster generados a procesar
		read_varfile();                                                 //lee archivo con capas vectoriales a procesar
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 
		//TRANSFERENCIA EN MODO SISTEMA CON RASTER-CSV
		if (tipmodo==1)  
		{
			//PARA CADA DATO VECTORIAL
			for(i=0;i<n_varfile;i++)                                    // <----****----       
			{
				csvpros = varfiles[i].vaispro;
				if (csvpros == 1)                                       //si es procesado o no
				{
					//LEE DATO VECTORIAL
					selec_var(i);
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					//PARA CADA RASTER 
					for(j=0;j<n_uaefile;j++)                            // <----****----
					{
						//lectrua unidadese espaciales
						tcal    = uefiles[j].uecalc;
						if (tcal == 1)                                  //solo aquellas consideradas
						{
							//UNIDAD DE ANALISIS ESPACIAL
							selec_uae(j);
							//-----------
							caladmin(orden);
							//----------- 
							freeuaegrd();                               //libera memoria
							inistruc();
						}
					}
					getnamout();
					write_vecsis();	
				}
			}
		}
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//TRANSFERENCIA DIRECTA DE VALOR SIN CONSIDERAR CSV DE RASTER
		if (tipmodo==2)
		{
			//PARA CADA DATO VECTORIAL
			for(i=0;i<n_varfile;i++)                                       
			{
				csvpros = varfiles[i].vaispro;
				if (csvpros == 1)                                       //si es procesado o no
				{
					//LEE DATO VECTORIAL
					selec_var(i);
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					//GENERA MATRICEZ PARA INDICADORES                  
					if (esint == 1)                                     //matriz integer para datos
					{
						printf("Genera matriz integer en %i por %i\n", hayuae, n_var);
						intgrid = Make2IntegerArray(hayuae, n_var);     //esta matriz se mantiene hasta procesar todos los raster
						for(k=0;k<n_var;k++) //col
						{
							for(l=0;l<hayuae;l++)intgrid[l][k]=0;           //pone todos los valores de la matriz a 0 (no lo hace al generarla)
						}
					}
					if (esint == 0)                                     //matriz float para datos
					{
						printf("Genera matriz double en %i por %i\n", hayuae, n_var);
						flogrid = Make2DDoubleArray(hayuae, n_var);     //esta matriz se mantiene hasta procesar todos los raster
						for(k=0;k<n_var;k++) //col
						{
							for(l=0;l<hayuae;l++)flogrid[l][k]=0;           //pone todos los valores de la matriz a 0 (no lo hace al generarla)
						}
					}
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					//PARA CADA RASTER
					idxuae= 0;                                          //indicador para controlar las uaes seleccionadas
					for(j=0;j<n_uaefile;j++)                            // <----****----
					{
						tcal    = uefiles[j].uecalc;
						if (tcal == 1)                                  //solo aquellas consideradas
						{
							//LEE DATO RASTER
							selec_uae(j);
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< 
							//CALCULA CONTENIDO
							calcusu(idxuae);
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<   
							//LIBERA RASTER                              
							freeuaegrd();                               //libera memoria raster
							idxuae++;                                   //genera indice de columna para matriz de datos
						}
					}
					getnamout();                                        //construye nombre de salida
					write_vecuser();                                    //escribe datos de salida
					//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
					//LIBERA MATRIZ DATOS
					freedatgrd();  //intgrid - flogrid
				}
			}
		}
		if (tipmodo==3)
		{
			//PARA CADA DATO VECTORIAL
			for(i=0;i<n_varfile;i++)                                        
			{
				csvpros = varfiles[i].vaispro;
				if (csvpros == 1)                                       //si es procesado o no
				{
					//LEE DATO VECTORIAL
					selec_var(i);
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					//GENERA MATRIZ PARA ALMACENAR VALOR SEGUN NUMERO DE RASTER DISPONIBLES 
					printf("Genera matriz de datos %i cols por %i rows\n", hayuae, n_var);
					intgrid = Make2IntegerArray(hayuae, n_var);         //esta matriz se mantiene hasta procesar todos los raster
					for(k=0;k<n_var;k++) //col
					{
						for(l=0;l<hayuae;l++)intgrid[l][k]=0;           //pone todos los valores de la matriz a 0 (no lo hace al generarla)
					}
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					//CAPA RASTER
					idxuae= 0;
					for(j=0;j<n_uaefile;j++)                            // <----****----
					{
						//lectrua unidadese espaciales
						tcal    = uefiles[j].uecalc;
						//SI ES PROCESABLE
						if (tcal == 1)
						{
							//LEE DATO RASTER
							selec_uae(j);
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
							//CALCULA CONTENIDO
							calcusu(idxuae);
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<   
							//ESCRIBE Y LIBERA RASTER UAE   
							getnamout_rast();
							write_rastgen();                            //escribe raster generado                              
							freeuaegrd();                               //libera memoria
							idxuae++;                                   //genera indice de columna para matriz de datos
						}
					}
					getnamout();                                        //construye nombre de salida
					write_vecuser();                                    //escribe datos de salida
					//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
					//LIBERA MATRIZ DATOS
					freedatgrd();  //intgrid - flogrid
				}
			}
		}
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
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
