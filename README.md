# 📚 Tarea 3: CC4102 - Diseño y Análisis de Algoritmos

Diseño y análisis de un algoritmo probabilístico: Filtro de Bloom.

## 👤 Integrantes (Sección 2): 

- Valentina Alarcón Yañez (valentina.alarcon.y at ug.uchile.cl).
- Naomi Cautivo Bahamóndez (naomi.cautivo at ug.uchile.cl).
- Máximo Flores Valenzuela (mflores at dcc.uchile.cl).

## 👤 Profesores:

- Benjamín Bustos (bebustos at dcc.uchile.cl).
- Gonzalo Navarro Badino (gnavarro at dcc.uchile.cl).

## ✍️ Entregables

En el siguiente repositorio se entregan los siguientes archivos. Todas las implementaciones fueron realizadas en C++:

- 📁 $\texttt{/csv/}$ - Contiene los archivos de tipo 'csv' de donde se obtienen los datos para la experimentación. ('Film-Names.csv' y 'Popular-Baby-Names.csv').
- 📁 $\texttt{/bloom-results/}$ - Contiene los archivos que describen los tiempos de ejecución y número de 'colisiones' de la búsqueda utilizando filtro de Bloom. Entiéndase 'colisiones' como el número de elementos que pasaron el filtro y sabemos que no se encuentran en el 'storage'. Estos contienen los resultados obtenidos para cada combinación de parámetros (N,p) en formato $\texttt{.txt}$, y cada uno tiene las ejecuciones para cada combinación de (m, k).
- 📁 $\texttt{/grep-results/}$ - Contiene los archivos que describen los tiempos de ejecución de la búsqueda secuencial utilizando el comando 'grep'. Estos contienen los resultados obtenidos para cada combinación de parámetros (N,p) en formato $\texttt{.txt}$, y, si bien cada uno indica ciertas combinaciones de (M, k), en la práctica son ejecuciones repetidas (puesto que esta búsqueda no depende de estos parámetros).
- 📄 $\texttt{rapidcsv.h}$ - Archivo con encabezados que se utilizan para las funciones de manejo de archivos de tipo 'csv'. Este archivo es importado de https://github.com/d99kris/rapidcsv.
- 📄 $\texttt{MurmurHash3.h}$ - Archivo con encabezados que se utilizan para crear las funciones de hash de tipo 'Murmur'.
- 📄 $\texttt{MurmurHash3.cpp}$ - Archivo con las funciones asociadas al hashing de tipo 'Murmur'. Estos últimos dos archivos son importados de https://github.com/aappleby/smhasher/tree/master/src.
- 📄 $\texttt{bloom.cpp}$ - Archivo que contiene la implementación de los algoritmos completos. Contiene diversas funciones auxiliares de manejo de archivos, creación de funciones de hashing, manejo de comando 'grep', aplicación del filtro de bloom y, por supuesto, la función 'main'. Esta última está encargada de la experimentación solicitada en el enunciado, utilizando tanto los parámetros pedidos para (N,p) como algunas selecciones propias para (m,k), y revisando tiempos de ejecución para ambos tipos de búsqueda (+ error para el filtro de Bloom). Deja los resultados en las carpetas 'grep-results' para la búsqueda secuencial, y en 'bloom-results' para Bloom.

La ejecución de este último archivo creará además cuatro archivos $\texttt{.txt}$ distintos, que no se incluyen en este repositorio. Estos son:
- 📄 $\texttt{after\\_terms.txt}$ - Archivo que guarda dinámicamente los strings que pasaron el filtro de Bloom tras cada ejecución.
- 📄 $\texttt{name\\_file.txt}$ - Archivo que guarda los strings de 'Popular-Baby-Names.csv' en un formato más amigable para su trabajo.
- 📄 $\texttt{search\\_terms.txt}$ - Archivo que guarda dinámicamente los strings que pertenecen a la 'secuencia N' mencionada en el enunciado: Aquellos que se someterán a ambas búsquedas para comparar.


## 💻 Ejecución

El $\texttt{main}$ ejecuta un gran proceso, con dos subprocesos similares.

En general:
- Se limpian las carpetas de resultados si actualmente ya existen, y se vuelven a crear.
- Se pasan los strings del 'csv' 'Popular-Baby-Names.csv' a un vector llamado 'names', y a un 'txt' llamado 'names_terms_file'. También se pasan los strings del 'csv' 'Film-Names.csv' a un vector llamado 'film_names'.
- Luego, se declaran los valores de 'm' para los que se va a iterar: {2^17, 2^18, 2^19}. Estos números se escogieron pensando en el hecho de que tenían que ser al menos mayores que la longitud de 'names' (alrededor de 90mil), y se comienza el ciclo. También se tendrá que los valores de 'k' serán $\{\lceil(\ln(2) \cdot m \/ N) \rceil, 2 \cdot \lceil(m\/N) \rceil\}$. El primer valor se argumentó en las demostraciones matemáticas del informe, y el segundo es una variación para comprobar que efectivamente el primer $k$ sea óptimo.
- Luego, para cada valor de 'N' solicitado ({2^10, 2^12, 2^14, 2^16}), y para cada valor de 'k', se crea un vector M, las k funciones de hashing distintas, y se hace la inserción de todos los nombres de 'names' en el filtro de Bloom.
- Tras esto último iteramos para cada 'p' ({0.0, 0.25, 0.5, 0.75, 1.0}), creando una secuencia N con esa proporción. Esta se somete a la búsqueda secuencial con grep tomando el tiempo para cada iteración, y a la del filtro de Bloom tomando tiempo y calculando colisiones. Cada experimento muestra sus resultados en un archivo designado de acuerdo a su combinación (N,p).


Para detonar la experimentación, se deben ejecutar los siguientes comandos en $\texttt{bash}$:

`g++ -o bloom bloom.cpp MurmurHash3.cpp`

`./bloom`

el cual ejecuta el $\texttt{main}$ de ese archivo.

El archivo debería ser considerado como seguro si existe antivirus. Si existe algún problema con el acceso, puede que desactivar temporalmente el antivirus sea una posible solución.

Notar además que los resultados pueden variar de acuerdo a lo visto en el informe de acuerdo al sistema que se utilice para correr la tarea. Para la realización de esta tarea se usó una gcc version 12.2.0, con distro Debian 12.2.0-14, utilizando WSL2.

Nota: Es muy importante que tenga instalado 'filesystem', pues de lo contrario, puede que la ejecución no funcione. Además, una alternativa de comando para compilar es el siguiente:

`g++ -std=c++17 -o bloom bloom.cpp MurmurHash3.cpp`

Esto pues 'filesystem' puede que no funcione correctamente con otras versiones de C++. Sin embargo, sigue siendo relevante que tenga este módulo instalado.

En nuestro caso, logramos su instalación en Debian12 con el comando: `sudo apt-get install build-essential gdb gdbserver zip sshfs less default-jre man`.

Recomendamos fuertemente el uso de algo similar, puesto que en nuestro caso, por ejemplo, el main no puede ejecutarse correctamente en PowerShell de Windows.
