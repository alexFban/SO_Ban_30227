#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define STR_AND_LEN(X) X, strlen(X)
#define CMP(X, Y) strcmp(X, Y) == 0

int readStringPipe(int fd, char* buffer){
    int i=0;
    int bytesRead;
    while(1){
        bytesRead = read(fd, &buffer[i], sizeof(char));
        if(bytesRead != sizeof(char)) return -1;
        if(buffer[i] == '$'){
            buffer[i] = '\0';
            break;
        }
        i++;
    }
    printf("[AFB] String:'%s'\n", buffer);
    return 0;
}

int readIntPipe(int fd, unsigned int* nr){
    int bytesRead;
    bytesRead = read(fd, nr, sizeof(unsigned int));
    if(bytesRead != sizeof(unsigned int)) return -1;
    printf("[AFB] UInt: %d\n", *nr);
    return 0;
}

int command(const char* cmd){
	if(CMP(cmd, "VARIANT"))
		return 1;
	if(CMP(cmd, "CREATE_SHM"))
		return 2;
	if(CMP(cmd, "WRITE_TO_SHM"))
		return 3;
	if(CMP(cmd, "MAP_FILE"))
		return 4;
	if(CMP(cmd, "READ_FROM_FILE_OFFSET"))
		return 5;
	if(CMP(cmd, "READ_FROM_FILE_SECTION"))
		return 6;
	if(CMP(cmd, "READ_FROM_LOGICAL_SPACE_OFFSET"))
		return 7;
	if(CMP(cmd, "EXIT"))
		return 8;
	return 0;
}

int main(){
	int exit = 0;
	char rqst[250];
	unsigned int msgInt;
	volatile char *data = NULL;
	volatile char* filePoz = NULL;
	off_t fileSize;
	if(mkfifo("RESP_PIPE_34300", 0644) != 0){
		printf("ERROR\ncannot create response pipe\n");
		return 1;
	}
	int fdW = -1, fdR = -1;
	fdR = open("REQ_PIPE_34300", O_RDONLY);
	if(fdR == -1){
		unlink("RESP_PIPE_34300");
		printf("ERROR\ncannot open the request pipe\n");
		return 1;
	}
	fdW = open("RESP_PIPE_34300", O_WRONLY);
	if(fdW == -1){
		unlink("RESP_PIPE_34300");
		perror("Could not open write pipe");
		return 1;
	}
	write(fdW, STR_AND_LEN("HELLO$"));
	printf("[AFB] CONNECTION SUCCESS\n");
	while(!exit){
		readStringPipe(fdR, rqst);
		switch(command(rqst)){
			case 1:{
				write(fdW, STR_AND_LEN("VARIANT$"));
				msgInt = 34300;
				write(fdW, &msgInt, sizeof(unsigned int));
				write(fdW, STR_AND_LEN("VALUE$"));
				break;
			}
			case 2:{
				int fd;
				unsigned int size;
				readIntPipe(fdR, &size);
				fd = shm_open("/aV5Xs5Gv", O_RDWR | O_CREAT, 0644);
				write(fdW, STR_AND_LEN("CREATE_SHM$"));
				if(fd >= 0){
					ftruncate(fd, size);	
					data = (volatile char*)mmap(NULL, size, PROT_READ 
						| PROT_WRITE, MAP_SHARED, fd, 0);
					if(data != (void*)-1)				
						write(fdW, STR_AND_LEN("SUCCESS$"));
					else
						write(fdW, STR_AND_LEN("ERROR$"));
				}
				else
					write(fdW, STR_AND_LEN("ERROR$"));
				break;
			}
			case 3:{
				unsigned int offset, value;
				readIntPipe(fdR, &offset);
				readIntPipe(fdR, &value);
				write(fdW, STR_AND_LEN("WRITE_TO_SHM$"));
				if(offset >= 0 && offset <= 4550114){ //UInt ocupa 4 octeti
					char bytes[4];
					bytes[0] =  (value >> 24) & 0xFF;
					bytes[1] =  (value >> 16) & 0xFF;
					bytes[2] =  (value >> 8) & 0xFF;
					bytes[3] =  (value) & 0xFF;
					data[offset] = bytes[3];
					data[offset + 1] = bytes[2];
					data[offset + 2] = bytes[1];
					data[offset + 3] = bytes[0];
					write(fdW, STR_AND_LEN("SUCCESS$"));
				} else write(fdW, STR_AND_LEN("ERROR$"));
				break;
			}
			case 4:{
				char fileName[250];
				readStringPipe(fdR, fileName);
				int fd = open(fileName, O_RDONLY);
				if(fd == -1){
					write(fdW, STR_AND_LEN("MAP_FILE$"));
					write(fdW, STR_AND_LEN("ERROR$"));
					break;
				}
				fileSize = lseek(fd, 0, SEEK_END);
				lseek(fd, 0, SEEK_SET);
				write(fdW, STR_AND_LEN("MAP_FILE$"));
				filePoz = (char*)mmap(NULL, fileSize, PROT_READ, 
						MAP_SHARED, fd, 0);
				if(filePoz == (void*)-1){
					write(fdW, STR_AND_LEN("ERROR$"));
				} else write(fdW, STR_AND_LEN("SUCCESS$"));
				close(fd);
				break;
			}
			case 5:{
				unsigned int offset, noOfBytes;
				readIntPipe(fdR, &offset);
				readIntPipe(fdR, &noOfBytes);
				if(offset >= 0 && offset + noOfBytes <= fileSize){
					for(int i=0;i<noOfBytes;i++)
						data[i] = filePoz[i+offset];
					write(fdW, STR_AND_LEN("READ_FROM_FILE_OFFSET$"));
					write(fdW, STR_AND_LEN("SUCCESS$"));
				} else {
					write(fdW, STR_AND_LEN("READ_FROM_FILE_OFFSET$"));
					write(fdW, STR_AND_LEN("ERROR$"));
				}
				break;
			}
			case 6:{
			    unsigned short header_size = 0;
			    unsigned int section_offset, section_size;
			    unsigned section_no, offset, no_of_bytes;
			    readIntPipe(fdR, &section_no);
			    readIntPipe(fdR, &offset);
			    readIntPipe(fdR, &no_of_bytes); 
			    header_size = filePoz[fileSize - 3] & 0xFF;
			    header_size = (header_size * 256) + (filePoz[fileSize - 4] & 0xFF);
			    section_offset = filePoz[fileSize - header_size + 5 + 
			    ((section_no - 1) * 27) + 22] & 0xFF;
			    section_offset = (section_offset << 8) + (filePoz[fileSize - 
			    header_size + 5 + ((section_no - 1) * 27) + 21] & 0xFF);
			    section_offset = (section_offset << 8) + (filePoz[fileSize - 
			    header_size + 5 + ((section_no - 1) * 27) + 20] & 0xFF);
			    section_offset = (section_offset << 8) + (filePoz[fileSize - 
			    header_size + 5 + ((section_no - 1) * 27) + 19] & 0xFF);
			    section_size = filePoz[fileSize - header_size + 5 + 
			    ((section_no - 1) * 27) + 26] & 0xFF;
			    section_size = (section_size << 8) + (filePoz[fileSize - 
			    header_size + 5 + ((section_no - 1) * 27) + 25] & 0xFF);
			    section_size = (section_size << 8) + (filePoz[fileSize - 
			    header_size + 5 + ((section_no - 1) * 27) + 24] & 0xFF);
			    section_size = (section_size << 8) + (filePoz[fileSize - 
			    header_size + 5 + ((section_no - 1) * 27) + 23] & 0xFF);
		        if(section_size >= offset + no_of_bytes){
		            for(int i=0;i<no_of_bytes;i++)
		                data[i] = filePoz[section_offset + offset + i];
		            write(fdW, STR_AND_LEN("READ_FROM_FILE_SECTION$"));
					    write(fdW, STR_AND_LEN("SUCCESS$"));
		        } else {
					    write(fdW, STR_AND_LEN("READ_FROM_FILE_SECTION$"));
					    write(fdW, STR_AND_LEN("ERROR$"));
		        }
					break;
			}
			case 7:{
			    unsigned int logical_offset, no_of_bytes;
			    readIntPipe(fdR, &logical_offset);
			    readIntPipe(fdR, &no_of_bytes);
			    unsigned short header_size = 0;
			    header_size = filePoz[fileSize - 3] & 0xFF;
			    header_size = (header_size * 256) + (filePoz[fileSize - 4] & 0xFF);
			    char no_of_sections;
			    no_of_sections = filePoz[fileSize - header_size + 4] & 0xFF;
			    int section_size;
			    unsigned int section_offset;
			    char* readContent = NULL;
			    readContent = (char*)malloc((no_of_bytes + 1) * sizeof(char));
			    int necPage = (logical_offset/3072) + 1;
			    int pozInPage = (logical_offset%3072);
			    printf("%d - %d\n", necPage, pozInPage);
			    int currPage = 0;
			    for(int i=0;i<no_of_sections;i++){
			    	section_size = filePoz[fileSize - header_size + 5 + 
			    	(i * 27) + 26] & 0xFF;
			    	section_size = (section_size << 8) + (filePoz[fileSize - 
				    header_size + 5 + (i * 27) + 25] & 0xFF);
				    section_size = (section_size << 8) + (filePoz[fileSize - 
				    header_size + 5 + (i * 27) + 24] & 0xFF);
				    section_size = (section_size << 8) + (filePoz[fileSize - 
				    header_size + 5 + (i * 27) + 23] & 0xFF);
				    section_offset = filePoz[fileSize - header_size + 5 + 
			    	(i * 27) + 22] & 0xFF;
			    	section_offset = (section_offset << 8) + (filePoz[fileSize - 
				    header_size + 5 + (i * 27) + 21] & 0xFF);
				    section_offset = (section_offset << 8) + (filePoz[fileSize - 
				    header_size + 5 + (i * 27) + 20] & 0xFF);
				    section_offset = (section_offset << 8) + (filePoz[fileSize - 
				    header_size + 5 + (i * 27) + 19] & 0xFF);
				    printf("Section %d size %d offset %d\n", i+1, section_size, section_offset);
				    while(section_size > 0){
				    	currPage++;
				    	if(currPage == necPage){
				    		for(int poz=0;poz<no_of_bytes;poz++)
				    			readContent[poz] = filePoz[section_offset + pozInPage + poz];
				    		printf("%s\n", readContent);
				    		goto exit;
				    	}
				    	section_size -= 3072;
				    }
			    }
			    exit: {}
			    for(int i=0;i<no_of_bytes;i++){
			    	data[i] = readContent[i];
			    }
				write(fdW, STR_AND_LEN("READ_FROM_LOGICAL_SPACE_OFFSET$"));
				write(fdW, STR_AND_LEN("SUCCESS$"));
				free(readContent);
				break;
			}
			case 8:{
				close(fdR);
				fdR = -1;
				close(fdW);
				fdW = -1;
				unlink("RESP_PIPE_34300");
				exit = 1;
				break;
			}
			default: break;
		}
	}
	return 0;
}
