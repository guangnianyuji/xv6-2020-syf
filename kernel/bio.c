// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define BUCKETSIZE 13 // number of hashing buckets
#define BUFFERSIZE 5 // number of available buckets per bucket

extern uint ticks; // system time clock

// 一共有 13 * 5 = 65个buffer块儿
struct {
  struct spinlock lock;
  struct buf buf[BUFFERSIZE];
} bcachebucket[BUCKETSIZE];



void
binit(void)
{
  for (int i = 0; i < BUCKETSIZE; i++) {
    initlock(&bcachebucket[i].lock, "bcachebucket");
    for (int j = 0; j < BUFFERSIZE; j++) {
      initsleeplock(&bcachebucket[i].buf[j].lock, "buffer");
    }
  }
  //   struct buf *b;

  // initlock(&bcache.lock, "bcache");

  // // Create linked list of buffers
  // bcache.head.prev = &bcache.head;
  // bcache.head.next = &bcache.head;
  // for(b = bcache.buf; b < bcache.buf+NBUF; b++){
  //   b->next = bcache.head.next;
  //   b->prev = &bcache.head;
  //   initsleeplock(&b->lock, "buffer");
  //   bcache.head.next->prev = b;
  //   bcache.head.next = b;
  // }
}

void bsteal
(int* pbucketno,int* pleastid)
{
   int originno=*pbucketno;
  for(int j=1;j<BUCKETSIZE;j++)
  {
    int bucketno=(originno+j)%BUCKETSIZE;
    acquire(&bcachebucket[bucketno].lock);
    uint least=0xffffffff;//最小值
    int least_id=-1;
    for (int i = 0; i < BUFFERSIZE; i++) {
    struct buf *b = &bcachebucket[bucketno].buf[i];
    if(b->refcnt == 0 && b->lastuse < least) {
      least = b->lastuse;
      least_id = i;
    }
  }
   release(&bcachebucket[bucketno].lock);
    if(least_id!=-1){
      *pbucketno=bucketno;
      *pleastid=least_id;
      return;
    }
  }
  panic("bget: no unused buffer for recycle");
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int bucketno=blockno%BUCKETSIZE;

  acquire(&bcachebucket[bucketno].lock);

  // Is the block already cached?
  for(int i=0;i<BUFFERSIZE;i++)
  {
    b = &bcachebucket[bucketno].buf[i];
    if (b->dev == dev && b->blockno == blockno) {
      b->refcnt++;
      b->lastuse = ticks;
      release(&bcachebucket[bucketno].lock);
      acquiresleep(&b->lock);
      // --- end of critical session
      return b;
    }
  
  }

  uint least=0xffffffff;//最小值
  int least_id=-1;
  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
    for (int i = 0; i < BUFFERSIZE; i++) {
    b = &bcachebucket[bucketno].buf[i];
    if(b->refcnt == 0 && b->lastuse < least) {
      least = b->lastuse;
      least_id = i;
    }
  }

  if (least_id == -1) {
    bsteal(&bucketno,&least_id);
    // panic("bget: no unused buffer for recycle");
  }

  b = &bcachebucket[bucketno].buf[least_id];//替换掉

  b->dev = dev;
  b->blockno = blockno;
  b->lastuse = ticks;
  b->valid = 0;
  b->refcnt = 1;
  release(&bcachebucket[bucketno].lock);
  acquiresleep(&b->lock);
  // --- end of critical session
  return b;
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

 //放下面 releasesleep(&b->lock);

  // acquire(&bcache.lock);
  // b->refcnt--;
  // if (b->refcnt == 0) {
  //   // no one is waiting for it.
  //   b->next->prev = b->prev;
  //   b->prev->next = b->next;
  //   b->next = bcache.head.next;
  //   b->prev = &bcache.head;
  //   bcache.head.next->prev = b;
  //   bcache.head.next = b;
  // }
 // release(&bcache.lock);
 int bucketno=b->blockno%BUCKETSIZE;
 acquire(&bcachebucket[bucketno].lock);
 b->refcnt--;
 release(&bcachebucket[bucketno].lock);

 releasesleep(&b->lock);

}

void
bpin(struct buf *b) {
   int bucketno=b->blockno%BUCKETSIZE;
   acquire(&bcachebucket[bucketno].lock);
   b->refcnt++;
   release(&bcachebucket[bucketno].lock);
  // acquire(&bcache.lock);
  // b->refcnt++;
  // release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  int bucketno=b->blockno%BUCKETSIZE;
   acquire(&bcachebucket[bucketno].lock);
   b->refcnt--;
   release(&bcachebucket[bucketno].lock);
  // acquire(&bcache.lock);
  // b->refcnt--;
  // release(&bcache.lock);
}


