DESCRIPCIÓN

Herramientas de análisis espacial para cruce y transferencia de información entre capas geográficas junto a archivos de ejemplo

Todos han sido programados en Lenguaje C diseñados para trabajar en la plataforma GNU/Linux:

Para el análisis Raster-Raster, disponemos del script Analiza_RR_usu.c
Para el análisis Raster-Vector, disponemos del script Analiza_RV_user.c
Para el análisis Vector-Raster, disponemos del script Analiza_VR.c
Para el análisis Vector-Vector, disponemos del script Analiza_VV.c
Para el análisis Via-Vector, disponemos del script Analiza_Vi-Vector.c

Para documentar de forma más completa el software, su propósito y aplicabilidad se ha creado el sitio web https://sites.google.com/views/TerritorioyRiesgos

También está disponible en la misma web el reporte técnico donde se describe parte del software y su aplicabilidad dentro del Distrito Metropolitano de Quito:
H. Yepes y J.M. Marrero (2021). Sistema Exploratorio de Análisis Espacial del Distrito Metropolitano de Quito (SEAE-DMQ). Informe Técnico . Alcaldía de Quito, Ecuador. 

INSTALACIÓN

Descarga el archivo y descomprime en una carpeta. Comprueba que la ruta completa no tiene espacios o caracteres extraños

En la descompresión verás los archivos de ejemplo y el software con los archivos de configuración. En estos últimos, sobre todo en los de tipo general, deberás actualizar las rutas de acuerdo al lugar donde hayas instalado la información.

En la página web indicada, se dan instrucciones más amplias para poder utilizar los programas en otros sistemas operativos que no sean GNU/Linux.

EJECUCIÓN

Dentro del paquete software, hemos añadido un script de compilación (compila-C.sh) que permite realizar dicha acción en el resto de programas. Tienes que editarlo y adaptar las rutas. También tendrás que darle permisos de ejecución, para ello:

Desde terminal: chmod +x compila-C.sh

O bien, desde el explorador de carpetas, botón derecho, propiedades, permisos, ejecutar como programa

La línea de ejecución es: ./compila-C.sh

Para ejecutar los datos de ejemplo

Deberás abrir una terminar en cada una de las carpetas disponibles y escribir el nombre del programa (el ejecutable) junto al archivo de configuración general. En cada uno existen diferentes opciones.

Verifica los datos de entrada y salida en el Sistema de Información Geográfica

Recomendamos que abras los programas para que puedas leer/aprender lo que hacen. Tienen muchos comentarios añadidos que pueden servir de guía.

DEPENDENCIAS

Los programas en lenguaje C se han programado usando librerías estándar, por lo que no deberías tener problemas. Solo necesitas el compilador de C.

VERSIÓN ACTUAL

Version 1.0-2021-05-19

VERSIONES HISTÓRICAS

Actualizaciones y mejoras realizadas

Primera versión pública

Version 1.0-2021-05-19