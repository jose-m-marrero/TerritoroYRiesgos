#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#define PI 3.141592653
//Si existe desbordamiento de memoria, hay que revisar el numero de elementos de entrada
#define UAECON      40   //NUMERO DE UNIDADES ESPACIALES EN CONFIG
#define VARCON      40   //NUMERO DE VARIABLES EN CONFIG
#define NUMUAE   30000   //NUM MAX DE UAES POR ARCHIVO
#define NUMCAT    1000   //NUM MAX DE CATEGORIAS EN VARIABLE
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
* Nombre script: Analiza_RR_usu
* Version: 1.0-2021-02-11
* Linea de compilacion: gcc Analiza_RR_usu.c -o Analiza_RR_usu -lm
* Linea de ejecucion:  ./Analiza_RR_usu configfilename.cfg
* Genera indicadores a partir de la superposicion de capas raster en UAE
* Cuenta espacio ocupado por cada categoria de raster en UAE
* Genera un archivo UAE-csv de salida por cada capa raster de entrada
* 
* Modo 1: UAE sistema - Variable usuario
* Modo 2: UAE usuario - Variable usuario
* Modo 3: UAE usu-gen - Variable usuario
* 
* IMPORTANTE: los ID de UAE no es necesario utilizar autonumericos correlativos, utilizamos el indice (bnum) o el valor (bnid) segun proceda
* IMPORTANTE: el ID de categoria esta referido al indice (primera columna), no al valor numerico de la segunda columna. 
* Este ultimo se utiliza en el titulo de salida de la matriz pero no en los cruces
* 
*/ 


/**
* GLOBAL VARIABLES
*/
char dirhom[100], dir_out[OUT_SIZE];                                                               //workig directories
char diruaer[100], diruaec[100], dirvarr[100], dirvarc[100], dirout[100];                          //directorios intermedios
char inues[150], invgr[150];                                                                       //nombres de archivos de entrada
char name_incfg[IN_SIZE], name_uaefile[IN_SIZE], name_varfile[IN_SIZE];                            //rutas y nombres de archivos de entrada
char name_uaegrd[IN_SIZE], name_uescsv[IN_SIZE2], name_vargrd[IN_SIZE2], name_varcat[IN_SIZE2];    //rutas y nombres de archivos de entrada
char out_uaecsv[OUT_SIZE], out_uaegen[OUT_SIZE];                                                   //rutas y nombres de archivos de salida
int tipmodo;                                                                                       //parametros generales
int n_uaefile, n_uaecsv, n_varfile, n_vcate;                                                       //conteo de datos
char namcsv[100], namgrd[100];                                                                     //parametros infiles
int usokm, orden, vgorder, hayuae, hayvar;                                                         //parametros infiles
int txtsize, txtsizefin;                                                                           //funciones auxiliares
int vcol, vrow;                                                                                    //funciones auxiliares
double globx, globy;                                                                               //funciones auxiliares
int **varuser;                                                                                     //matriz para almacenar datos de superficie ocupada por cat

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
* GLOBAL VARIABLES - RASTER GRD
*/
char header2[8];
int nullval2, grdpros;
int totdemcel;
int nx2, ny2, nxc2, nyc2;
int demmax2, demmin2;
double xlo2, xhi2, ylo2, yhi2, zlo2, zhi2, res2x, res2y, invresx2, invresy2, res2xx, res2yy;
double xc2, yc2, dx2, dy2, ddx2, ddy2, dx22, dy22;
int **rast_var; 
float vres2x[VARCON], vres2y[VARCON];

/**
* ALMACENAMIENTO DE DATOS EN ESTRUCTURAS
*/

/*! ALMACENA DATOS INFILES CAPAS UAE */
typedef struct  
{
	int uecalc;           
	int uenulval;        //valor nulo
	int uemin;           //valor minimo raster
	int uemax;           //valor maximo raster
	int ueuskm;          //si se usam metros o km para indicadores longitudinales
	int ueorden;         //orden de entrada
	char uecsvfile[100];
	char uegrdfile[100];
	//--
	double uexcor;
	double ueycor;
	float  uerad;
	float  ueres;
}UES;
UES uefiles[UAECON];

/*! ALMACENA DATOS INFILES CAPAS RASTER */
typedef struct  
{
	int vespros;        //es procesado
	int vnulval;        //valor nulo
	int vminval;        //valor minimo raster
	int vmaxval;        //valor maximo raster
	int vorder;         //orden de entrada
	char vgrdfile[150];
	char vcatfile[150];
	char vtipo[100];
}VAR;
VAR vargrd[VARCON];

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
}BAR;
BAR stuaecsv[NUMUAE];

/*! ALMACENA DATOS RASTER-CSV CATEGORIA */
typedef struct  
{
	int catid;         //valor indice categoria
	int catid2;        //valor numerico categoria
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
void calc_vecindex(double vx, double vy, int tipo);
void get_coor(int col, int row, int tipo);
int getime(void);
void crea_mkdir(char *path, mode_t mode);

int crea_uae(int num);
int countdat(int uaeval, int catval);
int getnamout(int numvar, int numuae);

int freevargrd(void);
int freeuaegrd(void);
int freeuaecsv(void);
void inistrucue(void);
void inistvarcat(void);

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
		fscanf(file,"%s", texto);
		fscanf(file,"%s %s", texto, diruaer);
		fscanf(file,"%s %s", texto, diruaec);
		fscanf(file,"%s %s", texto, dirvarr);
		fscanf(file,"%s %s", texto, dirvarc);
		fscanf(file,"%s %s", texto, dirout);
		fscanf(file,"%s", texto);
		fscanf(file,"%s %s", texto, inues);
		fscanf(file,"%s %s", texto, invgr);
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
	printf("DIR_IN UAE GRD %s\n", diruaer);
	printf("DIR_IN UAE CSV %s\n", diruaec);
	printf("DIR_INVAR GRD %s\n", dirvarr);
	printf("DIR_INVAR CATE %s\n", dirvarc);
	printf("DIR_OUTGEN %s\n", dirout);
	printf("UAE_CONFIG_FILE %s\n", inues);
	printf("VAR_CONFIG_FILE %s\n", invgr);
	return 1;		
}		

/*! LEYENDO CONFIG UAE INFILES */
int read_ueinfile(void)
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
	}
	else
	{
		fgets(texto,256,file); 
		printf("%s\n",texto);  //imprime cabecera
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

/*! LEYENDO CONFIG UAE GENERADOR INFILES */
int read_uaefilegen(void)
{
FILE *file;
//int i, j;
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

/*! LEYENDO CONFIG RASTER-GRD-CSV INFILES */
int read_vargrdfile(void)
{
FILE *file;
int i;
int tnul, tmin, tmax, torder, tpros;
char texto[256], tnamvar[100],  tnamcat[100], tvartip[100];	
	//LEE ARCHIVO INFILE CON LISTADO DE DATOS RASTER
	i = 0;
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
	}
	else
	{
		fgets(texto,256,file); 
		while (fscanf(file,"%i %i %i %i %i %s %s %s" , 
					&tpros,
					&tnul,
					&tmin,
					&tmax,
					&torder,
					tnamvar,
					tnamcat,
					tvartip
					) == 8)
		{
			vargrd[i].vespros = tpros;
			vargrd[i].vnulval = tnul;
			vargrd[i].vminval = tmin;
			vargrd[i].vmaxval = tmax;
			vargrd[i].vorder  = torder;
			sprintf(vargrd[i].vgrdfile,"%s", tnamvar);
			sprintf(vargrd[i].vcatfile,"%s", tnamcat);
			sprintf(vargrd[i].vtipo,"%s", tvartip);
			printf("%i %i %i %i %s %s %s\n", tpros, tnul, tmin, tmax, tnamvar, tnamcat, tvartip);
			if (tpros == 1) hayvar +=1;
			i++;
		}	
	}
	if (i == 0)
	{
		printf("Atencion error de lectura en variables grd\n");
		exit(0);
	}
	n_varfile = i; //numero de variables raster
	fclose(file);
	printf("Numero total de variables grd = %i\n", i);
	printf("Numero total de variables grd a procesar = %i\n", hayvar);
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

/*! LEYENDO DATOS UAE IN CSV FORMAT */	
int read_uecsv(void)
{
FILE *file;
int i, tid;
char texto[256], txtid[20];
double txcoor, tycoor, tarea, tperi;
	//LEE ARCHIVO UAE-CSV, SOLO MODOS 1 Y 2
	i     =0;
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
			stuaecsv[i].bnum   = i;                     //valor indice 
			stuaecsv[i].bnid   = tid;                   //valor en raster
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
	* Creando matrices de memoria
	*/
	//rast_uae = Make2DDoubleArray (nx, ny);                         
	rast_uae = Make2IntegerArray(nx, ny);
	for(j=0;j<ny;j++) //row
	{
		for(i=0;i<nx;i++)
		{                      
			fread(&datofl,sizeof(float),1,in);                         /*!< lee dato a dato por cada linea */
			//printf("%f %i %i\n", datofl[0], demmax, demmin);  
			if((datofl[0] > demmax) || (datofl[0] <= demmin))          /*!< si el valor esta fuera de limite, se asigna valor nulo */
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
double perim, area;
    //CREA UAE-RASTER A PARTIR DE PARAMENTROS, SOLO MODO 3
	printf("Genera raster a partir de punto\n");
	xlo  = uefiles[num].uexcor - uefiles[num].uerad;                  //coordenada x minima
	ylo  = uefiles[num].ueycor - uefiles[num].uerad;                  //coordenada y minima
	xhi  = uefiles[num].uexcor + uefiles[num].uerad;                  //coordenada x maxima
	yhi  = uefiles[num].ueycor + uefiles[num].uerad;                  //coordenada y maxima
	resx = uefiles[num].ueres;                                        //resolucion en x
	resy = uefiles[num].ueres;                                        //resolucion en y
	nx   = (xlo + (2* uefiles[num].uerad) - xlo) / resx;              //numero de celdas en x
	ny   = (ylo + (2* uefiles[num].uerad) - ylo) / resy;              //numero de celdas en y
	area = (nx * resx) * (ny * resy);
	perim= (nx * resx) * 2 + (ny * resy) * 2;
	// imprime valores de cabecera
	printf("colx  = %5i -- rowy = %5i\n", nx, ny);
	printf("Num total celdas %i\n", nx * ny);
	printf("xmin = %f -- ymin = %f\n", xlo, ylo);
	printf("xmax = %f -- ymax = %f\n", xhi, yhi);
	printf("DEM resolution in resx = %f\n", resx);
    printf("DEM resolution in resy = %f\n", resy);
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    //adiciona datos descriptivos de UAE para archivo de salida
    stuaecsv[0].bnum = 0;
	stuaecsv[0].bnid = 1;       //barrio id 
	stuaecsv[0].bperim = perim;
	stuaecsv[0].barea  = area; 
	stuaecsv[0].bxcoor = uefiles[num].uexcor;
	stuaecsv[0].bycoor = uefiles[num].ueycor;
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

/*! LEYENDO DATOS RASTER-GRD EN BINARY FORMAT */
int read_vargrd(void)
{
FILE *in;
char cabecera[8];
short int datoin[4];
double datodo[8];
float datofl[4];
int i, j, contnul, contgod;
	//LEE RASTER SUPERIOR
	printf("\n***Lectura archivo capa raster-grd, %s***\n", name_vargrd);
	if((in=fopen(name_vargrd,"rb"))==NULL)
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
	res2x=0;
	res2y=0;
	fread(&cabecera,4,1,in);
	fread(&datoin,2*sizeof(short int),1, in);
	fread(&datodo,6*sizeof(double),   1, in);
	cabecera[4] = 0;
	nx2  = datoin[0];
	ny2  = datoin[1];
	xlo2 = datodo[0];
	xhi2 = datodo[1]; 
	ylo2 = datodo[2];
	yhi2 = datodo[3];
	zlo2 = datodo[4];
	zhi2 = datodo[5];
	res2x = (xhi2-xlo2)/(double)(nx2-1);
	res2y = (yhi2-ylo2)/(double)(ny2-1);
	contnul=0;
	contgod=0;
	for(j=0;j<6;j++)printf("%lf\n",datodo[j]);
	/**
	* Creando matrices de memoria
	*/
	//rast_var = Make2DDoubleArray (nx2, ny2);                          
	rast_var = Make2IntegerArray (nx2, ny2);
	for(j=0;j<ny2;j++) //row
	{
		for(i=0;i<nx2;i++)
		{                        
			fread(&datofl,sizeof(float),1,in);                          /*!< lee dato a dato por cada linea */
			if((datofl[0] > demmax2) || (datofl[0] <= demmin2))         /*!< si el valor esta fuera de limite, se asigna valor nulo */
			{
				datofl[0] = nullval2;                                
				contnul+=1;
			}
			else  contgod +=1;
			rast_var[i][j]  = (int)datofl[0]; 
			//rast_var[i][j]  = (double)datofl[0]; 			
		}                                          
	}    
	fclose(in);
	/**
	* Calcula resoluciones y escalas
	*/
	invresx2 = 1/res2x;
	invresy2 = 1/res2y;
	res2xx = res2x*res2x;
	res2yy = res2y*res2y;
	totdemcel = nx2 * ny2;
	/**
	* Imprime resultados
	*/
	printf("Cabecera archivo RASTER-2\n");
	printf("RASTER tipo: %s\n", cabecera);
    printf("colx  = %5i -- rowy = %5i\n", nx2, ny2);
	printf("xmin = %f -- ymin = %f\n", xlo2, ylo2);
	printf("xmax = %f -- ymax = %f\n", xhi2, yhi2);
	printf("zlo = %10.4f -- zhi = %10.4f\n", zlo2, zhi2);
	printf("RASTER-2 resolucion en x = %f\n", res2x);
	printf("RASTER-2 resolucion en y = %f\n", res2y);
	printf("RASTER-2 invresx = %f and invresy = %f\n", invresx2, invresy2);
	printf("RASTER-2 resx2 = %f and resy2 = %f\n", res2xx, res2yy);
	printf("---------------------------------\n");
	printf("Total celdas en RASTER-2 = %i\n", totdemcel);
	printf("Area de trabajo %lf\n", contgod * res2x * res2y);
	printf("Total celdas en nulas en RASTER = %i\n", contnul);
	printf("Area nula %lf\n", contnul * res2x * res2y);
	printf("Final de lectura de RASTER-2\n");  
	printf("---------------------------------\n\n");
	return 1;
}

/*! LEYENDO DATOS RASTER CATEGORIAS-CSV */
int read_varcate(void)
{
FILE *file;
int i, tid, tid2;
char texto[256], tshort[50], texpan[100];
	//LECTURA DE ARCHIVO DE DICCIONARIO DE CATEGORIAS DEL RASTER SUPERIOR
	i      =0;
	n_vcate=0;
	printf("\n***Lectura archivo categorias raster, %s***\n", name_varcat);
	if ((file = fopen(name_varcat,"rt"))== NULL)
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
		while (fscanf(file,"%i %i %s %s" , 
					&tid,
					&tid2,
					tshort,
					texpan         
					) == 4)
	
		{	
			categoria[i].catid  = tid;
			categoria[i].catid2 = tid2;
			sprintf(categoria[i].catshor,"%s", tshort);
			sprintf(categoria[i].catexp,"%s", texpan);
			if (i<20)printf("%i %i :: %s - %s\n", tid, tid2, tshort, texpan); //imprime las primeras 20
			i++;
		}	
	}
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

/*! GENERADOR DE MATRIZ DINAMICA EN ENTERNOS */
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
void calc_vecindex(double vx, double vy, int tipo)
{
double difx, dify, demx, demy;
	if (tipo == 1)	
	{
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
	if (tipo == 2)	
	{
		difx = vx - xlo2; 
		vcol = (int)(difx / res2x);
		demx = (vcol * res2x) + xlo2; 
		if ((vx - demx) > (res2x/2))vcol++; 
		dify = vy - ylo2;
		vrow = (int)(dify / res2y);
		demy = (vrow * res2y) + ylo2;
		if ( (vy - demy) > (res2y/2))vrow++;
	}	

}

/*! CALCULA X E Y A PARTIR DE LOS INDICES DEL RASTER */
void get_coor(int col, int row, int tipo)
{
	globx = 0;
	globy = 0;
	if (tipo == 1)	
	{
		globx = xlo + (col * resx);
		globy = ylo + (row * resy);
	}	
	if (tipo == 2)	
	{
		globx = xlo2 + (col * res2x);
		globy = ylo2 + (row * res2y);
	}

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
	if (tipmodo<=2)      //ue en sistema -- UAE usuario 
	{
		//CAPTURA VALORES DE UAE DESDE INFILE
		nullval = uefiles[num].uenulval;                       //valor nulo a utilizar
		demmin  = uefiles[num].uemin;                          //valor grd minimo
		demmax  = uefiles[num].uemax;                          //valor grd maximo
		usokm   = uefiles[num].ueuskm;                         //segun area ue se convierte a km o se mantiene en m
		orden   = uefiles[num].ueorden;                        //orden en el que aparece la UAE
		sprintf(namgrd,"%s", uefiles[num].uegrdfile);          //nombre corto uae raster file
		sprintf(namcsv,"%s", uefiles[num].uecsvfile);          //nombre corto uae csv file
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
		printf("Calucla UE %i -- orden %i nul %i min %i max %i\n\n", num, orden, nullval, demmin, demmax);  
		read_uegrd();                                        //read unidad espacial grd		
		read_uecsv();                                        //read unidad espacial csv
	}
	//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	//GENERA VALORES DE UAE DESDE INFILE - GEN  
	if (tipmodo==3)  
	{   
		//CONSTRUYE UAE
		sprintf(namcsv,"%s", uefiles[num].uecsvfile);
		nullval=-9999;
		stuaecsv[num].bnum = 0;                                      //numero inicio en cero 
		stuaecsv[num].bnid = 1;                                      //UAE id 
		sprintf(stuaecsv[num].btxtid, "%s", uefiles[num].uecsvfile); //codigo 
		stuaecsv[num].bperim = (uefiles[num].uerad * 2) + (uefiles[num].uerad * 2) + (uefiles[num].uerad * 2) + (uefiles[num].uerad * 2);
		stuaecsv[num].barea  = (uefiles[num].uerad * 2) * (uefiles[num].uerad * 2);
		stuaecsv[num].bxcoor = uefiles[num].uexcor;
		stuaecsv[num].bycoor = uefiles[num].ueycor;
		n_uaecsv=1;    //en este modo solo hay un elemento a considerar en el archivo de salida
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		crea_uae(num);
	}
	return 1;
}

/*! CCONSTRUYE NOMBRE ARCHIVO DE SALIDA PARA UAE */
int getnamout(int numvar, int numuae)
{
int txtsize, txtsizefin;
char uaename[20];

	if (tipmodo == 1)sprintf(uaename, "RR_r-sis_r-usu_");
	if (tipmodo == 2)sprintf(uaename, "RR_r-usu_r-usu_");
	if (tipmodo == 3)sprintf(uaename, "RR_r-usu-gen_r-usu_");
	
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
	return 1;
}

/*! LEE DATOS RASTER-GRD */
int selec_var(int num, int numuae)
{
int i, j;
char namvari[150], namcate[150], tvartip[100];
	
	//CONTENIDO ARCHIVO INFILE...
	nullval2 = vargrd[num].vnulval;                   //valor nulo a utilizar
	demmin2  = vargrd[num].vminval;                   //valor grd minimo
	demmax2  = vargrd[num].vmaxval;                   //valor grd maximo
	vgorder  = vargrd[num].vorder;                    //orden de lectura y asignacion de variable
	sprintf(namvari,"%s", vargrd[num].vgrdfile);      //mombre corto archivo variable raster
	sprintf(namcate,"%s", vargrd[num].vcatfile);      //nombre corto archivo variable categoria
	sprintf(tvartip,"%s", vargrd[num].vtipo);         //tipo - grupo variable
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//ASIGNA NOMBRE ARCHIVO VARIABLE CATEGORIA
	txtsize = calc_size3(dirhom, dirvarc, namcate);
	char catas_ini[txtsize];
	sprintf(catas_ini,"%s%s%s", dirhom, dirvarc, namcate);
	if(txtsize < IN_SIZE2) sprintf(name_varcat,"%s", catas_ini);
	else 
	{
		printf("Se ha excedido name_varcat en %i\n", txtsize);
		exit(0);
	}
	//ASIGNA NOMBRE ARCHIVO VARIABLE RASTER
	txtsize = calc_size3(dirhom, dirvarr, namvari);
	char grd_ini[txtsize];
	sprintf(grd_ini,"%s%s%s", dirhom, dirvarr, namvari);
	if(txtsize < IN_SIZE2) sprintf(name_vargrd,"%s", grd_ini);
	else 
	{
		printf("Se ha excedido name_vargrd en %i\n", txtsize);
		exit(0);
	}
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//LEE ARCHIVOS
	printf("Calucla UE %i en VAR-GRD %i %s -- nul %i min %i max %i\n\n", numuae, num, tvartip, nullval2, demmin2, demmax2);
	read_vargrd();                                   //Lee archivo variable-raster
	read_varcate();                                  //lee archivo variable-raster-categoria
	//ALMACENA RESOLUCION
	vres2x[num] = res2x;                             //almacena resolucion en x
	vres2y[num] = res2y;                             //almacena resolucion en y 
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//GENERA MATRIZ PARA ALMACENAR LOS DATOS
	printf("***************************************************\n");
	printf("Crea matriz para variables de %i colum por %i filas\n", n_vcate, n_uaecsv); 
	printf("***************************************************\n"); 
	varuser = Make2IntegerArray(n_vcate, n_uaecsv);   //La matriz se crea segun numero de categorias disponibles
	for(i=0;i<n_uaecsv;i++) //col
	{
		for(j=0;j<n_vcate;j++)varuser[j][i]=0;        //pone todos los valores de la matriz a 0 (no lo hace al generarla)
	}
	return 1;
}

/*! CALCULA INDICES DE UAE Y CATEGORIA Y SUMA VALORES */
int countdat(int uaeval, int catval)
{
int j, uaeid, uaeidx;
int catid, catidx;

	//CALCULA INDICE CATEGORIA
	catidx = -1;  
	for (j=0;j<n_vcate;j++)                               //buscamos indice categoria, posicion que ocupa empenzando en 0
	{
		catid = categoria[j].catid2;
		if (catval == catid)
		{
			catidx = categoria[j].catid; //captura indice de categoria
			break;
		}
	}
	if (catidx > n_vcate || catidx == -1)
	{
		printf("Atencion hay un problema en la categorias, revise el archivo\n");
		if (catidx  > -1)printf("Tiene que adicionar campos, pasar de %i a %i\n", n_vcate, catidx); 
		if (catidx == -1)printf("No se econtro la categoria %i en el archivo de categorias\n", catval); 
		exit(0);
	}
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//CALCULA INDICE UAE
	if (tipmodo<=2) 
	{
		uaeidx = -1;
		for (j=0;j<n_uaecsv;j++)                             //buscamos indice uae en csv, posicion que ocupa empenzando en 0
		{
			uaeid = stuaecsv[j].bnid;
			if (uaeid == uaeval) 
			{
				uaeidx = stuaecsv[j].bnum;                   //captura indice de uae en csv
				break;
			}
		}
		if (uaeidx > n_uaecsv || uaeidx == -1)
		{
			printf("Atencion hay un problema en la uae, revise el archivo\n");
			if (uaeidx  > -1)printf("Revisar numero de uaes en csv, hay %i y se busca la %i\n", n_uaecsv, uaeval); 
			if (uaeidx == -1)printf("No se econtro la uae %i en el archivo de uae\n", uaeval); 
			exit(0);
		}
	}
	if (tipmodo==3) uaeidx = 0;
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	//ASIGNAR EL VALOR A LA MATRIZ
	//printf("%i :: %i\n", catidx, uaeidx);
	varuser[catidx][uaeidx] +=1;                //necesita posicion de categoria y posicion de elemento UAE
	return 1;
}

/*! LIBERA MEMORIA DE UAE-RASTER */
int freeuaegrd(void)
{
int k;
	printf("Liberando variable UAE\n");
	for(k=0;k<nx;k++)free(rast_uae[k]);
	free(rast_uae);
	rast_uae = NULL;
	return 1;
}

/*! LIBERA MEMORIA DE CAPA RASTER */
int freevargrd(void)
{
int k;
	printf("Liberando variable raster\n");
	for(k=0;k<nx2;k++)free(rast_var[k]);
	free(rast_var);
	rast_var = NULL;
	return 1;
}

/*! LIBERA MEMORIA DE UAE-CSV */
int freeuaecsv(void)
{
int k;
	printf("Liberando variable UAE\n");
	for(k=0;k<nx;k++)free(varuser[k]);
	free(varuser);
	varuser = NULL;
	return 1;
}

/*! INICIALIZA ESTRUCTURA DE DATOS UAE */
void inistrucue(void)
{
int i;	
	for(i=0;i<NUMUAE;i++)  
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

/*! INICIALIZA ESTRUCTURA RASTER-CATEGORIAS */
void inistvarcat(void)
{
int i;	
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

/*! CALCULA DATOS CAPA RASTER EN CAPA UAE */
int calc_dat(int ue)
{
int i, j, p, out, nul, fin, in;
int getval2, getval1;	
	out=0;
	fin=0;
	nul=0;
	in =0;
	p = 100;
	printf("Calcula en UAE %i-%i para raster %i\n", ue, orden, vgorder);
	//PARA CADA CELDA CAPA RASTER
	for(j=0;j<ny2;j++) //col
	{
		for(i=0;i<nx2;i++)
		{  
			getval2 = rast_var[i][j];                                   //captura valor celda en capa raster
			if (getval2 != nullval2)                                    //si es distinto a valor nulo
			{
				get_coor(i, j, 2);                                      //calculamos coordenadas a partir de indices en capa raster
				if(globx > xlo && globx < xhi && globy > ylo && globy < yhi) //si esta en raster 1
				{
					//OBTENEMOS FILA Y COLUMNA DEL RASTER
					calc_vecindex(globx, globy, 1);                     //calculamos indices-celda en uae raster
					//SI LOS VALORES ESTAN EN RASTER UAE
					if(vcol < nx && vrow < ny)
					{
						//OBTENERMOS VALOR DE LA UAE DONDE CONCIDE LA VARIABLE RASTER
						getval1 = rast_uae[vcol][vrow];                 //calculamos valor celda en uae raster
						if (getval1 != nullval)                         //si es distinto a valor nulo
						{
							//printf("ok\n");
							fin = countdat(getval1, getval2);           //transfiere valores a uae csv - variable usuario
							if (fin == 1)in+=1;                         //cuenta valores dentro de UAE
						}
						else nul+=1;                                    //cuenta valores dentro UAE pero nulos
					}	
					else out+=1;                                        //cuenta casos fuera de raster	
				}
				else out+=1;                                            //cuenta casos fuera de raster	
			}	
		}
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//imprime linea de control informativa durante el procesado
		if (j == p)
		{
			printf(" Contador en %i\n", j);
			p = p * 2;
		}	
	}
	printf("Finalizado Cruce en %i-%i\n", ue, orden);
	printf("Elementos dentro UAE: %i\n", in);	
	printf("Elementos fuera  UAE: %i\n", nul);	
	printf("Elementos Fuera de raster: %i\n", out);	
	printf("***********************\n\n");
	return 1;		
}	

//**********************************************************************
//ESCRIBE---------------------------------------------------------------
//**********************************************************************

/*! ESCRIBE DATOS UAE-CSV CON INDICADORES */
int write_uaecsv(int numvar)
{	
FILE *file;
int i,j, concat;
float txres, tyres;
float porcen, superf;

	printf("\nEscritura de uae-csv %s\n", out_uaecsv);
    if((file = fopen(out_uaecsv,"wt"))== NULL)
    {
        printf("ERROR %s\n",out_uaecsv);
        printf("ATENCION: LA RUTA DE SALIDA DEL ARCHIVO NO ES CORRECTA\n");
        exit(0);
    }
    else
    {        
		if (usokm  > 1)printf("Archivo uae small, en m para algunos indicadores lineales\n");
		if (usokm == 1)printf("Archivo uae grande, en km para algunos indicadores lineales\n");
		//CAPTURA RESOLUCION
		txres = vres2x[numvar];
		tyres = vres2y[numvar];
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//EVALUA LONGITUD PALABRAS PARA CABECERA
		concat=0;
		for(i=0;i<n_vcate;i++)
		{
			if (strlen(categoria[i].catshor) > 15)
			concat = 1;
			break;
		}
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//ESCRIBE CABECERA
		fprintf(file,"NUM ID CODE ARE PER X Y ");
		for(i=0;i<n_vcate;i++)
		{
			if (concat==0)  //si los nombres cortos son manejabes
			{
				if (i  < n_vcate -1) fprintf(file,"%s %sp ", categoria[i].catshor, categoria[i].catshor);  
				if (i == n_vcate -1) fprintf(file,"%s %sp\n", categoria[i].catshor, categoria[i].catshor);
			}
			if (concat==1)  //caso contrario se usan numeros
			{
				if (i  < n_vcate -1) fprintf(file,"CAT-%i CAT-%isp", categoria[i].catid2, categoria[i].catid2); 
				if (i == n_vcate -1) fprintf(file,"CAT-%i CAT-%i\n", categoria[i].catid2, categoria[i].catid2);
			}
		}
		//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		//ESCRIBE DATOS
		for(j=0;j<n_uaecsv;j++) //col
		{
			//DATOS INICIALES DE UAE
			fprintf(file,"%i %i %s %lf %lf %lf %lf ",
				stuaecsv[j].bnum,
				stuaecsv[j].bnid,       //barrio id 
				stuaecsv[j].btxtid,      //codigo id
				stuaecsv[j].barea,      //expresada en metros
				stuaecsv[j].bperim,
				stuaecsv[j].bxcoor,
				stuaecsv[j].bycoor);
			//INDICADORES
			for(i=0;i<n_vcate;i++)
			{  
				superf = varuser[i][j] * txres * tyres;                 //expresada en metros
				porcen = (superf * 100) / stuaecsv[j].barea;
				if (i  < n_vcate -1) fprintf(file,"%.2f %.2f ", superf, porcen); 
				if (i == n_vcate -1) fprintf(file,"%.2f %.2f\n",superf, porcen); 
			}
		}
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
int i, j;	
int call, tcal;

	printf("Inicio del proceso de calculo\n");
	getime();
	//------
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
		sprintf(name_uaefile, "%s", inues);        //nombre uae infiles config file
		sprintf(name_varfile, "%s", invgr);        //nombre raster infiles config file
		printf("%s\n", name_uaefile); 
		printf("%s\n", name_varfile);
		if (tipmodo<=2) read_ueinfile();           //lee archivo infiles con UAEs a procesar
		if (tipmodo==3) read_uaefilegen();         //lee archivo infiles para crear UAEs
		read_vargrdfile();                         //lee archivo infiles de capas raster a procesar		
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
					//PARA CADA RASTER
					for(i=0;i<n_varfile;i++)                            // <----****----
					{
						grdpros  = vargrd[i].vespros;                   //si el raster es procesado
						if (grdpros == 1)
						{
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
							//LEE DATO RASTER
							selec_var(i, j);
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
							//CALCULA DATOS
							calc_dat(j);
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
							//GENERA NOMBRES DE ARCHIVO DE SALIDA
							getnamout(vgorder, j);	
							write_uaecsv(i);                            //escribe archivo UAE-csv de salida
							//--
							freevargrd();                               //libera matriz capa raster
							inistvarcat();                              //libera estrcutura raster-cat
						}
					}
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					//LIBERA MEMORIA
					inistrucue();                                       //libera uae csv
					freeuaegrd();                                       //libera uae raster
					printf("*********************************\n");
					printf("*********************************\n");
					printf("*********************************\n");
					printf("*********************************\n\n");
				}
			}
		}
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		if (tipmodo==3) 
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
					//PARA CADA RASTER
					for(i=0;i<n_varfile;i++)                            // <----****----
					{
						grdpros  = vargrd[i].vespros;                   //si la variable es procesada
						if (grdpros == 1)
						{
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
							//LEE DATO RASTER
							selec_var(i, j);
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
							//CALCULA DATOS
							calc_dat(j);                      
							//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
							//GENERA NOMBRES DE ARCHIVO DE SALIDA
							getnamout(vgorder, j);	
							write_uaecsv(i);                            //escribe archivo UAE-csv de salida
							write_uaerast();                            //escribe archivo UAE raster de salida
							//--
							freevargrd();                               //libera matriz capa raster
							inistvarcat();                              //libera estrcutura raster-cat 
						}
					}
					//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
					//LIBERA MEMORIA
					inistrucue();                                       //libera uae csv
					freeuaegrd();                                       //libera uae raster 
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


