struct buf {
  int valid;   // has data been read from disk?
  int disk;    // does disk "own" buf?
  uint dev; //表示缓冲区所属的设备号。在多设备系统中，每个设备都有一个唯一的设备号用于标识设备。
  uint blockno;
  struct sleeplock lock;
  uint refcnt;
  struct buf *prev; // LRU cache list
  struct buf *next;
  uchar data[BSIZE];
  int lastuse;//最近被使用的时间戳
};

