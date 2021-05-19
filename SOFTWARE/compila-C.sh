#!/bin/bash

#Compila en cadena todos los programas C. Hay que actualizar la ruta en cada caso
echo "compilando raster-raster"
path=$HOME"/GIS/HAZARD_MODELS/REPO-Y-HERRAMIENTAS/REPO/TYR/SOFTWARE/RR/"
cd $path
gcc Analiza_RR_usu.c -o Analiza_RR_usu
echo "------------------"

echo "compilando raster-vector"
path=$HOME"/GIS/HAZARD_MODELS/REPO-Y-HERRAMIENTAS/REPO/TYR/SOFTWARE/RV/"
cd $path
gcc Analiza_RV_user.c -o Analiza_RV_user
echo "------------------"

echo "compilando vector-raster"
path=$HOME"/GIS/HAZARD_MODELS/REPO-Y-HERRAMIENTAS/REPO/TYR/SOFTWARE/VR/"
cd $path
gcc Analiza_VR.c -o Analiza_VR
echo "------------------"

echo "compilando vector-vector"
path=$HOME"/GIS/HAZARD_MODELS/REPO-Y-HERRAMIENTAS/REPO/TYR/SOFTWARE/VV/"
cd $path
gcc Analiza_VV.c -o Analiza_VV -lm
echo "------------------"

echo "compilando via-vector"
path=$HOME"/GIS/HAZARD_MODELS/REPO-Y-HERRAMIENTAS/REPO/TYR/SOFTWARE/ViV/"
cd $path
gcc Analiza_Vi-Vector.c -o Analiza_Vi-Vector -lm
echo "------------------"


