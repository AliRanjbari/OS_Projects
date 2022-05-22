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

const int N_THREADS = 4;
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
          if(picIsOriginal)
            fileBuffer[bufferSize - count] = pic[i][j].r;
          else
            fileBuffer[bufferSize - count] = tempPic[i][j].r;
          break;
        case 1:
          if(picIsOriginal)
            fileBuffer[bufferSize - count] = pic[i][j].g;
          else
            fileBuffer[bufferSize - count] = tempPic[i][j].g;
          break;
        case 2:
          if(picIsOriginal)
            fileBuffer[bufferSize - count] = pic[i][j].b;
          else
            fileBuffer[bufferSize - count] = tempPic[i][j].b;
          break;
        }
        count++;
      }
  }
  write.write(fileBuffer, bufferSize);
  pthread_exit(NULL);
}

RGB median(int x, int y, PICTURE& pic) {
  int _x, _y, size = 0;
  vector<DWORD> reds;
  vector<DWORD> greens;
  vector<DWORD> blues;
  for(auto dir : DIRS) {
    _x = x + dir.first;
    _y = y + dir.second;
    if(0 <= _x && _x < rows && 0 <= _y && _y < cols) {
      reds.push_back(pic[_x][_y].r);
      greens.push_back(pic[_x][_y].g);
      blues.push_back(pic[_x][_y].b);
      size++;
    }
  }
  std::sort(reds.begin(), reds.end());  
  std::sort(greens.begin(), greens.end());  
  std::sort(blues.begin(), blues.end());

  return RGB{reds[size/2], greens[size/2], blues[size/2]};
}

void *horizontalMirrorFilter(void* id) {
  intptr_t thread_id = (intptr_t)id;
  int start = rows / N_THREADS * (thread_id);
  int end = rows / N_THREADS * (thread_id+1);
  end = (thread_id+1==N_THREADS) ? rows : end;

  for(int x=start; x < end; x++)
    for(int y=0; y < cols; y++)
      if(picIsOriginal)
        tempPic[x][y] = pic[x][cols-y-1];
      else
        pic[x][y] = tempPic[x][cols-y-1];

  pthread_exit(NULL);
}

void *verticalMirrorFilter(void* id) {
  intptr_t thread_id = (intptr_t)id;
  int start = rows / N_THREADS * (thread_id);
  int end = rows / N_THREADS * (thread_id+1);
  end = (thread_id+1==N_THREADS) ? rows : end;

  for(int x=start; x < end; x++)
    for(int y=0; y < cols; y++)
      if(picIsOriginal)
        tempPic[x][y] = pic[rows-x-1][y];
      else
        pic[x][y] = tempPic[rows-x-1][y];

  pthread_exit(NULL);
}

void *medianFilter(void* id) {
  intptr_t thread_id = (intptr_t)id;
  int start = rows / N_THREADS * (thread_id);
  int end = rows / N_THREADS * (thread_id+1);
  end = (thread_id+1==N_THREADS) ? rows : end;
  
  for(int x=start; x < end; x++)
    for(int y=0; y < cols; y++)
      if(picIsOriginal)
        tempPic[x][y] = median(x, y, pic);
      else
        pic[x][y] = median(x, y, tempPic);

  pthread_exit(NULL);
}

void *inverseColorsFilter(void* id) {
  intptr_t thread_id = (intptr_t)id;
  int start = rows / N_THREADS * (thread_id);
  int end = rows / N_THREADS * (thread_id+1);
  end = (thread_id+1==N_THREADS) ? rows : end;
  
  for(int x=start; x < end; x++)
    for(int y=0; y < cols; y++) 
      if(picIsOriginal) {
        tempPic[x][y].r = 255 - pic[x][y].r;
        tempPic[x][y].g = 255 - pic[x][y].g;
        tempPic[x][y].b = 255 - pic[x][y].b;
      }
      else {
        pic[x][y].r = 255 - tempPic[x][y].r;
        pic[x][y].g = 255 - tempPic[x][y].g;
        pic[x][y].b = 255 - tempPic[x][y].b;
      }
    
}

void addPlusSign(PICTURE& input) {
  for(int x=0; x < rows; x++) {
    input[x][cols/2+1] = RGB{255, 255, 255};
    input[x][cols/2] = RGB{255, 255, 255};
    input[x][cols/2-1] = RGB{255, 255, 255};
  }
  for(int y=0; y < cols; y++) {
    input[rows/2+1][y] = RGB{255, 255, 255};
    input[rows/2][y] = RGB{255, 255, 255};
    input[rows/2-1][y] = RGB{255, 255, 255};
  }
}


auto current_time() {
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

int main(int argc, char *argv[])
{

  auto timeStart = current_time();

  char *fileName = argv[1];
  if (!fillAndAllocate(fileBuffer, fileName, rows, cols, bufferSize))
  {
    cout << "File read error" << endl;
    return 1;
  }

  pthread_t threads[N_THREADS];
  cout << "**Program is running with " << N_THREADS << " threads**\n";


  // ..........................................
  auto t1 = current_time();
  for(int i=0; i < N_THREADS; i++)
    pthread_create(&threads[i], NULL, getPixlesFromBMP24, (void*)i);
  for(int i=0; i<N_THREADS; i++)
    pthread_join(threads[i], NULL);
  auto t2 = current_time();
  cout << "Read from file: Done in " << t2 - t1 << " ms\n";

  cout << "Filters: \n";
  // ..........................................
  t1 = current_time();
  for(int i=0; i < N_THREADS; i++)
    pthread_create(&threads[i], NULL, horizontalMirrorFilter, (void*)i);
  for(int i=0; i<N_THREADS; i++)
    pthread_join(threads[i], NULL);
  picIsOriginal = !picIsOriginal;
  t2 = current_time();
  cout << "--> Horizontal Mirror: Done in " << t2 - t1 << " ms\n";

  // ..........................................
  t1 = current_time();
  for(int i=0; i < N_THREADS; i++)
    pthread_create(&threads[i], NULL, verticalMirrorFilter, (void*)i);
  for(int i=0; i<N_THREADS; i++)
    pthread_join(threads[i], NULL);
  picIsOriginal = !picIsOriginal;
  t2 = current_time();
  cout << "--> Vertical Mirror: Done in " << t2 - t1 << " ms\n";


  // ..........................................
  t1 = current_time();
  for(int i=0; i < N_THREADS; i++)
    pthread_create(&threads[i], NULL, medianFilter, (void*)i);
  for(int i=0; i<N_THREADS; i++)
    pthread_join(threads[i], NULL);
  picIsOriginal = !picIsOriginal;
  t2 = current_time();
  cout << "--> Median : Done in " << t2 - t1 << " ms\n";

  // ..........................................
  t1 = current_time();
  for(int i=0; i < N_THREADS; i++)
    pthread_create(&threads[i], NULL, inverseColorsFilter, (void*)i);
  for(int i=0; i<N_THREADS; i++)
    pthread_join(threads[i], NULL);
  picIsOriginal = !picIsOriginal;
  t2 = current_time();
  cout << "--> Inverse Colors: Done in " << t2 - t1 << " ms\n";


  // ..........................................
  t1 = current_time();
  if(picIsOriginal)
    addPlusSign(pic);
  else
    addPlusSign(tempPic);
  t2 = current_time();
  cout << "--> Added Plus sign: Done in " << t2 - t1 << " ms\n";


  // ..........................................
  t1 = current_time();
  for(int i=0; i < N_THREADS; i++)
    pthread_create(&threads[i], NULL, writeOutBmp24, (void*)i);
  for(int i=0; i<N_THREADS; i++)
    pthread_join(threads[i], NULL);
  t2 = current_time();
  cout << "Wrote bmp: Done in " << t2 - t1 << " ms\n";


  auto timeEnd = current_time();
  cout << "------------------------------\n";
  cout << "Execution Time: " << timeEnd - timeStart << " ms\n";

  return 0;
}