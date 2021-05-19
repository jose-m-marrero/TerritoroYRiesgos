#!/bin/bash

#Compila en cadena todos los programas C. Hay que actualizar la ruta en cada caso
echo "copiando raster-raster"
pathini=$HOME"/GIS/HAZARD_MODELS/SOFTWARE/QUITO_DMQ/QUITO_SEAE-DMQ/Raster-raster/Analiza_RR_usu.c"
pathdes=$HOME"/GIS/HAZARD_MODELS/REPO-Y-HERRAMIENTAS/REPO/TYR/SOFTWARE/RR/Analiza_RR_usu.c"
cp $pathini $pathdes
echo "------------------"

echo "copiando raster-vector"
pathini=$HOME"/GIS/HAZARD_MODELS/SOFTWARE/QUITO_DMQ/QUITO_SEAE-DMQ/Raster-vector/Raster/Analiza_RV_user.c"
pathdes=$HOME"/GIS/HAZARD_MODELS/REPO-Y-HERRAMIENTAS/REPO/TYR/SOFTWARE/RV/Analiza_RV_user.c"
cp $pathini $pathdes
echo "------------------"

echo "copiando vector-raster"
pathini=$HOME"/GIS/HAZARD_MODELS/SOFTWARE/QUITO_DMQ/QUITO_SEAE-DMQ/Vector-raster/Vector/Analiza_VR.c"
pathdes=$HOME"/GIS/HAZARD_MODELS/REPO-Y-HERRAMIENTAS/REPO/TYR/SOFTWARE/VR/Analiza_VR.c"
cp $pathini $pathdes
echo "------------------"

echo "copiando vector-vector"
pathini=$HOME"/GIS/HAZARD_MODELS/SOFTWARE/QUITO_DMQ/QUITO_SEAE-DMQ/Vector-vector/Analiza_VV.c"
pathdes=$HOME"/GIS/HAZARD_MODELS/REPO-Y-HERRAMIENTAS/REPO/TYR/SOFTWARE/VV/Analiza_VV.c"
cp $pathini $pathdes
echo "------------------"

echo "copiando via-vector"
pathini=$HOME"/GIS/HAZARD_MODELS/SOFTWARE/QUITO_DMQ/QUITO_SEAE-DMQ/Viario-vector/Analiza_Vi-Vector.c"
pathdes=$HOME"/GIS/HAZARD_MODELS/REPO-Y-HERRAMIENTAS/REPO/TYR/SOFTWARE/ViV/Analiza_Vi-Vector.c"
cp $pathini $pathdes
echo "------------------"


