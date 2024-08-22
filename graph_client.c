#include <stdio.h>
#include <SDL/SDL.h>
#include <math.h>
#include "buffer_code.h"
#include "timer_utils.h"

#define SCREEN_W 640 // Largura da janela que será criada
#define SCREEN_H 640 // Altura da janela que será criada

//#define BPP 8
//typedef Uint8 PixelType;
//#define BPP 16
//typedef Uint16 PixelType;
#define BPP 32
typedef Uint32 PixelType;

// Estrutura que define o canvas (superfície de desenho)
typedef struct canvas {
    SDL_Surface *canvas; // Ponteiro para a superfície SDL
    int Height;          // Altura do canvas
    int Width;           // Largura do canvas
    int Xoffset;         // Deslocamento em X, em pixels do canvas
    int Yoffset;         // Deslocamento em Y, em pixels do canvas
    int Xext;            // Extensão extra em X
    int Yext;            // Extensão extra em Y
    double Xmax;         // Valor máximo em X na escala
    double Ymax;         // Valor máximo em Y na escala
    double Xstep;        // Meio da distância entre pixels em X na escala 'Xmax'
    PixelType *zpixel;   // Ponteiro para os pixels da superfície

} Tcanvas;

// Estrutura que armazena os dados a serem desenhados no canvas
typedef struct dataholder {
    Tcanvas *canvas;     // Ponteiro para o canvas
    double Tcurrent;     // Tempo atual
    double Lcurrent;     // Nível atual
    PixelType Lcolor;    // Cor do nível
    double INcurrent;    // Ângulo de entrada atual
    PixelType INcolor;   // Cor do ângulo de entrada
    double OUTcurrent;   // Ângulo de saída atual
    PixelType OUTcolor;  // Cor do ângulo de saída

} Tdataholder;

// Função que desenha um pixel no canvas
void c_pixeldraw(Tcanvas *canvas, int x, int y, PixelType color) {
    *( ((PixelType*)canvas->canvas->pixels) + ((-y+canvas->Yoffset) * canvas->canvas->w + x+ canvas->Xoffset)) = color;
}

// Função que desenha uma linha horizontal no canvas
void c_hlinedraw(Tcanvas *canvas, int xstep, int y, PixelType color) {
    int offset =  (-y+canvas->Yoffset) * canvas->canvas->w;
    int x;

    for (x = 0; x < canvas->Width+canvas->Xoffset ; x += xstep) {
        *( ((PixelType*)canvas->canvas->pixels) + (offset + x)) = color;
    }
}

// Função que desenha uma linha vertical no canvas
void c_vlinedraw(Tcanvas *canvas, int x, int ystep, PixelType color) {
    int offset = x + canvas->Xoffset;
    int y;
    int Ystep = ystep * canvas->canvas->w;

    for (y = 0; y < canvas->Height+canvas->Yext ; y += ystep) {
        *( ((PixelType*)canvas->canvas->pixels) + (offset + y * canvas->canvas->w)) = color;
    }
}

// Função que desenha uma linha reta entre dois pontos no canvas
void c_linedraw(Tcanvas *canvas, double x0, double y0, double x1, double y1, PixelType color) {
    double x;

    for (x = x0; x <= x1; x += canvas->Xstep) {
        c_pixeldraw(canvas, (int)(x * canvas->Width / canvas->Xmax + 0.5), 
                            (int)((double)canvas->Height / canvas->Ymax * (y1 * (x1 - x) + y1 * (x - x0)) / (x1 - x0) + 0.5), color);
    }
}

// Função que inicializa o canvas e configura a superfície de desenho
Tcanvas *c_open(int Width, int Height, double Xmax, double Ymax) {
    int x, y;
    Tcanvas *canvas;
    canvas = malloc(sizeof(Tcanvas));

    canvas->Xoffset = 10;
    canvas->Yoffset = Height;
    canvas->Xext = 10;
    canvas->Yext = 10;

    canvas->Height = Height;
    canvas->Width  = Width; 
    canvas->Xmax   = Xmax;
    canvas->Ymax   = Ymax;

    canvas->Xstep  = Xmax / (double)Width / 2;

    SDL_Init(SDL_INIT_VIDEO); // Inicializa o SDL
    canvas->canvas = SDL_SetVideoMode(canvas->Width+canvas->Xext, canvas->Height+canvas->Yext, BPP, SDL_SWSURFACE); 

    c_hlinedraw(canvas, 1, 0, (PixelType) SDL_MapRGB(canvas->canvas->format,  255, 255,  255));
    for (y = 10; y < Ymax; y += 10) {
        c_hlinedraw(canvas, 3, y * Height / Ymax , (PixelType) SDL_MapRGB(canvas->canvas->format,  220, 220,  220));
    }
    c_vlinedraw(canvas, 0, 1, (PixelType) SDL_MapRGB(canvas->canvas->format,  255, 255,  255));
    for (x = 10; x < Xmax; x += 10) {
        c_vlinedraw(canvas, x * Width / Xmax, 3, (PixelType) SDL_MapRGB(canvas->canvas->format,  220, 220,  220));
    }

    return canvas;
}

// Função que inicializa os dados a serem desenhados no canvas
Tdataholder *datainit(int Width, int Height, double Xmax, double Ymax, double Lcurrent, double INcurrent, double OUTcurrent) {
    Tdataholder *data = malloc(sizeof(Tdataholder));

    data->canvas = c_open(Width, Height, Xmax, Ymax);
    data->Tcurrent = 0;
    data->Lcurrent = Lcurrent;
    data->Lcolor = (PixelType) SDL_MapRGB(data->canvas->canvas->format,  255, 180,  0);
    data->INcurrent = INcurrent;
    data->INcolor = (PixelType) SDL_MapRGB(data->canvas->canvas->format,  180, 255,  0);
    data->OUTcurrent = OUTcurrent;
    data->OUTcolor = (PixelType) SDL_MapRGB(data->canvas->canvas->format,  0, 180,  255);

    return data;
}

// Função que define as cores dos dados no canvas
void setdatacolors(Tdataholder *data, PixelType Lcolor, PixelType INcolor, PixelType OUTcolor) {
    data->Lcolor = Lcolor;
    data->INcolor = INcolor;
    data->OUTcolor = OUTcolor;
}

// Função que desenha os dados no canvas
void datadraw(Tdataholder *data, double time, double level, double inangle) {
    c_linedraw(data->canvas, data->Tcurrent, data->Lcurrent, time, level, data->Lcolor);
    c_linedraw(data->canvas, data->Tcurrent, data->INcurrent, time, inangle, data->INcolor);
    data->Tcurrent = time;
    data->Lcurrent = level;
    data->INcurrent = inangle;

    SDL_Flip(data->canvas->canvas);
}

// Função que verifica se o evento de saída foi acionado
void quitevent() {
    SDL_Event event;

    while (SDL_PollEvent(&event)) { 
        if (event.type == SDL_QUIT) { 
            SDL_Quit();
            exit(1); // Isso encerrará todas as threads!
        }
    }
}

// Função que reseta a simulação
void reset_simulation(Tdataholder *data) {
    data->Tcurrent = 0;
    data->Lcurrent = 100; // Define um valor inicial apropriado
    data->INcurrent = 50;
    data->OUTcurrent = 50;

    SDL_FillRect(data->canvas->canvas, NULL, SDL_MapRGB(data->canvas->canvas->format, 0, 0, 0)); // Limpa a tela

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

    SDL_Flip(data->canvas->canvas); // Atualiza a tela
}

// Função que desenha o gráfico em um loop infinito
void *plot_graph() {
    Tdataholder *data;
    double t = 0;
    double lvl = 40;
    double angleIn = 100;
    double tempo = 0;
    double var_aux;

    data = datainit(640, 480, 120, 110, 45, 0, 0);

    while (1) {
        tempo += 50;
        //buffer_put_string(&command_ccb,'GetNivel!');
        t = tempo / 1000;
        var_aux = 100 * buffer_get(&nivel_ccb_graph);
        if (var_aux != 0)
            lvl = var_aux;
        var_aux = buffer_get(&angleIn_ccb);
        if (var_aux != 0)
            angleIn = var_aux;
        datadraw(data, t, (double)lvl, (double)angleIn);

        if (buffer_get(&Start_ccb_graph) == 1) {
            tempo = 0;
            buffer_put(&Start_ccb_graph, 0);
            reset_simulation(data);
        }
        sleepMs(50);
    }

    while (1) {
        quitevent();
    }
}