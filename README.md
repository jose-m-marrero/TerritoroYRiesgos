# Descripción

Herramientas de análisis espacial para cruce y transferencia de información entre capas geográficas. También se proveen archivos de ejemplo para testear el software y entender la estructura de almacenamiento de datos.

Todos han sido programados en Lenguaje C diseñados para trabajar en la plataforma GNU/Linux:

1) Para el análisis Raster-Raster, disponemos del script Analiza_RR_usu.c 

2) Para el análisis Raster-Vector, disponemos del script Analiza_RV_user.c

3) Para el análisis Vector-Raster, disponemos del script Analiza_VR.c

4) Para el análisis Vector-Vector, disponemos del script Analiza_VV.c

5) Para el análisis Via-Vector, disponemos del script Analiza_Vi-Vector.c

# Documentación

Para documentar de forma más completa el software, su propósito y aplicabilidad se ha creado el sitio web https://sites.google.com/views/territorio-y-riesgos.
También está disponible en la misma web el reporte técnico donde se describe parte del software y su aplicabilidad dentro del Distrito Metropolitano de Quito:

Para citar el software: 

Marrero, J.M y Yepes, H. (Fecha de acceso). Territorio y Riesgos. https://sites.google.com/views/territorio-y-riesgos. En ESpañol.

H. Yepes y J.M. Marrero (2021). Sistema Exploratorio de Análisis Espacial del Distrito Metropolitano de Quito (SEAE-DMQ). Informe Técnico. Alcaldía de Quito, Ecuador. 

# Instalación

Descarga el archivo y descomprime en una carpeta. Comprueba que la ruta completa no tiene espacios o caracteres extraños

En la descompresión verás los archivos de ejemplo y el software con los archivos de configuración. En estos últimos, sobre todo en los de tipo general, deberás actualizar las rutas de acuerdo al lugar donde hayas instalado la información.

En la página web indicada, se dan instrucciones más amplias para poder utilizar los programas en otros sistemas operativos que no sean GNU/Linux.

# Ejecución

Dentro del paquete software, hemos añadido un script de compilación (compila-C.sh) que permite realizar dicha acción en el resto de programas (crear los archivos de ejecución similares a los .exe). Tienes que editarlo y adaptar las rutas. También tendrás que darle permisos de ejecución, para ello:

Desde terminal: chmod +x compila-C.sh

O bien, desde el explorador de carpetas, botón derecho, propiedades, permisos, ejecutar como programa

Para ejecutarlo desde terminal tienes que escribir: ./compila-C.sh

Una vez compilados los programas en lenguaje C, para ejecutar los datos de ejemplo:

Deberás abrir un terminal en cada una de las carpetas disponibles y escribir el nombre del programa (el ejecutable) junto al archivo de configuración general. En cada uno existen diferentes opciones.

Verifica los datos de entrada y salida en el Sistema de Información Geográfica

Recomendamos que abras los programas para que puedas leer/aprender lo que hacen. Tienen muchos comentarios añadidos que pueden servir de guía.

# Dependencias

Los programas en lenguaje C se han programado usando librerías estándar, por lo que no deberías tener problemas. Solo necesitas el compilador de C.

# Versión y actualizaciones

Version 1.0-2021-05-19

VERSIONES HISTÓRICAS

