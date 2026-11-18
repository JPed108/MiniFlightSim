# SuperMiniFlightSim

Projeto desenvolvido em C++ utilizando OpenGL, GLFW e GLM. O objetivo é simular um ambiente espacial contendo um planeta, uma estrela e uma câmera em primeira pessoa que representa a visão de uma aeronave.

## Funcionalidades

- Criação de um planeta esférico posicionado na origem do sistema de coordenadas.
- Representação de uma estrela por uma esfera amarela.
- Definição do raio do planeta e das coordenadas da estrela por argumentos de linha de comando.
- Alternância entre renderização sólida e wireframe.
- Câmera em primeira pessoa posicionada inicialmente na superfície do planeta.
- Simulação dos estados de decolagem, voo e pouso.
- Controle da direção e da inclinação da câmera durante o voo.
- Ajuste da velocidade máxima da aeronave.

## Dependências

### Linux (Ubuntu)

Instale as dependências necessárias com os comandos:

```bash
sudo apt update
sudo apt install build-essential libglfw3-dev libgl1-mesa-dev libglu1-mesa-dev libglm-dev
```

### Windows (MSYS2 UCRT64)

O projeto pode ser compilado utilizando o ambiente MSYS2 UCRT64.

Instale as ferramentas e bibliotecas necessárias:

```bash
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-glfw mingw-w64-ucrt-x86_64-glm
```

O projeto também utiliza o GLAD. Seus arquivos, incluindo `glad.c` e os cabeçalhos correspondentes, devem estar disponíveis no diretório do projeto.

## Compilação

### Linux

No diretório do projeto, execute:

```bash
make
```

O executável gerado será `projetoCG`, conforme configurado no Makefile.

### Windows (MSYS2 UCRT64)

No terminal UCRT64, acesse o diretório do projeto e execute:

```bash
g++ main.cpp glad.c SphereCube.cpp Camera.cpp -o programa.exe -Iinclude -lglfw3 -lopengl32 -lgdi32
```

Após a compilação, será gerado o executável `programa.exe`.

## Execução

O programa recebe quatro argumentos pela linha de comando:

```text
<raio_planeta> <x_estrela> <y_estrela> <z_estrela>
```

Onde:

- `raio_planeta`: raio do planeta.
- `x_estrela`: coordenada X da estrela.
- `y_estrela`: coordenada Y da estrela.
- `z_estrela`: coordenada Z da estrela.

### Linux

```bash
./projetoCG 40.0 20.0 0.0 -80.0
```

### Windows (MSYS2 UCRT64)

```bash
./programa.exe 40.0 20.0 0.0 -80.0
```

Os valores podem ser modificados para experimentar diferentes tamanhos de planeta e posições da estrela.

## Controles

| Tecla | Ação |
|---|---|
| `R` | Alterna entre renderização sólida e wireframe. |
| `Espaço` | Inicia a decolagem quando a câmera está no solo ou o pouso quando está voando. |
| `←` | Gira a câmera para a esquerda. |
| `→` | Gira a câmera para a direita. |
| `W` | Ajusta a inclinação da câmera para cima. |
| `S` | Ajusta a inclinação da câmera para baixo. |
| `↑` | Aumenta o parâmetro de velocidade máxima. |
| `↓` | Reduz o parâmetro de velocidade máxima. |
| `ESC` | Encerra a aplicação. |

## Funcionamento da aeronave

A câmera começa posicionada na superfície do planeta, representando a aeronave no solo.

Ao pressionar `Espaço`, a aeronave inicia o procedimento de decolagem. Durante essa etapa, a câmera passa para o estado `TAKING_OFF`, e sua movimentação é atualizada de acordo com a lógica implementada na classe `Camera`.

Após a decolagem, a câmera passa para o estado `FLYING`, permitindo o controle de direção, inclinação e velocidade.

Durante o voo:

- As setas esquerda e direita controlam a rotação da câmera.
- As teclas `W` e `S` ajustam a inclinação da câmera.
- As setas para cima e para baixo alteram o parâmetro de velocidade máxima da aeronave.

Ao pressionar `Espaço` novamente durante o voo, a aeronave entra no estado `LANDING`, iniciando o procedimento de pouso.

## Estrutura do projeto

- `main.cpp`: inicialização da aplicação, criação da cena, processamento das entradas e renderização.
- `Camera.h`: declaração da classe responsável pela câmera e pelos estados de voo.
- `Camera.cpp`: implementação da movimentação, orientação, decolagem e pouso.
- `SphereCube.h`: declaração da classe responsável pela geometria das esferas.
- `SphereCube.cpp`: implementação da geometria e dos dados das esferas.
- `glad.c`: implementação do carregador de funções OpenGL.
- `include/`: diretório com os cabeçalhos das bibliotecas e classes do projeto.

## Observações

A aplicação utiliza GLFW para criação da janela e gerenciamento de entradas, GLAD para carregamento das funções OpenGL e GLM para operações matemáticas e transformações geométricas.

Os comandos de compilação apresentados consideram a organização atual dos arquivos e o ambiente MSYS2 UCRT64 no Windows.