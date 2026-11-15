# Biblioteca 3D

La revisión actual suaviza el catálogo completo de pop-ups: carcasas y bases
usan perfiles redondeados, tubos y esferas con mayor resolución, y el
generador elimina triángulos degenerados antes de exportar. La ruta de
portabilidad a iPadOS y el orden de migración a Metal están documentados en
`PORTING_IPAD.md`.

Biblioteca de escritorio en C++20, OpenGL 3.3 y SDL2. Hay dos salas unidas por una puerta en el costado derecho de la sala infantil. El botón de arriba a la izquierda te lleva a la otra sala. También puedes cruzar a pie. El catálogo muestra automáticamente los libros de la sala en la que te encuentras.

La sala infantil tiene 40 ejemplares interactivos distribuidos entre los estantes del frente, los laterales y el fondo. Los cuentos tienen texto de alto contraste y figuras 3D desplegables. Las ocho adaptaciones de los PDF aprovechan más detalles de cada relato.

La sala juvenil y adulta tiene cuatro faroles y 19 lecturas de ocho páginas cada una. Sus libros están mezclados con los demás ejemplares en varias estanterías. Cada doble página combina prosa, un esquema y una escena 3D de formato compacto para dejar espacio al texto.

Los pop-ups del catálogo original se basan en siete historias de ocho dobles páginas temáticas. Las páginas y sus cantos llevan colores propios de Marte, Europa, la Luna, Saturno, la Tierra, la órbita terrestre o el Sol.

## Lecturas adaptadas de PDFS.zip

| Documento | Sala infantil | Sala juvenil y adulta |
| --- | --- | --- |
| Cuentos del Módulo Lunar Apolo 11 | El pequeño salto de una gran águila | 1969-059C: crónica del Águila |
| Cuentos del Módulo Lunar Apolo 12 | La aventura del valiente Intrepid | 1969-099C: Océano de las Tormentas |
| Cuentos infantiles (luna) | El secreto del Halcón Dorado | Falcon: travesía por Hadley |
| MARTE | El robot que escuchó a Marte y Los gemelos del espacio | Latidos del planeta rojo y Gemelos de hierro |
| Documento (5) | Kepler y K2: en busca de nuevos mundos | |
| Documento (6) | Pioneer 10: el pequeño viajero | |
| Pioneer 10 | Pioneer 10: el viajero que soñaba | |

Cada doble página de un título tiene una escena propia. Apolo 11 pasa del módulo lunar a la recogida de muestras, los experimentos de superficie y el regreso. Pioneer 10 recorre el cinturón de asteroides y Júpiter antes de alejarse de la Tierra. InSight muestra el sismómetro y cómo se estudia el interior de Marte.

Al elegir un título desde el estante o desde el catálogo, la cámara camina hasta el punto de lectura y lo abre al llegar. El libro abierto ocupa más superficie de la ventana, con el lomo centrado y el canto inferior más cerca del lector. Los rovers hacen recorridos cortos, las antenas se orientan y los indicadores pulsan. Las figuras se aplanan sobre las hojas y siguen el pliegue durante la vuelta.

El libro del atril está junto a la puerta, del lado de la sala juvenil y adulta. El libro permanece abierto sobre la madera. Toca el atril, usa el botón **Atril** o presiona **F** para acercar la cámara. Lee las citas de Marte en la primera doble página y gira la hoja para consultar las de Luna. Toca una URL subrayada para abrirla.

El cohete despega cuando se asienta su doble página y sale por arriba de la pantalla
en 2.10 segundos. La estela se disipa durante los 2.10 segundos siguientes.
Volver a esa escena reinicia el despegue. M permite pausarlo y reanudarlo.

Para compilar en GitHub Actions desde Linux o Windows, consulta [iPadOS 27](PORTING_IPAD.md).

## Dirección visual

La sala infantil toma elementos de un taller de cartografía celeste. Alrededor de la fogata hay un mosaico turquesa con divisiones de latón. Los estantes llevan frisos de madera y cartas orbitales en relieve. La sala juvenil y adulta usa verde oliva, nogal y lámparas de vidrio opalino.

Las portadas se componen a partir de fotografías e imágenes de misión de la NASA, con créditos en [ASSETS_OPEN.md](ASSETS_OPEN.md). Los lomos y folios se componen por separado. Barlow Condensed distingue los títulos y Barlow acompaña el catálogo y las lecturas infantiles. Se incluyen las fuentes Barlow y su licencia OFL. Las escenas desplegables se construyen a partir de modelos OBJ editables con tintas y superficies de cartulina.

La dirección visual y los criterios de lectura están en [DESIGN.md](DESIGN.md). La revisión de contraste, mandos y movimiento tomó como referencia [Anti Slop](https://github.com/miqdadbadjuber/anti-slop).

## Abrir

Extrae el ZIP completo y ejecuta desde la carpeta `Biblioteca-3D`:

```bash
./ejecutable.sh
```

Mantén `assets/` junto al ejecutable. En Arch y CachyOS se usan `sdl2-compat`, `cairo`, `fontconfig`, `libglvnd` y un controlador compatible con OpenGL 3.3, como `mesa` para la Radeon integrada.

| Control | Acción |
| --- | --- |
| Mantener botón derecho y mover el mouse | Mirar alrededor |
| Tab | Activar o soltar la cámara con el mouse |
| W, A, S, D | Caminar hacia delante, a la izquierda, hacia atrás y a la derecha |
| Mayús con W, A, S o D | Correr |
| G o botón de la esquina superior izquierda | Caminar automáticamente a la otra sala |
| Rueda del mouse | Avanzar o retroceder por la sala. Con un libro abierto, ajustar el zoom |
| Clic en un lomo, o E mirando un lomo | Acercarse al estante y abrir el volumen |
| Flechas izquierda y derecha | Caminar hacia otra ala o pasar página |
| Arrastre con botón izquierdo sobre el libro abierto | Girar el libro |
| M o el botón de movimiento del lector | Pausar o reanudar el movimiento de los modelos. Se puede seguir pasando página |
| Esc | Soltar la cámara, cancelar un recorrido o cerrar el libro |
| / o K | Abrir el catálogo de la sala actual. Al elegir un título, la cámara camina hasta el estante |
| F, botón Atril o clic en el atril | Acercarse al libro de citas |
| Tab dentro del catálogo | Cambiar entre búsqueda, filtro de género y cierre |
| F2 | Mostrar u ocultar controles |
| F11 | Pantalla completa |

### iPad (horizontal)

La versión iPad no necesita teclado ni mouse. En la sala, mantén las flechas
del control inferior izquierdo para caminar; arrastra sobre cualquier zona libre
del lado derecho para mirar sin salir de los límites del cuarto. Toca un lomo o
el atril para que la cámara se acerque sola. En el lector, desliza la página a
la izquierda o derecha para cambiar de doble página, arrastra en vertical para
girar el libro y usa `−` o `+` para ajustar el encuadre. Los botones superiores
son objetivos táctiles de 48 puntos y el catálogo se desplaza con un arrastre
vertical sobre su lista.

## Archivos

| Ruta | Contenido |
| --- | --- |
| `biblioteca` | Ejecutable nativo para Linux x86_64 |
| `ejecutable.sh` | Lanzador del ejecutable nativo |
| `src/catalog.inc` | Catálogo y paleta de encuadernaciones |
| `src/stories.cpp` y `src/child_text.cpp` | Historias originales, figuras y adaptaciones infantiles |
| `src/imported_books.cpp` y `src/technical_notes.cpp` | Lecturas adaptadas de los PDF y notas técnicas por tema |
| `src/adult_books.cpp` | Cuatro relatos de la colección juvenil y adulta anterior |
| `src/book_art.cpp` | Ilustraciones de las misiones y esquemas de las páginas técnicas |
| `src/room.cpp` | Geometría de estanterías, mobiliario y sala |
| `src/room_layout.cpp` | Límites, colisiones de la cámara y rutas por la sala |
| `src/observatory.cpp` | Mosaico orbital, anillos suspendidos y lámparas |
| `src/hearth.cpp` | Aro, leños, brasas, campana y geometría de las llamas |
| `src/library.cpp` | Selección, cámara y animaciones del libro |
| `src/popup.cpp` | Carga de mallas, animación del pop-up, despliegue y unión a la hoja |
| `src/printing.cpp` | Diseños de folios, cubiertas y lomos |
| `src/render.cpp` y `assets/shaders/` | Render, luz y sombras |
| `assets/materials/grain.ppm` | Textura de vetas de madera, fibra de papel y moteado mineral |
| `assets/fonts/` | Barlow, Barlow Condensed y su licencia |
| `assets/popups/` | 56 escenas y diez naves en OBJ con normales suavizadas, colores por vértice y materiales MTL |
| `tools/popup_scenes.py` | Composición de cada doble página |
| `tools/spacecraft.py` y `tools/celestial.py` | Naves, mobiliario de misión, planetas y terrenos |
| `tools/mesh_builder.py` y `tools/generate_popups.py` | Construcción y exportación de mallas |
| `src/checks.cpp` | Pruebas de geometría, controles y render |

## Compilar y probar

```bash
make -j2
make test
./biblioteca --render-check ./cuadros
```

El modo de prueba genera cuadros de ambas salas, las lecturas ilustradas, el giro de las hojas y las 56 escenas desplegadas de los pop-ups. Para regenerar los modelos editables, ejecuta `python3 tools/generate_popups.py`. Para reconstruir la textura de materiales, ejecuta `python3 tools/generate_materials.py`.
