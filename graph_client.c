#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <SDL/SDL.h>
#include <math.h>
#include "buffer_code.h"
#include "timer_utils.h"

#define SCREEN_W 640 //tamanho da janela que sera criada
#define SCREEN_H 640
#define GRAPH_CLIENT_PERIOD 50 //50ms

//#define BPP 8
//typedef Uint8 PixelType;
//#define BPP 16
//typedef Uint16 PixelType;
#define BPP 32
typedef Uint32 PixelType;



typedef struct canvas {
  SDL_Surface *canvas;
  int Height; // canvas height
  int Width;  // canvas width
  int Xoffset; // X off set, in canvas pixels
  int Yoffset; // Y off set, in canvas pixels
  int Xext; // X extra width
  int Yext; // Y extra height
  double Xmax;
  double Ymax;
  double Xstep; // half a distance between X pixels in 'Xmax' scale

  PixelType *zpixel;

} Tcanvas;

typedef struct dataholder {
  Tcanvas *canvas;
  double   Tcurrent;
  double   Lcurrent;
  PixelType Lcolor;
  double   INcurrent;
  PixelType INcolor;
  double   OUTcurrent;
  PixelType OUTcolor;

} Tdataholder;

void c_pixeldraw(Tcanvas *canvas, int x, int y, PixelType color)
{
  *( ((PixelType*)canvas->canvas->pixels) + ((-y+canvas->Yoffset) * canvas->canvas->w + x+ canvas->Xoffset)) = color;
}

void c_hlinedraw(Tcanvas *canvas, int xstep, int y, PixelType color)
{
  int offset =  (-y+canvas->Yoffset) * canvas->canvas->w;
  int x;

  for (x = 0; x< canvas->Width+canvas->Xoffset ; x+=xstep) {
        *( ((PixelType*)canvas->canvas->pixels) + (offset + x)) = color;
  }
}

void c_vlinedraw(Tcanvas *canvas, int x, int ystep, PixelType color)
{
  int offset = x+canvas->Xoffset;
  int y;
  int Ystep = ystep*canvas->canvas->w;

  for (y = 0; y< canvas->Height+canvas->Yext ; y+=ystep) {
    *( ((PixelType*)canvas->canvas->pixels) + (offset + y*canvas->canvas->w)) = color;
  }
}


void c_linedraw(Tcanvas *canvas, double x0, double y0, double x1, double y1, PixelType color) {
  double x;

  for (x=x0; x<=x1; x+=canvas->Xstep) {
    c_pixeldraw(canvas, (int)(x*canvas->Width/canvas->Xmax+0.5), (int)((double)canvas->Height/canvas->Ymax*(y1*(x1-x)+y1*(x-x0))/(x1-x0)+0.5),color);
  }
}


Tcanvas *c_open(int Width, int Height, double Xmax, double Ymax)
{
  int x,y;
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

  canvas->Xstep  = Xmax/(double)Width/2;

  //  canvas->zpixel = (PixelType *)canvas->canvas->pixels +(Height-1)*canvas->canvas->w;

  SDL_Init(SDL_INIT_VIDEO); //SDL init
  canvas->canvas = SDL_SetVideoMode(canvas->Width+canvas->Xext, canvas->Height+canvas->Yext, BPP, SDL_SWSURFACE); 

  c_hlinedraw(canvas, 1, 0, (PixelType) SDL_MapRGB(canvas->canvas->format,  255, 255,  255));
  for (y=10;y<Ymax;y+=10) {
    c_hlinedraw(canvas, 3, y*Height/Ymax , (PixelType) SDL_MapRGB(canvas->canvas->format,  220, 220,  220));
  }
  c_vlinedraw(canvas, 0, 1, (PixelType) SDL_MapRGB(canvas->canvas->format,  255, 255,  255));
  for (x=10;x<Xmax;x+=10) {
    c_vlinedraw(canvas, x*Width/Xmax, 3, (PixelType) SDL_MapRGB(canvas->canvas->format,  220, 220,  220));
  }

  return canvas;
}


Tdataholder *datainit(int Width, int Height, double Xmax, double Ymax, double Lcurrent, double INcurrent, double OUTcurrent) {
  Tdataholder *data = malloc(sizeof(Tdataholder));


  data->canvas=c_open(Width, Height, Xmax, Ymax);
  data->Tcurrent=0;
  data->Lcurrent=Lcurrent;
  data->Lcolor= (PixelType) SDL_MapRGB(data->canvas->canvas->format,  255, 180,  0);
  data->INcurrent=INcurrent;
  data->INcolor=(PixelType) SDL_MapRGB(data->canvas->canvas->format,  180, 255,  0);
  data->OUTcurrent=OUTcurrent;
  data->OUTcolor=(PixelType) SDL_MapRGB(data->canvas->canvas->format,  0, 180,  255);


  return data;
}

void setdatacolors(Tdataholder *data, PixelType Lcolor, PixelType INcolor, PixelType OUTcolor) {
  data->Lcolor=Lcolor;
  data->INcolor=INcolor;
  data->OUTcolor=OUTcolor;
}

void datadraw(Tdataholder *data, double time, double level, double inangle) {
  c_linedraw(data->canvas,data->Tcurrent,data->Lcurrent,time,level,data->Lcolor);
  c_linedraw(data->canvas,data->Tcurrent,data->INcurrent,time,inangle,data->INcolor);
  data->Tcurrent = time;
  data->Lcurrent = level;
  data->INcurrent = inangle;

  SDL_Flip(data->canvas->canvas);
}

void quitevent() {
  SDL_Event event;

  while(SDL_PollEvent(&event)) { 
    if(event.type == SDL_QUIT) { 
      // close files, etc...

      SDL_Quit();
      exit(1); // this will terminate all threads !
    }
  }

}

void reset_simulation(Tdataholder *data) {
    // Reset the simulation parameters
    data->Tcurrent = 0;
    data->Lcurrent = 100; // Set an appropriate initial value
    data->INcurrent = 50;
    data->OUTcurrent = 50;

    // Clear the screen
    SDL_FillRect(data->canvas->canvas, NULL, SDL_MapRGB(data->canvas->canvas->format, 0, 0, 0));

    // Redraw the grid lines
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

    // Update the screen
    SDL_Flip(data->canvas->canvas);
}


//
//
//
//
//
//
//

void *plot_graph() {
  Tdataholder *data;
  double t=0;
  double lvl = 40;
  double angleIn =50+100*0.5;
  double tempo = 0;
  double var_aux;
  struct timespec t_spec;

  data = datainit(640,480,120,110,45,0,0);
    clock_gettime(CLOCK_MONOTONIC_RAW, &t_spec);
    while (1) {
      while ((get_elapsed_time_ms(t_spec)) < GRAPH_CLIENT_PERIOD); //verifica se ja se passou o periodo da planta
        tempo += (double)get_elapsed_time_ms(t_spec);
        clock_gettime(CLOCK_MONOTONIC_RAW, &t_spec);
            t = tempo/1000;
            var_aux = buffer_get(&nivel_ccb_graph);
            if(var_aux != 0)
              lvl = var_aux;
              var_aux = buffer_get(&angleIn_ccb);
            if(var_aux != 0)
              angleIn = var_aux;
            datadraw(data, t, (double)lvl, (double)angleIn);
            //Código pra testar no bitwise sem a parte gráfica:
            /*
            fflush(stdout);
            system("clear");
            printf("tempo: %.2f\n", t);
            printf("nivel: %.2f\n", lvl);
            printf("angleIn: %.2f\n", angleIn);
            */
            //-------------------------------------------
        if(buffer_get(&Start_ccb_graph) == 1)
        {
          tempo = 0;
          buffer_put(&Start_ccb_graph, 0);
          reset_simulation(data);
        }
    }


  while(1) {
    quitevent();
  }
}