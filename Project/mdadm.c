#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cache.h"
#include "jbod.h"
#include "mdadm.h"

int is_mounted = 0;
int is_written = 0;

//bit oprations for disk, block, command, and reserved
uint32_t pack_bytes(uint32_t disk, uint32_t block, uint32_t command, uint32_t reserved) {
    uint32_t retval = 0x0, tempDisk, tempBlock, tempCommand, tempReserved;
	tempDisk = disk << 8;
	tempBlock = block;
	tempCommand = command << 12;
	tempReserved = reserved << 18;
	retval = tempDisk|tempBlock|tempCommand|tempReserved;
	return retval; 
}

int isMount = 0;

int mdadm_mount(void) {
  uint32_t op = pack_bytes(0,0,(JBOD_MOUNT),0); //uses JBOD_MOUNT command for mounting
	if (jbod_client_operation(op, NULL) == 0){ //checks if the mount has been successful
		isMount = 1; 
		return 1;
	} else {
		return -1;
	};
  return 0;
}

int mdadm_unmount(void) {
  uint32_t op = pack_bytes(0,0,(JBOD_UNMOUNT),0); //uses JBOD_UNMOUNT command for unmounting
	if (jbod_client_operation(op, NULL) == 0){ // checks if the unmount has been successful
		isMount = 0; 
		return 1;
	} else {
		return -1;
	};
}

int isPermission = 0;

int mdadm_write_permission(void){
  uint32_t op = pack_bytes(0,0,(JBOD_WRITE_PERMISSION),0);
  if (jbod_client_operation(op, NULL) == 0) {
	isPermission = 1;
    return 0;
  } else {
    return -1;
  }
}


int mdadm_revoke_write_permission(void){
  uint32_t op = pack_bytes(0,0,(JBOD_REVOKE_WRITE_PERMISSION),0);
  if (jbod_client_operation(op, NULL) == 0) {
	isPermission = 0;
    return 0;
  } else {
    return -1;
  }
}


int mdadm_read(uint32_t start_addr, uint32_t read_len, uint8_t *read_buf)  {
	//this checks if the system is mounted or unmounted
	if (isMount == 0) {
		return -1;
	} 

    //calculation for disk, block, and bytes
	int startAddrDisk = start_addr / JBOD_DISK_SIZE;
	int startAddrBlock = (start_addr  % JBOD_DISK_SIZE) / JBOD_BLOCK_SIZE;
	int startAddrBytes = ((start_addr) % JBOD_DISK_SIZE) % JBOD_BLOCK_SIZE;
	int endAddrDisk = (start_addr + read_len - 1) / JBOD_DISK_SIZE;
    int endAddrBlock = ((start_addr + read_len - 1) % JBOD_DISK_SIZE) / JBOD_BLOCK_SIZE;
	int endAddrBytes = ((start_addr + read_len - 1) % JBOD_DISK_SIZE) % JBOD_BLOCK_SIZE;

    //checks if the parameter is invalid
	if ((start_addr + read_len) > 1048576) { //when out of bound linear address
		return -1;
	} else if (startAddrBlock > 255 && startAddrBytes > 255) { //when it goes beyond the end of the linear 
		return -1;
	} else if (read_len > 2048) { //when larger than 2048 bytes
		return -1;
	} else if (read_len != 0 && read_buf == NULL) { //when passed NULL pointer and non zero length
		return -1;
	} else if (read_len == 0) { //when 0 length
		read_buf = NULL;
		return 0;
	}

	//read function 
	uint8_t *result = malloc(256); //temporary buffer

	int length = read_len;
	int index = 0;
	int currentDisk = startAddrDisk;
	int currentBlock = startAddrBlock;
	int finishBlock = endAddrBlock;

	if (cache_enabled() == true) { //checks the cache first
		while (currentDisk <= endAddrDisk) {
			if (currentDisk < endAddrDisk) { 
				finishBlock = 255;
			} else if (currentDisk == endAddrDisk ) {
				finishBlock = endAddrBlock;
			}
			while (currentBlock <= finishBlock) {
				if (cache_lookup(currentDisk, currentBlock, result) == 1) { //checks if the looking up cache has succeeded
					free(result);
					return read_len;
		    	}
				currentBlock += 1;
			}
			currentBlock = 0;
			currentDisk += 1;
		}
	}

	length = read_len;
	index = 0;
	currentDisk = startAddrDisk;
	currentBlock = startAddrBlock;
	finishBlock = endAddrBlock;

	while (currentDisk <= endAddrDisk) { //loop that runs until currentDisk reaches endAddrDisk
		uint32_t opSeekDisk = pack_bytes(currentDisk, 0, (JBOD_SEEK_TO_DISK),0); 
		jbod_client_operation(opSeekDisk, NULL); //operating JBOD_SEEK_TO_DISK command
		if (currentDisk < endAddrDisk) { //determins the finishBlock by checking if currentDisk is the same or less than endAddrDisk
			finishBlock = 255;
		} else if (currentDisk == endAddrDisk ) {
			finishBlock = endAddrBlock;
		}
		while (currentBlock <= finishBlock) { //loop that runs until currentBlock reaches finishBlock
			uint32_t opSeekBlock = pack_bytes(currentDisk,currentBlock,(JBOD_SEEK_TO_BLOCK),0);
			uint32_t opReadBlock = pack_bytes(currentDisk,currentBlock,(JBOD_READ_BLOCK),0);
			jbod_client_operation(opSeekBlock, NULL); //operating JBOD_SEEK_TO_BLOCK command
			jbod_client_operation(opReadBlock, result); //operating JBOD_READ_BLOCK command
			if (currentBlock == startAddrBlock && currentBlock == finishBlock) { //if current block is both starting and finishing block
				int finishLocation = 256;
				if (currentDisk == endAddrDisk) {
					finishLocation = endAddrBytes + 1;
				}
				memcpy(&read_buf[0], result, finishLocation - startAddrBytes);
				length -= (finishLocation-startAddrBytes);
				index += (finishLocation-startAddrBytes);
			} else if (currentBlock == startAddrBlock) { //if current block is the starting block
				
				memcpy(&read_buf[0], result, 255 - startAddrBytes + 1);
				int gap = 256 - startAddrBytes;
				length -= gap;
				index += gap;
			} else if (currentBlock != startAddrBlock && currentBlock != finishBlock) { //if current block is between starting and finishing block
				
				memcpy(&read_buf[index],result, 256);
				length -= 256;
				index += 256;
			} else if (currentBlock == finishBlock) { //if current block is the finishing block 
				int finishLocation = 256;
				if (currentDisk == endAddrDisk) {
					finishLocation = endAddrBytes + 1;
				}
		
				memcpy(&read_buf[index],result,finishLocation);
				length -= finishLocation;
				index += finishLocation;
			}
			cache_insert(currentDisk, currentBlock, result);
			currentBlock += 1;
        }
		currentBlock = 0;
		currentDisk += 1;
	}
	free(result);
	return read_len;
}


int mdadm_write(uint32_t start_addr, uint32_t write_len, const uint8_t *write_buf) {
	if (isMount == 0 && isPermission == 0) {
		return -1;
	}

	 //calculation for disk, block, and bytes
	int startAddrDisk = start_addr / JBOD_DISK_SIZE;
	int startAddrBlock = (start_addr  % JBOD_DISK_SIZE) / JBOD_BLOCK_SIZE;
	int startAddrBytes = ((start_addr) % JBOD_DISK_SIZE) % JBOD_BLOCK_SIZE;
	int endAddrDisk = (start_addr + write_len - 1) / JBOD_DISK_SIZE;
    int endAddrBlock = ((start_addr + write_len - 1) % JBOD_DISK_SIZE) / JBOD_BLOCK_SIZE;
	int endAddrBytes = ((start_addr + write_len - 1) % JBOD_DISK_SIZE) % JBOD_BLOCK_SIZE;


	if ((start_addr + write_len) > 1048576) { //when out of bound linear address
		return -1;
	}
	if (startAddrBlock > 255 && startAddrDisk > 15) { //when it goes beyond the end of the linear 
		return -1;
	}
	if (write_len > 2048) { //when larger than 2048 bytes
		return -1;
	}
	if (write_len != 0 && write_buf == NULL) { //when passed NULL pointer and non zero length
		return -1;
	}
	if (write_len == 0) { //when 0 length
		write_buf = NULL;
		return 0;
	}

	//read function 
	uint8_t *result = malloc(256); //temporary buffer


	int currentDisk = startAddrDisk;
	int currentBlock = startAddrBlock;
	int finishBlock;
	int startBytes = 0;
	int finishBytes;
	int bufferIndex = 0 ;

	while (currentDisk <= endAddrDisk) { //loop that runs until currentDisk reaches endAddrDisk
		uint32_t opSeekDisk = pack_bytes(currentDisk, 0, (JBOD_SEEK_TO_DISK),0); 
		jbod_client_operation(opSeekDisk, NULL); //operating JBOD_SEEK_TO_DISK command

		if (currentDisk < endAddrDisk) { //determines the finishBlock by checking if currentDisk is the same or less than endAddrDisk
			finishBlock = 255;
		} else if (currentDisk == endAddrDisk ) {
			finishBlock = endAddrBlock;
		}


		while (currentBlock <= finishBlock) { //loop that runs until currentBlock reaches finishBlock
			uint32_t opSeekBlock = pack_bytes(currentDisk,currentBlock,(JBOD_SEEK_TO_BLOCK),0);
			uint32_t opReadBlock = pack_bytes(currentDisk,currentBlock,(JBOD_READ_BLOCK),0);
			jbod_client_operation(opSeekBlock, NULL); //operating JBOD_SEEK_TO_BLOCK command
			jbod_client_operation(opReadBlock, result); //operating JBOD_READ_BLOCK command
			
			if ((currentDisk == startAddrDisk) && (currentBlock == startAddrBlock)) { //updates startBytes and finishBytes if currentDisk and currentBlock are both at the start
				startBytes = startAddrBytes;
				finishBytes = 255;
			}

			if ((currentDisk == endAddrDisk) && (currentBlock == endAddrBlock)) { //update startBytes and finishBytes if currentDisk and currentBlock are both at the end
				startBytes = 0;
				finishBytes = endAddrBytes;
			}

			if (((currentDisk == startAddrDisk) && (currentBlock == startAddrBlock)) && ((currentDisk == endAddrDisk) && (currentBlock == endAddrBlock))) {
				startBytes = startAddrBytes;
				finishBytes = endAddrBytes;
			}

			memcpy(&result[startBytes],&write_buf[bufferIndex], finishBytes - startBytes + 1);//copying data of the write_buf into result

			opSeekBlock = pack_bytes(currentDisk,currentBlock,(JBOD_SEEK_TO_BLOCK),0);
			jbod_client_operation(opSeekBlock, NULL); //operating JBOD_SEEK_TO_BLOCK command
		
			uint32_t opWriteBlock = pack_bytes(0,0,(JBOD_WRITE_BLOCK),0);
			jbod_client_operation(opWriteBlock, result); //operating JBOD_WRITE_BLOCK command
			
			if (cache_insert(currentDisk, currentBlock, result) == -1) { //if cache_insert fails, cache_update will run
				cache_update(currentDisk, currentBlock, result);
			}
			


			bufferIndex += (finishBytes - startBytes + 1);
			currentBlock += 1;
			startBytes = 0;
			finishBytes = 255; 
		}
		currentDisk += 1;
		currentBlock = 0;
	}

	free(result);
	return write_len;
}

