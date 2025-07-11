/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id) {
    Console::puts("Opening file.\n");
    fileId = _id;
    file_sys = _fs;
    int blockNo = file_sys->LookupFile(fileId)->block;
    memset(block_cache, 0, SimpleDisk::BLOCK_SIZE);
    file_sys->disk->read(blockNo,block_cache);
}

File::~File() {
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
    int blockNo = file_sys->LookupFile(fileId)->block;
    file_sys->disk->write(blockNo,block_cache);
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf) {
    Console::puts("reading from file\n");
    int cRead = 0;
    for(int i=0; i<_n; i++){
        if(EoF()) break;
        _buf[i] = block_cache[pos];
        pos++;
        cRead++;
    }
    return cRead;
}

int File::Write(unsigned int _n, const char *_buf) {
    Console::puts("writing to file\n");
    int cWrite = 0;
    for(int i=0; i<_n; i++){
        if(EoF()) break;
        block_cache[pos] = _buf[i];
        pos++;
        cWrite++;
    }
    return cWrite;
}

void File::Reset() {
    Console::puts("resetting file\n");
    pos = 0;
}

bool File::EoF() {
    Console::puts("checking for EoF\n");
    return pos == (SimpleDisk::BLOCK_SIZE - 1);
}
