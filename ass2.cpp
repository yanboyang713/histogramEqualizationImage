/*-------------------------------------------------------------
Student's Name: Boyang YAN
Student's email address: by932@uowmail.edu.au
Last modification:05/05/2016
-------------------------------------------------------------*/
#include <cstdio>
#include <iostream>
#include <signal.h>
#include <SDL/SDL_config.h>
#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_audio.h>
#include <stdlib.h>
#include <string>
#include <cmath>
using namespace std;
bool showHelp (int, char *[]);
bool checkInputFile (int, char *[]);
bool LoadSDLLib();
void makeScreen(SDL_Surface *&);
void loadBmp(SDL_Surface *&, char *[]);
static void quit(int);
void keyEven(Uint8 *&, int &, SDL_Event &, SDL_Surface *&, SDL_Surface *&, float &, bool);
void clearAndBlitAndUpdate(SDL_Surface *&, SDL_Surface *&);
float GetPixelComp(SDL_Surface *&, int, int, int);
SDL_Surface* zoomImage(SDL_Surface *&, float);
bool judgeFormat(SDL_Surface *&);
Uint32 getpixel(SDL_Surface *&, int, int);
void putpixel(SDL_Surface *&, int, int, Uint32);
void histogramEqualization(SDL_Surface *&);
void initArray(float*&, int);
bool judgeFormatHaveAlphaInfo(SDL_Surface *&);
static inline SDL_Surface* SPG_CopySurface(SDL_Surface*);
static const int SCR_WIDTH = 640;
static const int SCR_HEIGHT = 480;
int offsetX = 0;
int offsetY = 0;
int main(int argc, char *argv[])
{
	SDL_Surface *screen = NULL;
	SDL_Surface *image = NULL;
	SDL_Event event;
	Uint8 *keys;
	bool zoom = false;
	int done = 0;
	float zoomFactor = 1;
	if (showHelp (argc, argv) == true)	return 0;//show help
	//check input file format
	if (checkInputFile (argc, argv) == true)	return 0;
	//Load the SDL library
	if (LoadSDLLib() == false) return 0;
	makeScreen(screen);
	loadBmp(image, argv);
	histogramEqualization(image);
	clearAndBlitAndUpdate(screen, image);
	keyEven(keys,done,event, screen, image, zoomFactor, zoom);//process the keyboard event
	// free the image if it is no longer needed 
	SDL_FreeSurface(image);    
	// Release memeory and Quit SDL
	SDL_FreeSurface(screen);
	SDL_Quit();
	return 0;
}
bool showHelp (int argc, char *argv[])
{
	string check(argv[1]);//convert char* to string
	if (argc == 2 && check == "-help")
	{
		cout << "This program is a color image display and enhancement program!!!" << endl;
		cout << "Run this program command line is: ";
		cout << "histEqImage xxx.bmp" << endl;
		cout << "histEqImage is program name" << endl;
		cout << "xxx is the name of the input picture" << endl;
		return true;
	}
	return false;
}
bool checkInputFile (int argc, char *argv[])
{
	string check(argv[1]);//convert char* to string
	int size = check.length();
	if (argc < 2)
	{
		cout << "Usage: " << argv[0] << " <bmp file>" << endl;
		return true;
	}
	else if (check[size-1] != 'p' && check[size-2] != 'm' && check[size-3] != 'b')
	{
		cout << "Please, input bmp format picture" << endl;
		return true;
	}
	return false;
}
bool LoadSDLLib()
{
	if ( SDL_Init(SDL_INIT_VIDEO ) < 0 )
	{
		cerr << "Couldn't initialize SDL: " << SDL_GetError() << endl;
        	return false;
	}
	return true;
}
void makeScreen(SDL_Surface *& screen)
{
	// Set the window's caption and icon
	SDL_WM_SetCaption("Assessment2", "app.ico");
	#ifndef __DARWIN__
        	screen = SDL_SetVideoMode(SCR_WIDTH, SCR_HEIGHT, 0, 0);
	#else
		screen = SDL_SetVideoMode(SCR_WIDTH, SCR_HEIGHT, 24, SDL_HWSURFACE);
	#endif
	if (!screen)
	{
		cerr << "SDL: could not set video mode - exiting" << endl;
		quit(1);
	}
	return;
}
void loadBmp(SDL_Surface *& image, char *argv[])
{
	/* load BMP file */
	image = SDL_LoadBMP(argv[1]);
	if (image == NULL)
	{
		cout << "Can't load image of bmp: " << SDL_GetError() << endl;
		return;
	}
	return;
}
void keyEven(Uint8 *& keys, int & done, SDL_Event &event, SDL_Surface *& screen, SDL_Surface *& image, float &zoomFactor, bool zoom)
{
	// process the keyboard event
	while (!done)
	{
		clearAndBlitAndUpdate(screen, image);
		// Poll input queue, run keyboard loop
		while ( SDL_PollEvent(&event) )
		{
			if ( event.type == SDL_QUIT )
			{
				done = 1;
				break;
			}                    
		}
		keys = SDL_GetKeyState(NULL);
		if (keys[SDLK_q])
			done = 1;
		else if(keys[SDLK_UP])	//up arrow
			offsetY -= 5;
		else if(keys[SDLK_DOWN])	//down arrow
			offsetY += 5;
		else if(keys[SDLK_RIGHT])	//right arrow
			offsetX += 5;
		else if(keys[SDLK_LEFT])	//left arrow
			offsetX -= 5;
		else if (keys[SDLK_KP_MINUS] || keys[SDLK_MINUS]) //'-'	keypad minus
		{
			zoomFactor /= 2;
			image = SPG_CopySurface(zoomImage(image, zoomFactor));
			
		}
		else if (keys[SDLK_EQUALS] && (keys[SDLK_RSHIFT] || keys[SDLK_LSHIFT])) //'+'	keypad minus
		{
			zoomFactor *= 2;
			image = SPG_CopySurface(zoomImage(image, zoomFactor));
		}
		// Release CPU for others
		SDL_Delay(100);
	}
	return;
}
static inline SDL_Surface* SPG_CopySurface(SDL_Surface* src)
{
    return SDL_ConvertSurface(src, src->format, SDL_SWSURFACE);
}
SDL_Surface* zoomImage(SDL_Surface *& image, float zoomFactor)
{                             
	float sourceWidth = image->w;
	float sourceHeight = image->h;
	float scaledWidth = sourceWidth * zoomFactor;
	float scaledHeight = sourceHeight * zoomFactor;
	SDL_Surface* newImage = SDL_CreateRGBSurface(0, scaledWidth, scaledHeight, image->format->BitsPerPixel, image->format->Rmask, image->format->Gmask, image->format->Bmask, image->format->Amask);
	//find a ratio between the source image and the scaled image
	float ratioX = scaledWidth / sourceWidth;
	float ratioY = scaledHeight / sourceHeight;	
	bool isMasks = judgeFormat(image);
	int time = 0;
	if (isMasks == true)
	{
		if (judgeFormatHaveAlphaInfo(image))//pixel have any alpha information
			time = 4;
		else
			time = 3;	
	}
	else if (isMasks == false)
		time = 3;	
	// Then we need to loop through each scaled image pixel
	for(int i = 0; i < scaledWidth; i++)
	{
		for(int j = 0; j < scaledHeight; j++)
		{
			//set new RBG
			Uint8 red;
			Uint8 green;
			Uint8 blue;
			Uint8 alpha;
			//real number position on the source image
			float sourceX = static_cast<float>(i) / ratioX;
			float sourceY = static_cast<float>(j) / ratioY;
			//source the four pixels around our position
			int minX = floor(sourceX);
			int minY = floor(sourceY);
			int maxX = minX + 1;
			int maxY = minY + 1;
			//We need to check if the bottom-right pixel is out of the source image
			if(maxX >= sourceWidth)
			    maxX--;
			if(maxY >= sourceHeight)
			    maxY--;
			//distance between the top-left source pixel and the real number position
			float dX = sourceX - static_cast<float>(minX);
			float dY = sourceY - static_cast<float>(minY);		
			for(char k = 0; k < time; k++)
			{
				Uint8 component=
				(GetPixelComp(image, minX, minY, k) * (1 - dX) * (1 - dY) + // Top-Left
				GetPixelComp(image, maxX, minY, k) * dX * (1 - dY) + // Top-Right
				GetPixelComp(image, minX, maxY, k) * (1 - dX) * dY + // Bottom-Left
				GetPixelComp(image, maxX, maxY, k) * dX * dY); // Bottom-Right
				if (k == 0)	//red
					red = component;
				else if (k == 1)	//green
					green = component;
				else if (k == 2)	//blue
					blue = component;
				else if (k == 3)	//alpha
					alpha = component;
			}
			Uint32 newPixel;
			if (!judgeFormat(image))  //format palette
				newPixel = SDL_MapRGB(image->format, red, green, blue);
			if (judgeFormat(image)) //format masks
				newPixel = SDL_MapRGBA(image->format, red, green, blue, alpha);
			putpixel(newImage, i, j, newPixel);
		}
	}
	return newImage;
}
// Call this instead of exit(), so we can clean up SDL
static void quit(int rc)
{
    SDL_Quit();
    exit(rc);
}
void clearAndBlitAndUpdate(SDL_Surface *& screen, SDL_Surface *& image)
{
	SDL_Rect offset;
	offset.x = offsetX;
	offset.y = offsetY;
	// Clear screen
	SDL_FillRect(screen, NULL, 0);
	/* Blit image to the video surface */
	SDL_BlitSurface(image, NULL, screen, &offset);
	SDL_UpdateRect(screen, 0, 0, screen->w, screen->h);
	return;
}
void initArray(float *&data, int count)
{
	for (int i = 0; i < count; i++)
		data[i] = 0.0;	
	return;
}
bool judgeFormatHaveAlphaInfo(SDL_Surface *& image)
{
	if (image->format->Amask == 0)//pixel doesn't have any alpha information
		return false;
	return true;
}
void histogramEqualization(SDL_Surface *& image)
{
	int grayLevelValue = 0;
	float *Rpmf, *Gpmf, *Bpmf, *Apmf;
	float *oldRCDF, *oldGCDF, *oldBCDF, *oldACDF;
	float *newRCDF, *newGCDF, *newBCDF, *newACDF;
	int bitpp = image->format->BitsPerPixel;
	int channelNumber = 0;//story channel number
	//judge Format Have Alpha Info or not
	bool alphaHave = judgeFormatHaveAlphaInfo(image);
	if (alphaHave)//pixel have any alpha information
	{
		grayLevelValue = pow (2, bitpp / 4);
		channelNumber = 4;
	}
	else	//pixel does not have any alpha information
	{
		grayLevelValue = pow (2, bitpp / 3);
		channelNumber = 3;
	}
	Rpmf = new float[grayLevelValue];
	initArray(Rpmf, grayLevelValue);
	Gpmf = new float[grayLevelValue];
	initArray(Gpmf, grayLevelValue);
	Bpmf = new float[grayLevelValue];
	initArray(Bpmf, grayLevelValue);
	
	oldRCDF = new float[grayLevelValue];
	initArray(oldRCDF, grayLevelValue);
	oldGCDF = new float[grayLevelValue];
	initArray(oldGCDF, grayLevelValue);
	oldBCDF = new float[grayLevelValue];
	initArray(oldBCDF, grayLevelValue);
	
	newRCDF = new float[grayLevelValue];
	initArray(newRCDF, grayLevelValue);
	newGCDF = new float[grayLevelValue];
	initArray(newGCDF, grayLevelValue);
	newBCDF = new float[grayLevelValue];
	initArray(newBCDF, grayLevelValue);
	if (alphaHave)//pixel have any alpha information
	{
		Apmf = new float[grayLevelValue];
		initArray(Apmf, grayLevelValue);
		
		oldACDF = new float[grayLevelValue];
		initArray(oldACDF, grayLevelValue);
		
		newACDF = new float[grayLevelValue];
		initArray(newACDF, grayLevelValue);
	}
	//get frequency of gray level values
	for (int x = 0; x < image->h; x++)
	{
		for (int y = 0; y < image->w; y++)
		{
			Uint8 count = 0;
			//Get Red component
			count = GetPixelComp(image, x, y, 0);
			Rpmf[count] += 1;
			//Get Green component
			count = GetPixelComp(image, x, y, 1);
			Gpmf[count] += 1;
			// Get Blue component
			count = GetPixelComp(image, x, y, 2);
			Bpmf[count] += 1;
			// Get Alpha component
			if (alphaHave)//pixel have any alpha information
			{
				count = GetPixelComp(image, x, y, 3);
				Apmf[count] += 1;
			}
		}
	}
	//count total pixel number
	float totalCount = image->w * image->h;
	//get PMF(probability mass function)
	for (int x = 0; x < channelNumber; x++)
	{
		for (int i = 0; i < grayLevelValue; i++)
		{
			if (x == 0)	//red
				Rpmf[i] /= totalCount;
			else if (x == 1)	//green
				Gpmf[i] /= totalCount;
			else if (x == 2)	//blue
				Bpmf[i] /= totalCount;
			else if (x == 3)	//Alpha
				Apmf[i] /= totalCount;			
		}
	}	
	//count oldCDF (cumulative distributive function)
	for (int x = 0; x < channelNumber; x++)
	{
		for (int i = 0; i < grayLevelValue; i++)
		{
			if (x == 0)	//red
			{
				if (i == 0)
					oldRCDF[i] = Rpmf[i];
				else
					oldRCDF[i] = oldRCDF[i - 1] + Rpmf[i];
			}				
			else if (x == 1)	//green
			{
				if (i == 0)
					oldGCDF[i] = Gpmf[i];
				else
					oldGCDF[i] = oldGCDF[i - 1] + Gpmf[i];
			}
			else if (x == 2)	//blue
			{
				if (i == 0)
					oldBCDF[i] = Bpmf[i];
				else
					oldBCDF[i] = oldBCDF[i - 1] + Bpmf[i];
			}
			else if (x == 3)	//Alpha
			{
				if (i == 0)
					oldACDF[i] = Apmf[i];
				else
					oldACDF[i] = oldACDF[i - 1] + Apmf[i];
			}			
		}
	}
	//count new CDF (cumulative distributive function)
	for (int i = 0; i < grayLevelValue; i++)
	{
		newRCDF[i] = floor(oldRCDF[i] * (grayLevelValue - 1));
		newGCDF[i] = floor(oldGCDF[i] * (grayLevelValue - 1));
		newBCDF[i] = floor(oldBCDF[i] * (grayLevelValue - 1));
		if (alphaHave)//pixel have any alpha information
			newACDF[i] = floor(oldACDF[i] * (grayLevelValue - 1));
	}
	//set new RBG
	Uint8 red;
	Uint8 green;
	Uint8 blue;
	Uint8 alpha;
	for (int x = 0; x < image->h; x++)
	{
		for (int y = 0; y < image->w; y++)
		{
			//Get Red component
			red = GetPixelComp(image, x, y, 0);
			red = newRCDF[red];
			// Get Green component
			
			green = GetPixelComp(image, x, y, 1);
			green = newGCDF[green];
			// Get Blue component
			blue = GetPixelComp(image, x, y, 2);
			blue = newBCDF[blue];
			// Get Alpha component
			if (alphaHave)//pixel have any alpha information
			{
				alpha = GetPixelComp(image, x, y, 3);
				alpha = newACDF[alpha];
			}
			
			Uint32 newPixel;
			if (alphaHave)//pixel have any alpha information
				newPixel = SDL_MapRGBA(image->format, red, green, blue, alpha);
			else				
				newPixel = SDL_MapRGB(image->format, red, green, blue);
			putpixel(image, x, y, newPixel);
		}
	}
	delete []Rpmf;
	delete []Gpmf;
	delete []Bpmf;
	
	delete []oldRCDF;
	delete []oldGCDF;
	delete []oldBCDF;
	
	delete []newRCDF;
	delete []newGCDF;
	delete []newBCDF;
	if (alphaHave)//pixel have any alpha information
	{
		delete []newACDF;
		delete []oldACDF;
		delete []Apmf;
	}
	return;
}
Uint32 getpixel(SDL_Surface *& surface, int x, int y)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	switch(bpp)
	{
		case 1:
			return *p;
		case 2:
			return *(Uint16 *)p;
		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;
		case 4:
			return *(Uint32 *)p;
		default:
			return 0;       /* shouldn't happen, but avoids warnings */
	}
	return 0;
}
float GetPixelComp(SDL_Surface *& surface, int x, int y, int colorNum)
{	
	Uint8 index = surface->w * (x - 1) + y;
	SDL_Color * color;
	if (!judgeFormat(surface)) //format palette
	{
		/* Lock the surface */
		SDL_LockSurface(surface);
		color = &surface->format->palette->colors[index];
		/* Unlock the surface */
		SDL_UnlockSurface(surface);
		if (colorNum == 0)	//red
			return color->r;
		else if (colorNum == 1)	//green
			return color->g;
		else if (colorNum == 2)	//blue
			return color->b;
	}
	if (judgeFormat(surface)) //format masks
	{
		SDL_PixelFormat *fmt;
		Uint32 temp, pixel;		
		fmt = surface->format;
		SDL_LockSurface(surface);
		pixel = getpixel(surface, x, y);
		SDL_UnlockSurface(surface);
		if (colorNum == 0)/* Get Red component */
		{
			temp = pixel & fmt->Rmask;  /* Isolate red component */
			temp = temp >> fmt->Rshift; /* Shift it down to 8-bit */
			temp = temp << fmt->Rloss;  //Expand to a full 8-bit number
		}
		else if (colorNum == 1)
		{
			/* Get Green component */
			temp = pixel & fmt->Gmask;  /* Isolate green component */
			temp = temp >> fmt->Gshift; //Shift it down to 8-bit */
			temp = temp << fmt->Gloss;  //Expand to a full 8-bit number
		}
		else if (colorNum == 2)
		{
			/* Get Blue component */
			temp = pixel & fmt->Bmask;  /* Isolate blue component */
			temp = temp >> fmt->Bshift; /* Shift it down to 8-bit */
			temp = temp << fmt->Bloss;  //Expand to a full 8-bit number
		}
		else if (colorNum == 3)
		{
			/* Get Alpha component */
			temp = pixel & fmt->Amask;  /* Isolate alpha component */
			temp = temp >> fmt->Ashift; /* Shift it down to 8-bit */
			temp = temp << fmt->Aloss;  //Expand to a full 8-bit number
		}
		return temp;		
	}
	return -1;	
}
void putpixel(SDL_Surface *&surface, int x, int y, Uint32 pixel)
{
	int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to set */
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	switch(bpp)
	{
		case 1:
			*p = pixel;
			break;
		case 2:
			*(Uint16 *)p = pixel;
			break;
		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
			{
				p[0] = (pixel >> 16) & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = pixel & 0xff;
			}
			else
			{
				p[0] = pixel & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = (pixel >> 16) & 0xff;
			}
			break;
		case 4:
			*(Uint32 *)p = pixel;
			break;
	}
	return;
}
bool judgeFormat(SDL_Surface *& surface)
{
	if (surface->format->Rmask != 0 || surface->format->Gmask != 0 || surface->format->Bmask != 0 || surface->format->Amask != 0) //format masks
		return true;
	else
		return false;
}
