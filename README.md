Este proyecto implementa un analizador sintáctico LL(1) y un traductor dirigido por la sintaxis para archivos JSON simplificados.
## Alumna: Blanca Amada Franco Saldivar

## Archivos principales

- 'lexer.c' y 'lexer.h': Analizador léxico.
- 'parser.c': Analizador sintáctico LL(1) predictivo con panic mode.
- 'traductor.c': Traductor LL(1) que convierte JSON a XML.
- 'fuente.txt': Archivo de entrada JSON.
- 'traducido.xml': Archivo de salida generado por el traductor.

## Compilación en Windows (con GCC)
Para compilar manualmente desde la terminal:

--bash
gcc lexer.c parser.c -o parser.exe
gcc lexer.c traductor.c -o traductor.exe
## Ejecución
bash
parser.exe       # Verifica la validez sintáctica del JSON
traductor.exe    # Genera un archivo traducido.xml con la representación XML
```
## Compilación en Linux (con GCC)
gcc lexer.c parser.c -o parser
gcc lexer.c traductor.c -o traductor

##EJecución en linux
./parser
./traductor

