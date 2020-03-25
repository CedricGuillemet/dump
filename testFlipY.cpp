// testFlipY.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <stdint.h>
#include <chrono>

void FlipYInImageBytes(uint8_t* bytes, size_t rowCount, size_t rowPitch)
{
  std::vector<uint8_t> buffer{};
  buffer.reserve(rowPitch);

  for (size_t row = 0; row < rowCount / 2; row++)
  {
    auto frontPtr = bytes + (row * rowPitch);
    auto backPtr = bytes + ((rowCount - row - 1) * rowPitch);

    std::memcpy(buffer.data(), frontPtr, rowPitch);
    std::memcpy(frontPtr, backPtr, rowPitch);
    std::memcpy(backPtr, buffer.data(), rowPitch);
  }
}

// Proposal1: Justin's solution : 10 times slower
void FlipYInImageBytesProposal1(uint8_t* bytes, size_t rowCount, size_t rowPitch)
{
  std::vector<uint8_t> buffer{};
  static int halfSize = (rowCount / 2) * rowPitch;
  buffer.reserve(halfSize);

  memcpy(buffer.data(), bytes, halfSize);
  // 1st pass :  copy bottom to top
  for (size_t row = 0; row < rowCount / 2; row++)
  {
    std::memcpy(bytes + rowPitch * row, bytes + (rowCount - 1 - row) * rowPitch, rowPitch);
  }
  // 2nd pass : copy temp to bottom
  for (size_t row = 0; row < rowCount / 2; row++)
  {
    std::memcpy(bytes + (rowCount - 1 - row) * rowPitch, buffer.data() + row * rowPitch, rowPitch);
  }
}

// Proposal1 with simpler pointer arithmetics : 2% improvements
void FlipYInImageBytesProposal1b(uint8_t* bytes, size_t rowCount, size_t rowPitch)
{
  std::vector<uint8_t> buffer{};
  static int halfSize = (rowCount / 2) * rowPitch;
  buffer.reserve(halfSize);

  memcpy(buffer.data(), bytes, halfSize);
  // 1st pass :  copy bottom to top
  uint8_t* ptrDst = bytes;
  uint8_t* ptrSrc = bytes + (rowCount - 1) * rowPitch;
  for (size_t row = 0; row < rowCount / 2; row++)
  {
    std::memcpy(ptrDst, ptrSrc, rowPitch);
    ptrDst += rowPitch;
    ptrSrc -= rowPitch;
  }
  // 2nd pass : copy temp to bottom
  ptrDst = bytes + (rowCount - 1) * rowPitch;
  ptrSrc = buffer.data();
  for (size_t row = 0; row < rowCount / 2; row++)
  {
    std::memcpy(ptrDst, ptrSrc, rowPitch);
    ptrDst -= rowPitch;
    ptrSrc += rowPitch;
  }
}

// Proposal1 with arithmetics and static vector, so no allocation at each call : 50% improvement but still 5 times slower
void FlipYInImageBytesProposal1c(uint8_t* bytes, size_t rowCount, size_t rowPitch)
{
  static std::vector<uint8_t> buffer{};
  static int halfSize = (rowCount / 2) * rowPitch;
  buffer.reserve(halfSize);

  memcpy(buffer.data(), bytes, halfSize);
  // 1st pass :  copy bottom to top
  uint8_t* ptrDst = bytes;
  uint8_t* ptrSrc = bytes + (rowCount - 1) * rowPitch;
  for (size_t row = 0; row < rowCount / 2; row++)
  {
    std::memcpy(ptrDst, ptrSrc, rowPitch);
    ptrDst += rowPitch;
    ptrSrc -= rowPitch;
  }
  // 2nd pass : copy temp to bottom
  ptrDst = bytes + (rowCount - 1) * rowPitch;
  ptrSrc = buffer.data();
  for (size_t row = 0; row < rowCount / 2; row++)
  {
    std::memcpy(ptrDst, ptrSrc, rowPitch);
    ptrDst -= rowPitch;
    ptrSrc += rowPitch;
  }
}

// dumb byte swap : Ouch!
void FlipYInImageBytesByteSwap(uint8_t* bytes, size_t rowCount, size_t rowPitch)
{
  for (size_t row = 0; row < rowCount / 2; row++)
  {
    auto frontPtr = bytes + (row * rowPitch);
    auto backPtr = bytes + ((rowCount - row - 1) * rowPitch);

    for (int i = 0; i<rowPitch;i++)
    {
      uint8_t temp = frontPtr[i];
      frontPtr[i] = backPtr[i];
      backPtr[i] = temp;
    }
  }
}

void FlipYInImageBytesByteSwapNoTemp(uint8_t* bytes, size_t rowCount, size_t rowPitch)
{
  for (size_t row = 0; row < rowCount / 2; row++)
  {
    auto frontPtr = bytes + (row * rowPitch);
    auto backPtr = bytes + ((rowCount - row - 1) * rowPitch);

    for (int i = 0; i < rowPitch; i++)
    {
      uint8_t x = frontPtr[i];
      uint8_t y = backPtr[i];

      x = x ^ y;
      y = x ^ y;
      x = x ^ y;
      frontPtr[i] = x;
      backPtr[i] = y;
    }
  }
}

// dumb uint64_t swap :  closer to reference
void FlipYInImageBytes64bSwap(uint8_t* bytes, size_t rowCount, size_t rowPitch)
{
  for (size_t row = 0; row < rowCount / 2; row++)
  {
    uint64_t* frontPtr = (uint64_t *)(bytes + (row * rowPitch));
    uint64_t* backPtr = (uint64_t *)(bytes + ((rowCount - row - 1) * rowPitch));

    for (int i = 0; i < rowPitch / 8; i++)
    {
      uint64_t temp = frontPtr[i];
      frontPtr[i] = backPtr[i];
      backPtr[i] = temp;
    }
  }
}


void FlipYInImageBytes64bSwapNoTemp(uint8_t* bytes, size_t rowCount, size_t rowPitch)
{
  for (size_t row = 0; row < rowCount / 2; row++)
  {
    uint64_t* frontPtr = (uint64_t*)(bytes + (row * rowPitch));
    uint64_t* backPtr = (uint64_t*)(bytes + ((rowCount - row - 1) * rowPitch));

    for (int i = 0; i < rowPitch / 8; i++)
    {
      uint64_t x = frontPtr[i];
      uint64_t y = backPtr[i];

      x = x ^ y;
      y = x ^ y;
      x = x ^ y;
      frontPtr[i] = x;
      backPtr[i] = y;
    }
  }
}

typedef void (*FlipFunction)(uint8_t* bytes, size_t rowCount, size_t rowPitch);

void Benchmark(const char* label, FlipFunction function, uint8_t* image, int size, int pixel)
{
  static std::chrono::duration<double> time_span;
  auto tstart = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < 10000; i++)
  {
    function(image, size, size * pixel);
  }
  auto tend = std::chrono::high_resolution_clock::now();
  time_span = std::chrono::duration_cast<std::chrono::duration<double>>(tend - tstart);
  std::cout << label << time_span.count() << std::endl;
}

int main()
{
  static const int size = 1024;
  static const int pixel = 4;

  uint8_t*image = (uint8_t*)malloc(size * size * pixel);

  struct Bench
  {
    const char* label;
    FlipFunction function;
  };

  Bench bench[] = { 
    {"time 3n/2 : ", FlipYInImageBytes},
    {"time proposal 1 : ", FlipYInImageBytesProposal1},
    {"time proposal 1b : ", FlipYInImageBytesProposal1b},
    {"time proposal 1c : ", FlipYInImageBytesProposal1c},
    {"time proposal byteswap : ", FlipYInImageBytesByteSwap},
    {"time proposal byteswap no temp: ", FlipYInImageBytesByteSwapNoTemp},
    {"time proposal uint64_t swap : ", FlipYInImageBytes64bSwap},
    {"time proposal uint64_t swap no temp : ", FlipYInImageBytes64bSwapNoTemp},
  };

  for (auto& benching : bench)
  {
    Benchmark(benching.label, benching.function, image, size, pixel);
  }
}
