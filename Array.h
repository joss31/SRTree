#ifndef _ARRAY_H_
#define _ARRAY_H_

#include <math.h>
#include <stdlib.h>


template<class T> class Array{
  protected:
    struct Block{
      bool alloc;
      T    *ptr;
      } *blocks;
    int numBlocks, szBlock;
    int size;
    int blockMask;
  
  public:

    Array(){
      size = 0;
      blocks = NULL;
      numBlocks = 0;
      szBlock = 8;
      blockMask = getMask(szBlock);
      }

    Array(int initSize, int blockSize = 8){
      size = 0;
      blocks = NULL;
      numBlocks = 0;
      allocate(initSize, blockSize);      
      }

    ~Array(){
      if (numBlocks != 0)
        free();
      }

    void allocate(int initSize, int initBlockSize = 8){

      int elementsPerBlock = getEPB(initBlockSize);  
      int initBlocks = (int)ceil((float)initSize/elementsPerBlock);

      blocks = (Block*)malloc(initBlocks*sizeof(Block));
      CHECK_MEMORY1(blocks != NULL, "Tryed to Allocate %d bytes", (initBlocks*sizeof(Block)));

      blocks[0].alloc = true;
      blocks[0].ptr = new T[elementsPerBlock*initBlocks];
      CHECK_MEMORY1(blocks[0].ptr != NULL, "Tryed to Allocate %d bytes", elementsPerBlock*initBlocks);
      for (int i = 1; i < initBlocks; i++){
        blocks[i].alloc = false;
        blocks[i].ptr = &blocks[0].ptr[i*elementsPerBlock];
        }

      size = initSize;
      szBlock = initBlockSize;
      numBlocks = initBlocks;
      blockMask =  getMask(szBlock);
      }

    void free(){
      CHECK_DEBUG(numBlocks != 0, "Not Allocated");
      for (int i = 0; i < numBlocks; i++)
        if (blocks[i].alloc)
          delete [] blocks[i].ptr;

      ::free((void*)blocks);
      numBlocks = 0;
      blocks = NULL;
      size = 0;
      }

    void reallocate(int newSize, int newBlockSize = -1){
      if (numBlocks != 0)
        free();

      if (newBlockSize < 0)
        newBlockSize = szBlock;

      allocate(newSize, newBlockSize);
      }

    void resize(int newSize){
      if (newSize >= getAllocSize()){
        int elementsPerBlock = getEPB(szBlock);
        int newBlocks = (int)ceil((float)newSize/elementsPerBlock);

        if (newBlocks > numBlocks){
          blocks = (Block*)realloc(blocks, newBlocks*sizeof(Block));
          CHECK_MEMORY1(blocks != NULL, "Tryed to Allocate %d bytes", newBlocks*sizeof(Block));

          blocks[numBlocks].alloc = true;
          blocks[numBlocks].ptr = new T[elementsPerBlock*(newBlocks-numBlocks)];
          CHECK_MEMORY1(blocks[numBlocks].ptr != NULL, "Tryed to Allocate %d bytes", elementsPerBlock*(newBlocks-numBlocks));
          for (int i = numBlocks+1; i < newBlocks; i++){
            blocks[i].alloc = false;
            blocks[i].ptr = &blocks[numBlocks].ptr[(i-numBlocks)*elementsPerBlock];
            }

          numBlocks = newBlocks;
          }
        }
      setSize(newSize);
      }

    __inline void setSize(int newSize){
      CHECK_DEBUG2(newSize >= 0 && newSize <= getAllocSize(), "Invalid Size : %d, AllocSize : %d", newSize, getAllocSize());
      size = newSize;
      }

    __inline int getSize() const{
      return size;
      }

    __inline const T& index(int i) const{
      CHECK_DEBUG2((unsigned int)i < (unsigned int)size, "Index Array Out of Bounds, Size : %d, Index : %d", size, i);
      CHECK_DEBUG(blocks != NULL, "BLOCKS isn't Valid");
      return blocks[i>>szBlock].ptr[i&blockMask];
      }

    __inline T& index(int i){
      CHECK_DEBUG2((unsigned int)i < (unsigned int)size, "Index Array Out of Bounds, Size : %d, Index : %d", size, i);
      CHECK_DEBUG(blocks != NULL, "BLOCKS isn't Valid");
      return blocks[i>>szBlock].ptr[i&blockMask];
      }

    void copy(const Array<T> &src){
      CHECK_DEBUG2(getAllocSize() >= src.size, "Array not big enough Src : %d, Dest : %d", src.size, getAllocSize());

      size = src.size;
      for (int i = 0; i < src.size; i++)
        index(i) = src.index(i);
      }

    void clone(const Array<T> &src){
      resize(src.getSize());
      copy(src);
      }

    void append(const Array<T> &src){
      int n = getSize();
      int m = src.getSize();

      resize(n + m);
      for (int i = 0; i < m; i++)
        index(n+i) = src.index(i);
      }

    void clear(){
      int BpB = sizeof(T)*getEPB(szBlock);
      for (int i = 0; i < numBlocks; i++)
        memset(blocks[i].ptr, 0, BpB);
      }

    __inline int addIndex(){
      int s = getSize();
      resize(s+1);
      return s;
      }

    __inline T& addItem(){
      int s = addIndex();
      return index(s);
      }

    void removeItem(int i){

      int s = getSize();
      for (int j = i; j < size-1; j++)
        index(j) = index(j+1);
      resize(s-1);
      }

    __inline int getAllocSize() const{
      return numBlocks*getEPB(szBlock);
      }

    bool inList(int v, int sI = 0, int eI = -1) const{
      if (eI < 0)
        eI = getSize();
      for (int i = sI; i < eI; i++)
        if (index(i) == v)
          return true;

      return false;
    }

  private:
    __inline static int getMask(int bs){
      return ~((unsigned int)-1 << bs);
      }

    __inline static int getEPB(int bs){
      return 0x01 << bs;  
      }
};

#endif
