# Compilación de Windows

El flujo `.github/workflows/windows.yml` compila con MSYS2 UCRT64 en un runner Windows. Produce `Biblioteca-Windows.exe`, un único archivo que contiene la aplicación, las bibliotecas de terceros requeridas y el directorio `assets/`. Al ejecutarse, extrae esos archivos a una carpeta temporal, abre la aplicación y espera a que se cierre. Requiere un controlador con OpenGL 3.3; Windows aporta OpenGL mediante el controlador instalado en el equipo.

## Compilación local

En una terminal MSYS2 UCRT64 con GCC, pkg-config, SDL2, Cairo, Fontconfig y GLEW instalados:

```sh
make -f Makefile.windows -j2
make -f Makefile.windows test
```

La salida de esta fase es `biblioteca.exe`, que requiere `assets/` y las DLL correspondientes en el mismo directorio o en la ruta de carga de Windows. El flujo automatizado reúne esos componentes y usa NSIS para producir el archivo de distribución autónomo.

## GitHub Actions

En la pestaña Actions, selecciona **Biblioteca para Windows** y ejecuta **Run workflow**. Al terminar, descarga el artefacto `Biblioteca-Windows`. También se puede iniciar desde una terminal autenticada:

```sh
gh workflow run windows.yml --ref windows-build
```

El archivo distribuido no requiere un instalador ni un directorio de recursos separado. Puede tardar unos segundos en abrirse mientras extrae sus archivos temporales. No se ha validado visualmente en un equipo Windows hasta ejecutar el flujo y abrir el resultado.
