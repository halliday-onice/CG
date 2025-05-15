### Guia de como executar um programa C/C++ em máquinas apple silicon (M family)

A apple tem procurado desistimular o uso de biblioteca OpenGL nas máquinas por acreditar ser algo datado, porém a Academia ainda utiliza para ensinar conceitos de Computação Gráfica.
Portanto, irei colocar aqui como fiz para compilar e executar 

- Foi necessario baixar utilizando o brew o GLUT, GL/glext:
```brew install freeglut```

```brew install glut```

```brew install glew```

```brew install glrxt```

```brew install mesa```

- Também foi necessario perceber que o MacOS coloca por padrão as bibliotecas OpenGL no caminho
```opt/homebrew/include/GL ```
- Baixar do browser um gerenciador de janelas chamado [XQuartz] (https://www.xquartz.org)
- Incluir no arquivo .c ou .cpp 
obs : dependendo do projeto podem haver outros header files para incluir
```#include <GLUT/glut.h>```

```#include <GL/glext.h>```

```#include <math.h>```
Para compilar:
 ```gcc -o output programName.c -I/opt/homebrew/include -L/opt/homebrew/lib -lglut -framework OpenGL -lm```

Onde ```-I/opt/homebrew/include``` Diz pro compilador onde encontrar os arquivos, inlcuindo o GL/glut e etc.

```-L/opt/homebrew/lib``` Indica pro compilador onde está a biblioteca Mesa

```lglut``` faz o link com OpenGL

```lm``` faz o link com a biblioteca que possui varias implementações de funções matemáticas
Para rodar basta fazer ```./nomeArquivo```

Para rodar arquivos cpp basta fazer ```g++ nomeArquivo.cpp -o nomeArquivo -I/opt/homebrew/include```

Precisa vincular os frameworks OpenGL e Glut explicitamente como ```g++ cameraTask.cpp -o cameraTask -framework OpenGL -framework GLUT -I/opt/homebrew/include```