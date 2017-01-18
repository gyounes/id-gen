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
    else if (pos == a->used-1) // insert in the end
      return ByteArray_GenerateBetween(a->ba[pos], bar, Compression);
    else
      return ByteArray_GenerateBetween(a->ba[pos-1], a->ba[pos], Compression);
}

void insertArrayAt(Array *a, int pos) {
  // a->used is the number of used entries, because a->ba[a->used++] updates a->used only *after* the array has been accessed.
  // Therefore a->used can go up to a->size 

  if (pos <= a->used+1) {
    ByteArray element = GenerateIdAt(a, pos);
    if (a->used == a->size) {
      a->size *= 2;
      a->ba = realloc(a->ba, a->size * sizeof(ByteArray));
    }
    // shift values right
    for (int i = a->used-1; i > pos; --i)
      a->ba[i+1] = a->ba[i];
    a->ba[pos] = element;
  }
  else
    printf("Position is out of bounds\n");
}

void freeArray(Array *a) {
  free(a->ba);
  a->ba = NULL;
  a->used = a->size = 0;
}

///// end of Seq as Growable Array

///// Sequence implmented as Linked list (initial dummy implementation)

/* linked list functions */
int Delete(node *, ByteArray);
void Traverse(node *);
node * createList();

node * createList()
{
   /* initializing list */
    node *head, *tail;
    head = (node *)malloc(sizeof(node));
    tail = (node *)malloc(sizeof(node));

    head->ba.len = 1;
    head->ba.data = malloc(head->ba.len);
    head->ba.data[0] = 0x00;
    head->next = tail;

    tail->ba.len = 1;
    tail->ba.data = malloc(tail->ba.len);
    tail->ba.data[0] = 0x80;
    tail->next = NULL;
    
    return head;
}

node * InsertAfter(node *pos)
{
    node * next = pos->next;
    assert(next != NULL);
    ByteArray ba = ByteArray_GenerateBetween(pos->ba, next->ba, Compression);
    node * newnode;
    newnode = (node *)malloc(sizeof(node));
    newnode->ba = ba;

    pos->next = newnode;
    newnode->next = next;
    return newnode;
}

void pushFront(node *head)
{
    ByteArray ba = ByteArray_GenerateBetween(head->ba, head->next->ba, Compression);
    node * newnode;
    newnode = (node *)malloc(sizeof(node));
    newnode->ba = ba;

    newnode->next = head->next;
    head->next = newnode;
}

void pushBack(node *head)
{
    node * previous = head;
    node * last = head->next;
    while (last->next != NULL)
    {
        previous = last;
        last = last->next;
    }
    ByteArray ba = ByteArray_GenerateBetween(previous->ba, last->ba, Compression);
    node * newnode;
    newnode = (node *)malloc(sizeof(node));
    newnode->ba = ba;

    newnode->next = last;
    previous->next = newnode;
}

int Delete(node *head, ByteArray ba)
{
    node * previous = head;
    node * current = head->next;
    while (current != NULL && !equalsTo(current->ba, ba))
    {
        previous = current;
        current = current->next;
    }
    if (current != head && current != NULL) /* if list empty or data not found */
    {
        previous->next = current->next;
        free(current);
        return 0;
    }
    else
        return 1;
}
 
void Traverse(node * head){
    node * current = head;
    while (current != NULL){
        //printf("%d ", current->ba);
        printByteArray(current->ba);
        current = current->next;
    }
    printf("\n");
}

void printSeqSize(node * head){
  int a[20]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  node * current = head->next;
  while (current != NULL){
    a[current->ba.len-1]++;
    current = current->next;
  }
  for (int i = 0; i < 20; ++i)
    printf("ids of size %d Byte(s): %d\n", i+1, a[i]);
}

ByteArray getByteArrayAt(node* head, int pos){
  int ctr = 0;
  while(ctr != pos){
    if (head->next == NULL)
      return head->ba;
    else
      head = head->next;
    ctr++;
  };
  return head->ba;
}

node * insertAfterPos(node* head, int pos){
  int ctr = 0;
  node * after;
  while(ctr != pos){
    if (head->next == NULL)
      break;
    else
      head = head->next;
    ctr++;
  };
  after = head;
  ByteArray ba = ByteArray_GenerateBetween(after->ba, after->next->ba, Compression);
  node * newnode;
  newnode = (node *)malloc(sizeof(node));
  newnode->ba = ba;
  newnode->next = after->next;
  after->next = newnode;
  return newnode;
}

int getSeqLen(node* head){
  int len = 0;
  while(head->next != NULL){
    len++;
    head = head->next;
  };
  return len-1;
}

void randomInsertTest(int max){
  node *seq = createList(), *last = seq;
  while(max > 0){
    int r1 = rand() % 2;
    if (r1 == 0){ // insert
      int r2 = rand() % getSeqLen(seq);
      last = insertAfterPos(seq, r2);
      max--;
    }
    else { // append
      int r3 = rand() % max;
      while(r3 > 0){
        last = InsertAfter(last);
        max--;
        r3--;
      }
    }
  };
  //Traverse(seq);
  printSeqSize(seq);
}

///// end of seq: linked list

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

void testPushFront(){
  node* seq = createList();
  Traverse(seq);
  for (int i = 0; i < 10; ++i)
    pushFront(seq);
  Traverse(seq);
  printSeqSize(seq);
}

void testPushBack(){
  node* seq = createList();
  Traverse(seq);
  for (int i = 0; i < 1000; ++i)
    pushBack(seq);
  Traverse(seq);
  printSeqSize(seq);
}

void testInsertAfter(){
  node* seq = createList();
  Traverse(seq);
  node * next = InsertAfter(seq);
  for (int i = 0; i < 5; ++i){
    pushFront(seq);
    next = InsertAfter(next);
  }
  Traverse(seq);
  printSeqSize(seq);
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
  // testInsertAfter();
  // testPushBack();
  // testPushFront();
  srand((unsigned int)time(NULL));
  rand();
  randomInsertTest(10000);
  return 0;
}