/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
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
#include "file_system.H"

/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    inodeArray = new Inode[SimpleDisk::BLOCK_SIZE];
free_blocks = new unsigned char[SimpleDisk::BLOCK_SIZE];
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    /* Make sure that the inode list and the free list are saved. */
    delete inodeArray;
    delete free_blocks;
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system from disk\n");
disk = _disk;
unsigned char dataBuffer[SimpleDisk::BLOCK_SIZE];
memset(dataBuffer, 0, SimpleDisk::BLOCK_SIZE);
disk->read(0, dataBuffer);
inodeArray = (Inode*) dataBuffer;
memset(dataBuffer, 0, SimpleDisk::BLOCK_SIZE);
disk->read(1, dataBuffer);
free_blocks = (unsigned char *) dataBuffer;
return true;

}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
    // Calculate the total number of blocks based on disk size
long totalBlocks = _size / SimpleDisk::BLOCK_SIZE;
unsigned char dataBuffer[SimpleDisk::BLOCK_SIZE];
memset(dataBuffer, 0, SimpleDisk::BLOCK_SIZE);
for(int blockIndex = 3; blockIndex < totalBlocks; blockIndex++){
    _disk->write(blockIndex, dataBuffer);
}

Inode* inodeTemp = (Inode*) dataBuffer;
for (int index = 0; index < MAX_INODES; index++) {
    inodeTemp[index].id = -1;
    inodeTemp[index].block = -1;
    inodeTemp[index].size = 0;
}
_disk->write(0, dataBuffer);
memset(dataBuffer, 0, SimpleDisk::BLOCK_SIZE);
dataBuffer[0] = 1;
dataBuffer[1] = 1;

for (int pos = 3; pos < MAX_INODES; pos++) {
    dataBuffer[pos] = 0;
}
_disk->write(1,dataBuffer);
    return true;

}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    unsigned char dataBuffer[SimpleDisk::BLOCK_SIZE];
    disk->read(0, dataBuffer);
    inodeArray = (Inode *) dataBuffer;
    for(int i=0;i<MAX_INODES;i++){
        if(inodeArray[i].id == _file_id){
            return &inodeArray[i];
        }
    }
    return NULL;
}

int FileSystem::GetFreeBlock(){
    unsigned char dataBuffer[SimpleDisk::BLOCK_SIZE];
    memset(dataBuffer,1, SimpleDisk::BLOCK_SIZE);
    disk->read(1,dataBuffer);
    for(int i=0;i<MAX_INODES;i++){
        if(dataBuffer[i] == 0){
            dataBuffer[i] = 1;
            disk->write(1,dataBuffer);
            return i;
        }
    }
    return -1;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
    // Initialize buffer to hold block data
unsigned char dataBuffer[SimpleDisk::BLOCK_SIZE];
memset(dataBuffer, 0, SimpleDisk::BLOCK_SIZE);
disk->read(0, dataBuffer);
inodeArray = (Inode *) dataBuffer;
int availableInode = -1;
for (int index = 0; index < MAX_INODES; index++) {
    if (inodeArray[index].id == -1 && availableInode == -1) {
        availableInode = index; // Mark first free inode
    }
    if (inodeArray[index].id == _file_id) {
        return false;
    }
}

inodeArray[availableInode].id = _file_id;
inodeArray[availableInode].size = SimpleDisk::BLOCK_SIZE;
inodeArray[availableInode].block = GetFreeBlock();
disk->write(0, dataBuffer);
return true;

}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
       
       // Setup buffers for inode and data block
unsigned char dataBuffer[SimpleDisk::BLOCK_SIZE];
memset(dataBuffer, 0, SimpleDisk::BLOCK_SIZE);
disk->read(0, dataBuffer);
unsigned char iBuffer[SimpleDisk::BLOCK_SIZE];
memset(iBuffer, 0, SimpleDisk::BLOCK_SIZE);
disk->read(1, iBuffer);

// Cast the inode buffer to an Inode array
inodeArray = (Inode *) dataBuffer;

// Search for the inode with the specified file ID
int inodeIndex = -1;
for (int idx = 0; idx < MAX_INODES; idx++) {
    if (inodeArray[idx].id == _file_id) {
        inodeIndex = idx;
    }
}

// Exit if no matching inode is found
if (inodeIndex == -1) return false;

// Reset the found inode and update the data block
inodeArray[inodeIndex].id = -1;
iBuffer[inodeArray[inodeIndex].block] = 0;
inodeArray[inodeIndex].block = -1;
inodeArray[inodeIndex].size = 0;

// Write the updated buffers back to disk
disk->write(0, dataBuffer);
disk->write(1, iBuffer);

// Indicate success
return true;

       
       
       
}
