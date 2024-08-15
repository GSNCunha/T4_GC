// graphics.h

#ifndef GRAPH_SERVER_H
#define GRAPH_SERVER_H

#include <SDL/SDL.h>
#include <math.h>

#define SCREEN_W 640  // Window width
#define SCREEN_H 640  // Window height
#define BPP 32        // Bits per pixel

typedef Uint32 PixelType;  // Pixel type depending on BPP

// Struct representing the canvas for drawing
typedef struct canvas {
    SDL_Surface *canvas;
    int Height;    // Canvas height
    int Width;     // Canvas width
    int Xoffset;   // X offset in canvas pixels
    int Yoffset;   // Y offset in canvas pixels
    int Xext;      // Extra width in canvas
    int Yext;      // Extra height in canvas
    double Xmax;   // Maximum X value for scaling
    double Ymax;   // Maximum Y value for scaling
    double Xstep;  // Half the distance between X pixels in 'Xmax' scale

    PixelType *zpixel;  // Pointer to pixel data
} Tcanvas;

// Struct holding data for plotting
typedef struct dataholder {
    Tcanvas *canvas;
    double Tcurrent;
    double Lcurrent;
    PixelType Lcolor;
    double INcurrent;
    PixelType INcolor;
    double OUTcurrent;
    PixelType OUTcolor;
} Tdataholder;

// Function prototypes

// Initializes the canvas and returns a pointer to the canvas struct
Tcanvas *c_open(int Width, int Height, double Xmax, double Ymax);

// Initializes the dataholder and returns a pointer to it
Tdataholder *datainit(int Width, int Height, double Xmax, double Ymax, double Lcurrent, double INcurrent, double OUTcurrent);

// Sets the colors for different data lines
void setdatacolors(Tdataholder *data, PixelType Lcolor, PixelType INcolor, PixelType OUTcolor);

// Draws the data on the canvas
void datadraw(Tdataholder *data, double time, double level, double inangle, double outangle);

// Handles SDL quit event
void quitevent();

// drawing functions for the canvas

void c_pixeldraw(Tcanvas *canvas, int x, int y, PixelType color);

void c_hlinedraw(Tcanvas *canvas, int xstep, int y, PixelType color);

void c_vlinedraw(Tcanvas *canvas, int x, int ystep, PixelType color);

void c_linedraw(Tcanvas *canvas, double x0, double y0, double x1, double y1, PixelType color);

// Main plotting function
void *plot_graph();

#endif // GRAPHICS_H
