#include <stdio.h>
#include <SDL/SDL.h>
#include <math.h>
#include "buffer_code.h"
#include "timer_utils.h"

// Definições das dimensões da tela e do valor de PI
#define SCREEN_W 640 // Largura da tela que será criada
#define SCREEN_H 640 // Altura da tela que será criada
#define M_PI 3.14159265358979323846 // Valor de PI

// Definição do tipo de pixel com base em 32 bits
#define BPP 32
typedef Uint32 PixelType;

// Estrutura que representa o canvas (superfície de desenho)
typedef struct canvas {
    SDL_Surface *canvas; // Ponteiro para a superfície SDL
    int Height; // Altura do canvas
    int Width;  // Largura do canvas
    int Xoffset; // Deslocamento X em pixels do canvas
    int Yoffset; // Deslocamento Y em pixels do canvas
    int Xext; // Largura extra em X
    int Yext; // Altura extra em Y
    double Xmax; // Valor máximo no eixo X
    double Ymax; // Valor máximo no eixo Y
    double Xstep; // Metade da distância entre pixels no eixo X na escala de 'Xmax'
    PixelType *zpixel; // Ponteiro para os pixels
} Tcanvas;

// Estrutura que armazena os dados do gráfico
typedef struct dataholder {
    Tcanvas *canvas; // Ponteiro para o canvas
    double Tcurrent; // Tempo atual
    double Lcurrent; // Nível atual
    PixelType Lcolor; // Cor do nível
    double INcurrent; // Valor de entrada atual
    PixelType INcolor; // Cor da entrada
    double OUTcurrent; // Valor de saída atual
    PixelType OUTcolor; // Cor da saída
} Tdataholder;

// Função para desenhar um pixel no canvas
void c_pixeldraw(Tcanvas *canvas, int x, int y, PixelType color) {
    *((PixelType*)canvas->canvas->pixels + ((-y + canvas->Yoffset) * canvas->canvas->w + x + canvas->Xoffset)) = color;
}

// Função para desenhar uma linha horizontal no canvas
void c_hlinedraw(Tcanvas *canvas, int xstep, int y, PixelType color) {
    int offset = (-y + canvas->Yoffset) * canvas->canvas->w;
    int x;

    for (x = 0; x < canvas->Width + canvas->Xoffset; x += xstep) {
        *((PixelType*)canvas->canvas->pixels + (offset + x)) = color;
    }
}

// Função para desenhar uma linha vertical no canvas
void c_vlinedraw(Tcanvas *canvas, int x, int ystep, PixelType color) {
    int offset = x + canvas->Xoffset;
    int y;
    int Ystep = ystep * canvas->canvas->w;

    for (y = 0; y < canvas->Height + canvas->Yext; y += ystep) {
        *((PixelType*)canvas->canvas->pixels + (offset + y * canvas->canvas->w)) = color;
    }
}

// Função para desenhar uma linha inclinada no canvas
void c_linedraw(Tcanvas *canvas, double x0, double y0, double x1, double y1, PixelType color) {
    double x;

    for (x = x0; x <= x1; x += canvas->Xstep) {
        c_pixeldraw(canvas, (int)(x * canvas->Width / canvas->Xmax + 0.5),
            (int)((double)canvas->Height / canvas->Ymax * (y1 * (x1 - x) + y1 * (x - x0)) / (x1 - x0) + 0.5), color);
    }
}

// Função para abrir e inicializar um canvas
Tcanvas *c_open(int Width, int Height, double Xmax, double Ymax) {
    int x, y;
    Tcanvas *canvas = malloc(sizeof(Tcanvas));

    canvas->Xoffset = 10;
    canvas->Yoffset = Height;
    canvas->Xext = 10;
    canvas->Yext = 10;
    canvas->Height = Height;
    canvas->Width = Width; 
    canvas->Xmax = Xmax;
    canvas->Ymax = Ymax;
    canvas->Xstep = Xmax / (double)Width / 2;

    SDL_Init(SDL_INIT_VIDEO); // Inicializa o SDL
    canvas->canvas = SDL_SetVideoMode(canvas->Width + canvas->Xext, canvas->Height + canvas->Yext, BPP, SDL_SWSURFACE); 

    // Desenha as linhas horizontais e verticais iniciais no canvas
    c_hlinedraw(canvas, 1, 0, (PixelType) SDL_MapRGB(canvas->canvas->format, 255, 255, 255));
    for (y = 10; y < Ymax; y += 10) {
        c_hlinedraw(canvas, 3, y * Height / Ymax, (PixelType) SDL_MapRGB(canvas->canvas->format, 220, 220, 220));
    }
    c_vlinedraw(canvas, 0, 1, (PixelType) SDL_MapRGB(canvas->canvas->format, 255, 255, 255));
    for (x = 10; x < Xmax; x += 10) {
        c_vlinedraw(canvas, x * Width / Xmax, 3, (PixelType) SDL_MapRGB(canvas->canvas->format, 220, 220, 220));
    }

    return canvas;
}

// Função para inicializar os dados do gráfico
Tdataholder *datainit(int Width, int Height, double Xmax, double Ymax, double Lcurrent, double INcurrent, double OUTcurrent) {
    Tdataholder *data = malloc(sizeof(Tdataholder));

    data->canvas = c_open(Width, Height, Xmax, Ymax);
    data->Tcurrent = 0;
    data->Lcurrent = Lcurrent;
    data->Lcolor = (PixelType) SDL_MapRGB(data->canvas->canvas->format, 255, 180, 0);
    data->INcurrent = INcurrent;
    data->INcolor = (PixelType) SDL_MapRGB(data->canvas->canvas->format, 180, 255, 0);
    data->OUTcurrent = OUTcurrent;
    data->OUTcolor = (PixelType) SDL_MapRGB(data->canvas->canvas->format, 0, 180, 255);

    return data;
}

// Função para definir as cores dos dados
void setdatacolors(Tdataholder *data, PixelType Lcolor, PixelType INcolor, PixelType OUTcolor) {
    data->Lcolor = Lcolor;
    data->INcolor = INcolor;
    data->OUTcolor = OUTcolor;
}

// Função para desenhar os dados no gráfico
void datadraw(Tdataholder *data, double time, double level, double inangle, double outangle) {
    c_linedraw(data->canvas, data->Tcurrent, data->Lcurrent, time, level, data->Lcolor);
    c_linedraw(data->canvas, data->Tcurrent, data->INcurrent, time, inangle, data->INcolor);
    c_linedraw(data->canvas, data->Tcurrent, data->OUTcurrent, time, outangle, data->OUTcolor);
    data->Tcurrent = time;
    data->Lcurrent = level;
    data->INcurrent = inangle;
    data->OUTcurrent = outangle;

    SDL_Flip(data->canvas->canvas); // Atualiza a tela
}

// Função para lidar com eventos de saída
void quitevent() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) { 
        if (event.type == SDL_QUIT) { 
            SDL_Quit(); // Fecha o SDL
            exit(1); // Termina todos os threads
        }
    }
}

// Função para resetar a simulação
void reset_simulation(Tdataholder *data) {
    // Reseta os parâmetros da simulação
    data->Tcurrent = 0;
    data->Lcurrent = 100; // Define um valor inicial apropriado
    data->INcurrent = 50;
    data->OUTcurrent = 50;

    // Limpa a tela
    SDL_FillRect(data->canvas->canvas, NULL, SDL_MapRGB(data->canvas->canvas->format, 0, 0, 0));

    // Redesenha as linhas de grade
    c_hlinedraw(data->canvas, 1, 0, (PixelType) SDL_MapRGB(data->canvas->canvas->format, 255, 255, 255));
    int y;
    for (y = 10; y < data->canvas->Ymax; y += 10) {
        c_hlinedraw(data->canvas, 3, y * data->canvas->Height / data->canvas->Ymax, (PixelType) SDL_MapRGB(data->canvas->canvas->format, 220, 220, 220));
    }
    c_vlinedraw(data->canvas, 0, 1, (PixelType) SDL_MapRGB(data->canvas->canvas->format, 255, 255, 255));
    int x;
    for (x = 10; x < data->canvas->Xmax; x += 10) {
        c_vlinedraw(data->canvas, x * data->canvas->Width / data->canvas->Xmax, 3, (PixelType) SDL_MapRGB(data->canvas->canvas->format, 220, 220, 220));
    }

    // Atualiza a tela
    SDL_Flip(data->canvas->canvas);
}

// Função que executa a plotagem do gráfico
void *plot_graph() {
    Tdataholder *data;
    double t = 0;
    double lvl;
    double angleIn;
    double angleOut;
    double var_aux;

    data = datainit(640, 480, 120, 110, 45, 0, 0);

    while (1) {
        // Verifica se há dados disponíveis nos buffers e desenha no gráfico
        while (nivel_scb.count > 0 && tempo_scb.count > 0 && angleIn_scb.count > 0 && angleOut_scb.count > 0) {
            t = buffer_get(&tempo_scb) / 1000;
            lvl = 100 * buffer_get(&nivel_scb);
            var_aux = buffer_get(&angleIn_scb);
            if (var_aux != 0)
                angleIn = var_aux;
            angleOut = buffer_get(&angleOut_scb);
            if (t != 0) {
                datadraw(data, t, (double)lvl, (double)angleIn, (double)angleOut);
            }
        }
        // Verifica se o buffer Start_scb foi acionado para resetar a simulação
        if (buffer_get(&Start_scb) == 1) {
            buffer_put(&Start_scb, 0);
            reset_simulation(data);
        }
        sleepMs(50);
    }

    // Checa por eventos de saída
    while (1) {
        quitevent();
    }
}