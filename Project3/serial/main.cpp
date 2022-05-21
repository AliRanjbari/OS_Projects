#include <iostream>
#include <unistd.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <chrono>

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

typedef struct rgb
{
  DWORD r;
  DWORD g;
  DWORD b;

} RGB;

typedef vector<vector<RGB>> PICTURE;

const vector<std::pair<int, int>> DIRS = {{1,0},{1,1},{0,1},{-1,1},
                                          {-1, 0},{-1,-1},{0,-1},
                                          {1,-1}};

PICTURE pic;
PICTURE tempPic;
bool picIsOriginal = true;

int rows;
int cols;

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


void getPixlesFromBMP24(int end, int rows, int cols, char *fileReadBuffer)
{
  int count = 1;
  int extra = cols % 4;
  for (int i = 0; i < rows; i++)
  {
    count += extra;
    pic.push_back(vector<RGB>(cols, RGB{0, 0, 0}));
    tempPic.push_back(vector<RGB>(cols, RGB{0, 0, 0}));
    for (int j = cols - 1; j >= 0; j--)
      for (int k = 0; k < 3; k++)
      {
        switch (k)
        {
        case 0:
          pic[i][j].r = fileReadBuffer[end - count];
          tempPic[i][j].r = fileReadBuffer[end - count];
          break;
        case 1:
          pic[i][j].g = fileReadBuffer[end - count];
          tempPic[i][j].g = fileReadBuffer[end - count];
          break;
        case 2:
          pic[i][j].b = fileReadBuffer[end - count];
          tempPic[i][j].b = fileReadBuffer[end - count];
          break;
        }
        count++;
      }
  }
}

void writeOutBmp24(char *fileBuffer, const char *nameOfFileToCreate, int bufferSize)
{
  std::ofstream write(nameOfFileToCreate);
  if (!write)
  {
    cout << "Failed to write " << nameOfFileToCreate << endl;
    return;
  }
  int count = 1;
  int extra = cols % 4;
  for (int i = 0; i < rows; i++)
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

void horizontalMirrorFilter(PICTURE& input, PICTURE& output) {
  picIsOriginal = !picIsOriginal;
  for(int x=0; x < rows; x++)
    for(int y=0; y < cols; y++)
      output[x][y] = input[x][cols-y-1];
}

void verticalMirrorFilter(PICTURE& input, PICTURE& output) {
  picIsOriginal = !picIsOriginal;
  for(int x=0; x < rows; x++)
    for(int y=0; y < cols; y++)
      output[x][y] = input[rows-x-1][y];
}

void medianFilter(PICTURE& input, PICTURE& output) {
  picIsOriginal = !picIsOriginal;
  for(int x=0; x < rows; x++)
    for(int y=0; y < cols; y++)
      output[x][y] = median(x, y, input);
}

void inverseColorsFilter(PICTURE& input, PICTURE& output) {
  picIsOriginal = !picIsOriginal;
  for(int x=0; x < rows; x++)
    for(int y=0; y < cols; y++) {
      output[x][y].r = 255 - input[x][y].r;
      output[x][y].g = 255 - input[x][y].g;
      output[x][y].b = 255 - input[x][y].b;
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
  char *fileBuffer;
  int bufferSize;
  char *fileName = argv[1];
  if (!fillAndAllocate(fileBuffer, fileName, rows, cols, bufferSize))
  {
    cout << "File read error" << endl;
    return 1;
  }
  auto t1 = current_time();
  getPixlesFromBMP24(bufferSize, rows, cols, fileBuffer);
  auto t2 = current_time();
  cout << "Read from file: Done in " << t2 - t1 << " ms\n";

  t1 = current_time();
  cout << "Filters: \n";
  horizontalMirrorFilter(pic, tempPic);
  t2 = current_time();
  cout << "--> Horizontal Mirror: Done in " << t2 - t1 << " ms\n";
  t1 = current_time();
  verticalMirrorFilter(tempPic, pic);
  t2 = current_time();
  cout << "--> Vertical Mirror Done in " << t2 - t1 << " ms\n";
  t1 = current_time();
  medianFilter(pic, tempPic);
  t2 = current_time();
  cout << "--> Median: Done in " << t2 - t1 << " ms\n";
  t1 = current_time();
  inverseColorsFilter(tempPic, pic);
  t2 = current_time();
  cout << "--> Inverse Colors: Done in " << t2 - t1 << " ms\n";
  t1 = current_time();
  addPlusSign(pic);
  t2 = current_time();
  cout << "--> Added Plus sign: Done in " << t2 - t1 << " ms\n";

  if(!picIsOriginal)
    for(int i=0; i<rows; i++)
      for(int j=0; j<cols; j++)
        pic[i][j] = tempPic[i][j];

  char outputName[] = "output.bmp";
  t1 = current_time();
  writeOutBmp24(fileBuffer, outputName, bufferSize);
  t2 = current_time();
  cout << "Wrote bmp: Done in " << t2 - t1 << " ms\n";

  

  auto timeEnd = current_time();
  cout << "------------------------------\n";
  cout << "Execution Time: " << timeEnd - timeStart << " ms \n";
  return 0;
}