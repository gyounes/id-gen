//-------------------------------------------------------------------
//
// File:      id-gen.c
//
// @author    Georges Younes <georges.r.younes@gmail.com>
//
// @copyright 2016-2017 Georges Younes
//
// This file is provided to you under the Apache License,
// Version 2.0 (the "License"); you may not use this file
// except in compliance with the License.  You may obtain
// a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License. 
//
//
//-------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define N127 0x7f
#define N128 0x80
#define Compression 1

typedef struct _ByteArray{
  size_t len; /**< Number of bytes in the `data` field. */
  uint8_t* data; /**< Pointer to an allocated array of data bytes. */
} ByteArray;

typedef struct {
  ByteArray *ba;
  size_t used;
  size_t size;
} Array;

struct node {
    ByteArray ba;
    struct node *next;
};
typedef struct node node;

///// ByteArray Functions

void printByteArray(ByteArray ba){
  printf("Byte array is: ");
  for (int i = 0; i < ba.len; ++i)
    printf("%x ", ba.data[i]);
  printf("\n");
}

ByteArray decompress(ByteArray compba){
  ByteArray ba;
  ba.len = compba.len;
  ba.data = malloc(ba.len);
  int ctr, sum, k=0;
  for (int i = 0; i < compba.len; ++i){
    ctr = 0;
    sum = 0;
    if (compba.data[i] >= N128){
      while(compba.data[i] >= N128){
        ctr++;
        if (ctr == 1)
          sum  = compba.data[i] - N128;
        else
        sum = sum * N128 + compba.data[i] - N128;
        i++;
      };
      ba.len = ba.len - ctr + sum;
      ba.data = realloc(ba.data, ba.len);
      for (int j = k; j < k+sum; ++j)
        ba.data[j] = N127;
      k = k + sum;
      --i;
    }
    else{
      ba.data[k]=compba.data[i];
      k++;
    }  
  }
  return ba;
}

int isTopVal(ByteArray ba){
  return (ba.len == 1 && ba.data[0]==N128);
}

int getNumberOfSevenBits(int num){
  int count = 0;
  while (num > 0){
    count++;
    num = num >> 7;
  }
  return count;
}

ByteArray compress(ByteArray ba){
  ByteArray compba;
  compba.len = ba.len;
  compba.data = malloc(compba.len);
  int ctr, sum, k=0;
  for (int i = 0; i < ba.len; ++i){
    ctr = 0;
    sum = 0;
    if (ba.data[i] == N127){
      while(ba.data[i] == N127){
        ctr++;
        i++;
      };
      sum = getNumberOfSevenBits(ctr);
      compba.len = compba.len - ctr + sum;
      compba.data = realloc(compba.data, compba.len);
      for (int j = k; j < k+sum-1; ++j)
        compba.data[j] = ctr/N128 + N128;
      compba.data[k+sum-1] = ctr % N128 + N128;
      k = k + sum;
      --i;
    }
    else{
      compba.data[k]=ba.data[i];
      k++;
    }  
  }
  return compba;
}

int is_full(ByteArray ba, int start){
  for (int i = start; i < ba.len-1; ++i)
    if (ba.data[i] != N127)
      return 0;
  return 1;
}

ByteArray incrementByteArray(ByteArray ba){
  ByteArray newba;
  if (ba.data[ba.len-1] == N127){
    newba.len = ba.len+1;
    newba.data = malloc(newba.len);
    for (int i = 0; i < ba.len; ++i)
      newba.data[i] = ba.data[i];
    newba.data[newba.len-1] = 0x01;
  }
  else{
    newba.len = ba.len; 
    newba.data = malloc(newba.len);
    for (int i = 0; i < ba.len; ++i)
      newba.data[i] = ba.data[i];
    newba.data[newba.len-1]++;
  }
  return newba;
}

ByteArray ByteArray_GenerateBetween(ByteArray ba1, ByteArray ba2, int withCompression){
  if(withCompression){
    if (!isTopVal(ba1))
      ba1 = decompress(ba1);
    if (!isTopVal(ba2))
      ba2 = decompress(ba2);
  }
  assert(lessThan(ba1, ba2));
  ByteArray res;

  for (int i = 0; i < ba1.len; ++i){
    uint8_t diff = ba2.data[i] - ba1.data[i];
    if (diff == 0){
      if (ba1.len > i+1)
        continue;
      else{
        if (ba2.data[i+1] == 0x01){
          res.len = i+3;
          res.data = malloc(res.len);
          for (int j = 0; j <= i; ++j)
            res.data[j] = ba2.data[j];
          res.data[i+1] = 0x00;
          res.data[i+2] = 0x40;
        }
        else{
          res.len = i+2;
          res.data = malloc(res.len);
          for (int j = 0; j <= i; ++j)
            res.data[j] = ba2.data[j];
          res.data[i+1] = (ba2.data[i+1]+1)/2;
        }
        break;
      }
    }
    else if (diff == 1){
      if ((ba2.len-i>1 && ba1.len-i==1) || (ba2.len-i==1 && ba1.len-i >1 && is_full(ba1, i+1))){
        //increment
        res = incrementByteArray(ba1);
      }
      else{
        // append
        if (ba2.len-i>1){
          res.len = i+1;
          res.data = malloc(res.len);
          for (int j = 0; j <= i; ++j)
            res.data[j] = ba2.data[j]; 
        }
        else{
          res.len = i+2;
          res.data = malloc(res.len);
          for (int j = 0; j <= i; ++j)
            res.data[j] = ba1.data[j];
          res.data[i+1] = 0x40;
        }
      }
      break;
    }
    else{ 
      //new tab size is always 1
      if (ba1.len - i > 1){
        //divide
        res.len = i+1;
        res.data = malloc(res.len);
        for (int j = 0; j < i; ++j)
          res.data[j] = ba1.data[j];
        // res.data[i] = (uint8_t)ceil((double)(ba2.data[i]+ba1.data[i])/2);
        res.data[i] = (ba2.data[i]+ba1.data[i]+1)/2;
      }
      else{
        //increment
        res = incrementByteArray(ba1);
      }
      break;
    }
  }
  assert(lessThan(ba1, res));
  assert(lessThan(res, ba2));
  if(withCompression)
    res = compress(res);
  return res;
}

int compare(ByteArray a, ByteArray b)
{
  for (int i = 0; i < MIN(a.len, b.len); ++i)
    if(a.data[i] < b.data[i])
      return -1;
    else if(a.data[i] > b.data[i])
      return 1;
    else
      continue;
  if(a.len<b.len)
    return -1;
  else
    return 0;
}

int lessThan(ByteArray a, ByteArray b){
  return compare(a, b) == -1;
}

int greaterThan(ByteArray a, ByteArray b){
  return compare(a, b) == 1;
}

int equalsTo(ByteArray a, ByteArray b){
  return compare(a, b) == 0;
}

///// end of ByteArray

///// Sequence imlpemented as growable array

void initArray(Array *a, size_t initialSize) {
  a->ba = malloc(initialSize * sizeof(ByteArray));
  a->used = 0;
  a->size = initialSize;
}

ByteArray GenerateIdAt(Array *a, int pos) {
  ByteArray bal, bar;
  bal.len = 1;
  bal.data = malloc(bal.len);
  bal.data[0] = 0x01;
  bar.len = 1;
  bar.data = malloc(bar.len);
  bar.data[0] = 0x80;
  if (a->used == 0) // empty Array
    return ByteArray_GenerateBetween(bal, bar, Compression);
  else // not empty Array
    if (pos == 0) // insert in the begining
      return ByteArray_GenerateBetween(bal, a->ba[pos], Compression);
    else if (pos == a->used) // insert in the end
      return ByteArray_GenerateBetween(a->ba[pos-1], bar, Compression);
    else
      return ByteArray_GenerateBetween(a->ba[pos-1], a->ba[pos], Compression);
}

void insertArrayAt(Array *a, int pos) {
  // a->used is the number of used entries, because a->ba[a->used++] updates a->used only *after* the array has been accessed.
  // Therefore a->used can go up to a->size 
  if (pos <= a->used+1) {
    if (a->used == a->size) {
      a->size *= 2;
      a->ba = realloc(a->ba, a->size * sizeof(ByteArray));
    }
    // shift values right
    if (a->used > 0 || pos < a->used)
      for (int i = a->used-1; i >= pos; --i)
        a->ba[i+1] = a->ba[i];
    ByteArray element = GenerateIdAt(a, pos);
    a->ba[pos] = element;
    ++a->used;
  }
  else
    printf("Position is out of bounds\n");
}

void printArray(Array *a) {
  for (int i = 0; i < a->used; ++i)
    printByteArray(a->ba[i]);
}

void deleteArrayAt(Array *a, int pos) {
  if (pos <= a->used) {
    // shift values left
    for (int i = pos; i < a->size-1; ++i)
      a->ba[i] = a->ba[i+1];
    --a->size;
    --a->used;
    a->ba = realloc(a->ba, a->size * sizeof(ByteArray));
  }
  else
    printf("Position is out of bounds\n");
}

void freeArray(Array *a) {
  free(a->ba);
  a->ba = NULL;
  a->used = a->size = 0;
}

void printArrayBytes(Array *a){
  int size = 10;
  int b[size];
  for (int i = 0; i < size; ++i)
    b[i] = 0;
  for (int i = 0; i < a->used; ++i)
    b[a->ba[i].len-1]++;
  for (int i = 0; i < size; ++i)
    printf("ids of size %d Byte(s): %d\n", i+1, b[i]);
}

void randomInsertTest(int max){
  Array a;
  initArray(&a, max);
  while(max > 0){
    int pos = rand() % (a.used+1);
    int k = MAX(rand() % max, 1);
    // int k = 1;
    printf("pos is: %d\n", pos);
    printf("k is: %d\n", k);
    for (int i = 0; i < MIN(k, max); ++i)
      insertArrayAt(&a, pos+i);
    max = max-k;
  };
  printArray(&a);
  printArrayBytes(&a);
  freeArray(&a);
}

///// end of Seq as Growable Array

///// unit tests

void testDecompress(){
  ByteArray ba;
  ba.len = 3;
  ba.data = malloc(ba.len);
  ba.data[0] = 0x03;
  ba.data[1] = 0x8f;
  ba.data[2] = 0x01;

  printByteArray(ba);
  printByteArray(decompress(ba));
}

void testGenerateBetween(){
  ByteArray ba1;
  ba1.len = 3;
  ba1.data = malloc(ba1.len);
  ba1.data[0] = 0x01;
  ba1.data[1] = 0x41;
  ba1.data[2] = 0x40;
  printByteArray(ba1);

  ByteArray ba2;
  ba2.len = 3;
  ba2.data = malloc(ba2.len);
  ba2.data[0] = 0x01;
  ba2.data[1] = 0x41;
  ba2.data[2] = 0x41;
  printByteArray(ba2);

  ByteArray res = ByteArray_GenerateBetween(ba1,ba2, Compression);
  printByteArray(res);
}

void testCompress(){
  ByteArray ba;
  ba.len = 130;
  ba.data = malloc(ba.len);
  ba.data[0] = 0x03;
  for (int i = 1; i < 129; ++i)
    ba.data[i] = 0x7f;
  ba.data[129] = 0x21;

  printByteArray(ba);
  printByteArray(compress(ba));
  printByteArray(decompress(compress(ba)));
}

///// end of unit tests

int main(int argc, char **argv) {
  // testCompress();
  // testDecompress();
  // testGenerateBetween();
  // srand((unsigned int)time(NULL));
  // rand();
  // randomInsertTest(1000);

  FILE *f = fopen("a", "w");
  if (f == NULL)
  {
      printf("Error opening file!\n");
      exit(1);
  }

  /* print some text */
  const char *text = "Write this to the file";

  fprintf(f, "%s,%s,%s\n", "Bytes","Random test","All appends");

  /* print integers and floats */
  fprintf(f, "%d,%d,%d\n", 1,23,121);
  fprintf(f, "%d,%d,%d\n", 2,213,234);
  fprintf(f, "%d,%d,%d\n", 3,124,176);
  fprintf(f, "%d,%d,%d\n", 4,43,37);
  fprintf(f, "%d,%d,%d\n", 5,12,5);
  fprintf(f, "%d,%d,%d\n", 6,3,2);

  fclose(f);

  return 0;
}