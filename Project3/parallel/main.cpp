#include <iostream>
#include <unistd.h>
#include <fstream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <pthread.h>


using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::vector;
using std::chrono::milliseconds;
using std::chrono::system_clock;
using std::chrono::duration_cast;

#pragma pack(1)
#pragma once

typedef int LONG;
typedef unsigned short WORD;
typedef unsigned int DWORD;

typedef struct tagBITMAPFILEHEADER
{
  WORD bfType;
  DWORD bfSize;
  WORD bfReserved1;
  WORD bfReserved2;
  DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
  DWORD biSize;
  LONG biWidth;
  LONG biHeight;
  WORD biPlanes;
  WORD biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  LONG biXPelsPerMeter;
  LONG biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

typedef struct rgb {
  DWORD r;
  DWORD g;
  DWORD b;
} RGB;

typedef vector<vector<RGB>> PICTURE;

const int N_THREADS = 1;
const int MAX_SIZE = 1080;
const char OUTPUT_NAME[] = "output.bmp";
const vector<std::pair<int, int>> DIRS = {{1,0},{1,1},{0,1},{-1,1},
                                          {-1, 0},{-1,-1},{0,-1},
                                          {1,-1}};

PICTURE pic(MAX_SIZE, vector<RGB>(MAX_SIZE, RGB{0, 0, 0}));
PICTURE tempPic(MAX_SIZE, vector<RGB>(MAX_SIZE, RGB{0, 0, 0}));
bool picIsOriginal = true;

int rows;
int cols;
char *fileBuffer;
int bufferSize;



bool fillAndAllocate(char *&buffer, const char *fileName, int &rows, int &cols, int &bufferSize)
{
  std::ifstream file(fileName);

  if (file)
  {
    file.seekg(0, std::ios::end);
    std::streampos length = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer = new char[length];
    file.read(&buffer[0], length);

    PBITMAPFILEHEADER file_header;
    PBITMAPINFOHEADER info_header;

    file_header = (PBITMAPFILEHEADER)(&buffer[0]);
    info_header = (PBITMAPINFOHEADER)(&buffer[0] + sizeof(BITMAPFILEHEADER));
    rows = info_header->biHeight;
    cols = info_header->biWidth;
    bufferSize = file_header->bfSize;
    return 1;
  }
  else 
  {
    cout << "File" << fileName << " doesn't exist!" << endl;
    return 0;
  }
}


void *getPixlesFromBMP24(void* id)
{
  intptr_t thread_id = (intptr_t)id;
  int start = rows / N_THREADS * (thread_id);
  int end = rows / N_THREADS * (thread_id+1);
  end = (thread_id+1==N_THREADS) ? rows : end;
  int count = 1 + (start * 3 * cols);
  int extra = cols % 4;
  for (int i = start; i < end; i++)
  {
    count += extra;
    for (int j = cols - 1; j >= 0; j--)
      for (int k = 0; k < 3; k++)
      {
        switch (k)
        {
        case 0:
          pic[i][j].r = fileBuffer[bufferSize - count];
          tempPic[i][j].r = fileBuffer[bufferSize - count];
          break;
        case 1:
          pic[i][j].g = fileBuffer[bufferSize - count];
          tempPic[i][j].g = fileBuffer[bufferSize - count];
          break;
        case 2:
          pic[i][j].b = fileBuffer[bufferSize - count];
          tempPic[i][j].b = fileBuffer[bufferSize - count];
          break;
        }
        count++;
      }
  }
  pthread_exit(NULL);
}

void *writeOutBmp24(void* id)
{
  std::ofstream write(OUTPUT_NAME);
  if (!write)
  {
    cout << "Failed to write " << OUTPUT_NAME << endl;
    pthread_exit(NULL);
  }

  intptr_t thread_id = (intptr_t)id;
  int start = rows / N_THREADS * (thread_id);
  int end = rows / N_THREADS * (thread_id+1);
  end = (thread_id+1==N_THREADS) ? rows : end;
  int count = 1 + (start * 3 * cols);
  int extra = cols % 4;
  for (int i = start; i < end; i++)
  {
    count += extra;
    for (int j = cols - 1; j >= 0; j--)
      for (int k = 0; k < 3; k++)
      {
        switch (k)
        {
        case 0:
          fileBuffer[bufferSize - count] = pic[i][j].r;
          break;
        case 1:
          fileBuffer[bufferSize - count] = pic[i][j].g;
          break;
        case 2:
          fileBuffer[bufferSize - count] = pic[i][j].b;
          break;
        }
        count++;
      }
  }
  write.write(fileBuffer, bufferSize);
  pthread_exit(NULL);
}


auto current_time() {
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

int main(int argc, char *argv[])
{
  char *fileName = argv[1];
  if (!fillAndAllocate(fileBuffer, fileName, rows, cols, bufferSize))
  {
    cout << "File read error" << endl;
    return 1;
  }

  pthread_t threads[N_THREADS];


  // ..........................................
  auto t1 = current_time();
  for(int i=0; i < N_THREADS; i++)
    pthread_create(&threads[i], NULL, getPixlesFromBMP24, (void*)i);
  for(int i=0; i<N_THREADS; i++)
    pthread_join(threads[i], NULL);
  auto t2 = current_time();
  cout << "Read from file: Done in " << t2 - t1 << " ms\n";

  // ..........................................
  t1 = current_time();
  for(int i=0; i < N_THREADS; i++)
    pthread_create(&threads[i], NULL, writeOutBmp24, (void*)i);
  for(int i=0; i<N_THREADS; i++)
    pthread_join(threads[i], NULL);
  t2 = current_time();
  cout << "Wrote bmp: Done in " << t2 - t1 << " ms\n";


  return 0;
}